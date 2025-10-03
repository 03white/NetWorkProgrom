#include"TcpConnection.h"
#include"./utils/Logger.h"
#include"./Acceptor/Socket.h"
#include"./reactor/Channel.h"
#include"./reactor/EventLoop.h"
#include<functional>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<strings.h>
#include<netinet/tcp.h>
#include<sys/socket.h>
#include<string>
//static method
static EventLoop* CheckLoopNotNull(EventLoop*loop){
    if(loop==nullptr){
        LOG_FATAL("%S:%S:%d TcpConnection loop is null! \n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}
/*************
public method
*************/
TcpConnection::TcpConnection(EventLoop*loop,
                  const std::string&name,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr
                  )
        : reading_(true) 
        , state_(kConnecting)
        , name_(name)
        , localAddr_(localAddr)
        , peerAddr_(peerAddr)
        , socket_(new Socket(sockfd))
        , channel_(new Channel(loop,sockfd))
        , loop_(CheckLoopNotNull(loop))
        , highWaterMark_(64*1024*1024) //64M
{
    //set channel's callback
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n",name_.c_str(),sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection(){
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d \n",name_.c_str(),channel_->fd(),(int)state_);
}

//establish connect
void TcpConnection::connectEstablished(){
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    //excute callback when new connection is established
    connectionCallback_(shared_from_this());
}

//destroyed
void TcpConnection::connectDestroyed(){
    if(state_==kConnected){
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();//remove channel from poll
}

//send data
void TcpConnection::send(const std::string &buf){
    if(state_==kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(buf.c_str(),buf.size());
        }else{
            loop_->postInLoop(std::bind(&TcpConnection::sendInLoop,this,buf.c_str(),buf.size()));
        }
    }
}

//close connection
void TcpConnection::shutdown(){
    if(state_==kConnected){
        setState(kDisconnecting);
        loop_->postInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
    }    
}

/**************
private method 
**************/
//the method I don't understand most
void TcpConnection::sendInLoop(const void*data,size_t len){
    ssize_t nwrote=0;
    size_t remaining=len;
    bool faultError=false;
    if(state_==kDisconnected){
        LOG_ERROR("disconnected, give up writing!");
        return;
    }
    if(!channel_->isWriting()&&outputBuffer_.readableBytes()==0){
        nwrote =::write(channel_->fd(),data,len);
        if(nwrote>=0){
            remaining=len-nwrote;
            if(remaining==0&&writeCompleteCallback_){
                loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
            }
        }else{
            nwrote=0;
            if(errno!=EWOULDBLOCK){
                LOG_ERROR("TcpConnection::senndInLoop");
                if(errno==EPIPE||errno==ECONNRESET){
                    faultError=true;
                }
            }
        }
    }
    /*
     that's a little difficult for me to understand,emm 
    */
   if(!faultError&&remaining>0){
    size_t oldLen=outputBuffer_.readableBytes();
    if(oldLen+remaining>=highWaterMark_&&oldLen<highWaterMark_&&highWaterMark_){
        loop_->queueInLoop(std::bind(highWaterMarkCallback_,shared_from_this(),oldLen+remaining));
    }
   }
}

void TcpConnection::shutdownInLoop(){
    if(!channel_->isWriting()){
        socket_->shutdownWrite();
    }
}

void TcpConnection::handleRead(Timestamp receiveTime){
    int saveErrno=0;
    ssize_t n=inputBuffer_.readFd(channel_->fd(),&saveErrno);
    if(n>0){
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }else if(n==0){// the TcpConnection has closed
        handleClose();
    }else{
        errno=saveErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite(){
    if(channel_->isWriting()){
        int saveErrno=0;
        ssize_t n=outputBuffer_.writeFd(channel_->fd(),&saveErrno);
    }

}

//when the TcpConnection is closed,we use handleClose function
void TcpConnection::handleClose(){
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n",channel_->fd(),(int)state_);
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError(){
    int optval;
    socklen_t optlen=sizeof(optval);
    int err=0;
    if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optval,&optlen)<0){
        err=errno;
    }else{
        err=optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n",name_.c_str(),err);
}

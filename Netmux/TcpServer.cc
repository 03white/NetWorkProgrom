#include"TcpServer.h"
#include"./utils/Logger.h"
#include"TcpConnection.h"
#include<strings.h>
#include<functional>
//static method
static EventLoop* CheckLoopNotNull(EventLoop* loop){
    if(loop==nullptr){
        LOG_FATAL("%s:%s%d mainLoop is null! \n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}
/*************
public method
************/
TcpServer::TcpServer(
    EventLoop*loop,
    const InetAddress &listenAddr,
    const std::string &name,
    Option option=kNoReusePort)
    : ipPort_(listenAddr.toIpPort())
    , name_(name)
    , started_(false)
    , nextConnId_(1)
    , loop_(CheckLoopNotNull(loop))
    , threadPool_(new EventLoopThreadPool(loop,name))
    , acceptor_(new Acceptor(loop,listenAddr,option==kReusePort))
    , connectionCallback_()
    , messageCallback_() 
{
    //when new client come,then we execute TcpServer::newConnection this callback
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));// I don't know that,emm,especially pram
}

TcpServer::~TcpServer(){
    for(auto &item: connections_){
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        //destroy connection
        conn->getLoop()->postInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
    }
}

void TcpServer::setThreadNum(int num){
    threadPool_->setThreadNum(num);
}

void TcpServer::start(){
    if(started_++==0){
        threadPool_->start(threadInitCallback_);//start the threadPool loop
        loop_->postInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
    }
}
/*************
private method
************/
// the most complex,import method in this class
void TcpServer::newConnection(int sockfd,const InetAddress &peerAddr){
    //Polling algrithm,randly choose a ioloop to charge the channel that acceptor creates right now
    EventLoop *ioloop=threadPool_->getNextLoop();
    char buf[64]={0};
    snprintf(buf,sizeof(buf),"-%s#%d",ipPort_.c_str(),nextConnId_++); 
    std::string connName=name_+buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

    sockaddr_in local;
    ::bzero(&local,sizeof(local));
    socklen_t addrlen=sizeof(local);
    if(::getsockname(sockfd,(sockaddr*)&local,&addrlen)<0){
        LOG_ERROR("sockets::getLocalAddr");
    }
    InetAddress localAddr(local);
    TcpConnectionPtr connPtr(new TcpConnection(
                            ioloop,
                            connName,
                            sockfd,
                            localAddr,
                            peerAddr)
    );
    connections_[connName]=connPtr;
    connPtr->setConnectionCallback(connectionCallback_);
    connPtr->setMessageCallback(messageCallback_);
    connPtr->setWriteCompleteCallback(writeCompleteCallback_);
    connPtr->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
    ioloop->postInLoop(std::bind(&TcpConnection::connectEstablished,connPtr));
}

void TcpServer::removeConnection(const TcpConnectionPtr &connPtr){
    loop_->postInLoop(std::bind(&TcpServer::removeConnection,this,connPtr));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &connPtr){
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n",name_.c_str(),connPtr->name().c_str());
    connections_.erase(connPtr->name());
    EventLoop*ioLoop=connPtr->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed,connPtr)) ;   
}

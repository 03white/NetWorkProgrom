#include"Acceptor.h"
#include"../utils/Logger.h"
#include"../utils/InetAddress.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<unistd.h>
//static global method
static int createNonblocking(){
    int sockfd=::socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,0);
    if(sockfd<0){
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno);
    }
    return sockfd;
}
/**************
 public method 
 *************/
Acceptor::Acceptor(EventLoop*loop,const InetAddress &listenAddr,bool reuseport)
    : listenning_(false)
    , acceptSocket_(createNonblocking())
    , acceptChannel_(loop,acceptSocket_.fd())
    , loop_(loop)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}

Acceptor::~Acceptor(){
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

//important method
void Acceptor::listen(){
    listenning_=true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

/*************
private method 
*************/
// the most important method in this class
void Acceptor::handleRead(){
    InetAddress peerAddr;
    int connfd=acceptSocket_.accept(&peerAddr);
    if(connfd>=0){
        if(newConnectionCallback_){
            newConnectionCallback_(connfd,peerAddr);
        }else{
            ::close(connfd);
        }
    }else{
        LOG_ERROR("%s:%s:%d accept err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno);
        if(errno==EMFILE){
            LOG_ERROR("%s:%s:%d sockfd reached limit!\n",__FILE__,__FUNCTION__,__LINE__);
        }
    }
}
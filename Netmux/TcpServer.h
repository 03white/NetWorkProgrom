#pragma once
#include"./utils/noncopyable.h"
#include"./utils/InetAddress.h"
#include"Callbacks.h"
#include"Buffer.h"
#include"./Acceptor/Acceptor.h"
#include"./reactor/EventLoopThreadPool.h"
#include"./reactor/EventLoop.h"
#include"TcpConnection.h"
#include<functional>
#include<memory>
#include<string>
#include<atomic>
#include<unordered_map>
/*



*/
class TcpServer:public noncopyable{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;
    using ConnectionMap=std::unordered_map<std::string,TcpConnectionPtr>;
    enum Option{kNoReusePort,kReusePort};
public:
    //constructor and destructor
    TcpServer(EventLoop*loop,const InetAddress &listenAddr,const std::string &name,Option option=kNoReusePort);
    ~TcpServer();
    //callback setter,the user'll interest most about it
    void setThreadInitCallback(const ThreadInitCallback &cb){threadInitCallback_=cb;}
    void setConnectionCallback(const ConnectionCallback &cb){connectionCallback_=cb;}
    void setMessageCallback(const MessageCallback &cb){messageCallback_=cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback &cb){writeCompleteCallback_=cb;}
    //main memberr we use in main
    void setThreadNum(int num);
    void start();
private:
    //flag to tag the TcpServer
    const std::string ipPort_;
    const std::string name_;
    std::atomic_int started_;
    int nextConnId_;
    //functional member,what we need to pay more attention
    EventLoop*loop_;
    std::shared_ptr<EventLoopThreadPool>threadPool_;
    std::unique_ptr<Acceptor>acceptor_;
    ConnectionMap connections_;
    //callback use 
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;
    void newConnection(int sockfd,const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &connPtr);
    void removeConnectionInLoop(const TcpConnectionPtr &connPtr);
}; 
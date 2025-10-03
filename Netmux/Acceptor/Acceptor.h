#pragma once
#include"../utils/noncopyable.h"
#include"Socket.h"
#include"Channel.h"
#include<functional>
class EventLoop;
class InetAddress;
class Acceptor:public noncopyable{
public:
    using NewConnectionCallback=std::function<void(int sockfd,const InetAddress&)>;
public:
    Acceptor(EventLoop*loop,const InetAddress &listenAddr,bool reuseport);
    ~Acceptor();
    //setter
    void setNewConnectionCallback(const NewConnectionCallback &cb){newConnectionCallback_=cb;}
    //getter
    bool listenning()const{return listenning_;}
    //important method
    void listen();
private:
    bool listenning_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    EventLoop*loop_;
    NewConnectionCallback newConnectionCallback_;
    void handleRead();
};
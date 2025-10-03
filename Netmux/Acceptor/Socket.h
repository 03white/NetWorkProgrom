#pragma once
#include"../utils/noncopyable.h"
class InetAddress;
class Socket:public noncopyable{
public:
    explicit Socket(int sockfd):sockfd_(sockfd){}
    ~Socket();
    //setter
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    //getter
    int fd()const{return sockfd_;}
    //important methods
    void bindAddress(const InetAddress &localaddr);
    void listen();
    int  accept(InetAddress *peeraddr);
    void shutdownWrite();
private:
    const int sockfd_;
};
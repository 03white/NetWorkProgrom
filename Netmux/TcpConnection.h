#pragma once
#include"../utils/noncopyable.h"
#include"../utils/InetAddress.h"
#include"Callbacks.h"
#include"Buffer.h"
#include"Timestamp.h"
#include<memory>
#include<string>
#include<atomic>
class Channel;
class EventLoop;
class Socket;
/*


*/
class TcpConnection:public noncopyable,public std::enable_shared_from_this<TcpConnection>{
public:
    //constructor and destructor
    TcpConnection(EventLoop*loop,
                  const std::string&name,
                  int sockfd,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr
                  );
    ~TcpConnection();
    //getter
    const std::string& name()const{return name_;}
    const InetAddress& localAddress()const{return localAddr_;}
    const InetAddress& peerAddress()const{return peerAddr_;}
    EventLoop* getLoop()const{return loop_;}
    bool connected()const{return state_==kConnected;}
    //setter
    void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_=cb;}
    void setMessageCallback(const MessageCallback& cb){messageCallback_=cb;}
    void setWriteCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_=cb;}
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,size_t highWaterMark){highWaterMarkCallback_=cb;highWaterMark_=highWaterMark;}
    void setCloseCallback(const CloseCallback& cb){closeCallback_=cb;}
    //establish connect
    void connectEstablished();
    //destroyed
    void connectDestroyed();
    //send data
    void send(const std::string &buf);
    //close connection
    void shutdown();
private:
    enum StateE{kConnecting,kConnected,kDisconnecting,kDisconnected};
    bool reading_;
    std::atomic_int state_;
    const std::string name_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    size_t highWaterMark_;// high water level ,we usually say it this name
    std::unique_ptr<Socket>socket_;
    std::unique_ptr<Channel>channel_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    EventLoop*loop_;

    //callback function
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    //setter
    void setState(StateE state){state_=state;}
    //the following two method,I don't know what's this?
    void sendInLoop(const void*message,size_t len);
    void shutdownInLoop();

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
};
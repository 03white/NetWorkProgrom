#pragma once
#include"../utils/noncopyable.h"
#include"../utils/Timestamp.h"
#include"../utils/Logger.h"
#include<functional>
#include<memory>

class EventLoop;
class Channel:public noncopyable{
public:
    using EventCallback=std::function<void()>;
    using ReadEventCallback=std::function<void(Timestamp)>;//if we bind member function,the member function need args timestamp
public:
    //constructor and destroyer
    Channel(EventLoop*loop,int fd);
    ~Channel();
    //getter
    int fd()const{return fd_;}
    int events()const{return events_;}
    bool isNonEvent()const{return events_==kNoneEvent;}
    bool isWriting()const{return events_==kWriteEvent;}
    bool isReading()const{return events_==kReadEvent;}
    EventLoop*ownerLoop()const{return loop_;}
    int status(){return status_;}

    //setter
    void enableReading(){events_|=kReadEvent;update();}
    void disenableReading(){events_&=~kReadEvent;update();}
    void enableWriting(){events_|=kWriteEvent;update();}
    void disenableWriting(){events_&=~kWriteEvent;update();}
    void disableAll(){events_=kNoneEvent;update();}
    int set_revents(int revt){revents_=revt;return 0;}
    void set_status(int status){status_=status;}
    //set callback
    void setWriteCallback(EventCallback cb){writeCallback_=std::move(cb);}
    void setReadCallback(ReadEventCallback cb){readCallback_=std::move(cb);}
    void setErrorCallback(EventCallback cb){errorCallback_=std::move(cb);}
    void setCloseCallback(EventCallback cb){closeCallback_=std::move(cb);}
    void handleEvent(Timestamp receiveTime);
    //specail method
    void tie(const std::shared_ptr<void>&);
    void remove();
private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    
    const int fd_;
    int events_;
    int revents_;
    EventLoop*loop_;
    int status_;//I don't konw why it being
    //special member
    std::weak_ptr<void>tie_;
    bool tied_;
    //callback
    EventCallback writeCallback_;
    ReadEventCallback readCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
    void update();
    void handleEventWithGuard(Timestamp receiveTIme);
};
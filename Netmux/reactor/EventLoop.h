#pragma once
#include"../utils/noncopyable.h"
#include"../utils/Timestamp.h"
#include"../utils/CurrentThread.h"
#include<functional>
#include<vector>
#include<atomic>
#include<memory>
#include<mutex>

class Channel;
class Poller;

class EventLoop:public noncopyable{
public:
    using Functor=std::function<void()>;
    using ChannelList=std::vector<Channel*>;
public:
    //constructor
    EventLoop();
    ~EventLoop();

    void loop();// the core method that start our while(true)
    void quit();
    void wakeup();
    //method to adjust the object(channel) listened by we
    void updateChannel(Channel*channel);
    void removeChannel(Channel*channel);
    bool hasChannel(Channel*channel);
    Timestamp pollReturnTime()const{return pollReturnTime_;}
    //methods to maintain the Thread Affinity---"one loop per thread"
    void postInLoop(Functor cb);
    void queueInLoop(Functor cb);
    bool isInLoopThread()const{return threadId_==CurrentThread::tid();}
private:
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    const pid_t threadId_;
    Timestamp pollReturnTime_;
    ChannelList activeChannel_;
    int wakeupFd_;
    std::unique_ptr<Channel>wakeupChannel_;
    std::unique_ptr<Poller>poller_;
    std::vector<Functor>pendingFunctors_;//task queue
    std::atomic_bool callingPendingFunctors_;
    std::mutex mutex_;
    //private method
    void handleRead();//consume the product that wakeup() create
    void doPendingFunctors();// execute callback
};
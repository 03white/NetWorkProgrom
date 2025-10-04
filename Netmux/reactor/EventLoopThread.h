#pragma once
#include"../utils/noncopyable.h"
#include"../utils/Thread.h"
#include<functional>
#include<mutex>
#include<condition_variable>
#include<string>

class EventLoop;
class EventLoopThread:noncopyable{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;
public:
     //constructor and destroyer
     EventLoopThread(const ThreadInitCallback &cb=ThreadInitCallback(),const std::string &name=std::string());
     ~EventLoopThread();
     // start eventloop,the most important method
     EventLoop* startLoop();
private:
    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
    //private method
    void threadFunc();
};
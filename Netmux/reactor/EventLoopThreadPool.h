#pragma once
#include"noncopyable.h"
#include<functional>
#include<string>
#include<vector>
#include<memory>

class EventLoop;;
class EventLoopThread;
class EventLoopThreadPool:public noncopyable{
public:
    using ThreadInitCallback=std::function<void(EventLoop*)>;
public:
    //constructor and destructor
    EventLoopThreadPool(EventLoop*mainLoop,const std::string &name);
    ~EventLoopThreadPool(){}
    //setter
    void setThreadNum(int threadNum){threadNum_=threadNum;}
    //getter
    bool started()const{return started_;}
    const std::string name()const{return name_;}
    EventLoop* getNextLoop();// selecter/getter
    std::vector<EventLoop*>getAllEventLoop()const{return loops_.empty()?std::vector<EventLoop*>(1,mainLoop_):loops_;} 
    //maybe the most useful method for outside model
    void start(const ThreadInitCallback &cb=ThreadInitCallback());
private:
    bool started_;
    int threadNum_;
    int next_;//I don't know what's this
    std::string name_;
    EventLoop*mainLoop_;
    std::vector<std::unique_ptr<EventLoopThread>>threads_;
    std::vector<EventLoop*>loops_;
};
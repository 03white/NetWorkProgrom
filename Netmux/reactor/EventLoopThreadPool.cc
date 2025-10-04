#include"EventLoopThreadPool.h"
#include<memory>
/***************
 * public method
 **************/
EventLoopThreadPool::EventLoopThreadPool(EventLoop*mainLoop,const std::string &name)
    : started_(false)
    , threadNum_(0)
    , next_(0)
    , name_(name)
    , mainLoop_(mainLoop)
{
}
// the Load Balance we support to outside model
EventLoop* EventLoopThreadPool::getNextLoop(){
    EventLoop*loop=mainLoop_;
    if(!loops_.empty()){
        loop=loops_[next_];
        next_=(next_+1)%loops_.size();//Polling strategy
    }
    return loop;
}

//maybe the most useful method for outside model
void EventLoopThreadPool::start(const ThreadInitCallback &cb){
    started_=true;
    for(int i=0;i<threadNum_;i++){
        char buf[name_.size()+32];
        snprintf(buf,sizeof(buf),"%s%d",name_.c_str(),i);
        EventLoopThread*t=new EventLoopThread(cb,buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
    if(threadNum_==0&&cb){
        cb(mainLoop_);
    }
}

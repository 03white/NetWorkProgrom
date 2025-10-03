#include"EventLoop.h"
#include"./utils/Logger.h"
#include"./reactor/Poller.h"
#include"./reactor/Channel.h"
#include<sys/eventfd.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<memory>
__thread EventLoop *t_loopInThisThread=nullptr;// Thread_local
const int kPollTImeMs=10000;
//create wakeupfd,to notify subReactor to deal with new Channel
int createEventfd(){
    int evtfd=::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
    if(evtfd<0){
        LOG_FATAL("eventfd error:%d\n",errno);
    }
    return evtfd;
}
/********** 
public method
**********/
EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , threadId_(CurrentThread::tid())
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this,wakeupFd_))
    , poller_(Poller::newDefaultPoller(this))
    , callingPendingFunctors_(false)
{
    LOG_DEBUG("EventLoop created %p in thread%d\n",this,threadId_);
    if(t_loopInThisThread){
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n",t_loopInThisThread,threadId_);
    }else{
        t_loopInThisThread=this;
    }
    //
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread=nullptr;
}

void EventLoop::loop(){
    looping_=true;
    quit_=false;
    LOG_INFO("EventLoop %p start loop\n",this);
    while(!quit_){
        activeChannel_.clear();
        //listen two kind of fd,one is client's fd,another is wakeupfd
        pollReturnTime_=poller_->poll(kPollTImeMs,&activeChannel_);
        for(Channel*channel:activeChannel_){
            channel->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping. \n",this);
    looping_=false;
}

void EventLoop::quit(){
    quit_=true; 
    if(!isInLoopThread()){
        wakeup();
    }
}

void EventLoop::wakeup(){
    uint64_t num=1;
    ssize_t n=write(wakeupFd_,&num,sizeof(num));
    if(n!=sizeof(num)){
        LOG_ERROR("EventLoop::wakeup() write %lu bytes instead of 8 \n",n);
    }
}

void EventLoop::updateChannel(Channel*channel){
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel*channel){
    poller_->updateChannel(channel);
}

bool EventLoop::hasChannel(Channel*channel){
    poller_->hasChannel(channel);
}

//one of the most import method,depatch the Functor
void EventLoop::postInLoop(Functor cb){
    if(isInLoopThread()){
        cb();
    }else{
        queueInLoop(cb);
    }
}

// almostly the most essential method,put functor in loop's message queue,and wakeup thread
void EventLoop::queueInLoop(Functor cb){
    {
        std::unique_lock<std::mutex>lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    if(!isInLoopThread()||callingPendingFunctors_){
        wakeup();
    }
}

/********** 
private method
**********/
//cunsume the product from wakeupFd
void EventLoop::handleRead(){
    uint64_t num=1;
    ssize_t n=read(wakeupFd_,&num,sizeof(num));
    if(n!=sizeof(num)){
        LOG_ERROR("EventLoop::wakeup() write %lu bytes instead of 8\n",n);
    }
}

//excute the functor in message queue
void EventLoop::doPendingFunctors(){
    std::vector<Functor>functors;
    callingPendingFunctors_=true;
    {
        std::unique_lock<std::mutex>lock(mutex_);
        functors.swap(pendingFunctors_);
    }    
    for(const Functor &functor:functors){
        functor();
    }
    callingPendingFunctors_=false;
}

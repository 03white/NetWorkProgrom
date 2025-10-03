#include"Thread.h"
#include"CurrentThread.h"
#include<semaphore.h>

Thread::Thread(ThreadFunc func,const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{

}

Thread::~Thread(){


}

void Thread::start(){
    started_=true;
    sem_t sem;
    sem_init(&sem,false,0);//I don't know the function of sem_
    thread_=std::shared_ptr<std::thread>(new std::thread([&](){
        tid_=CurrentThread::tid();
        sem_post(&sem);
        func_();
    }));
    sem_wait(&sem);
}

void Thread::join(){
    joined_=true;
    thread_->join();
}
//private method
void Thread::setDefaultName(){
    int num=++numCreated_;
    if(name_.empty()){
        char buf[32]={0};
        snprintf(buf,sizeof(buf),"Thread%d",num);
        name_=buf;
    }
}    
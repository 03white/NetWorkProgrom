#pragma once
#include"../utils/noncopyable.h"
#include<functional>
#include<thread>
#include<memory>
#include<unistd.h>
#include<string>
#include<atomic>

class Thread:public noncopyable{
public:
    using ThreadFunc=std::function<void()>;
public:
    //constructor and destroyer
    explicit Thread(ThreadFunc,const std::string &name=std::string());
    ~Thread();
    void start();
    void join();
    //getter
    bool started()const{return started_;}
    pid_t tid()const {return tid_;}
    const std::string& name()const{return name_;}
    //static method
    static int numCreated(){return numCreated_;}
private:
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread>thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    static std::atomic_int numCreated_;
    //private method
    void setDefaultName();
};
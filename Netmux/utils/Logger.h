#pragma once
#include<iostream>
#include<string>
#include"noncopyable.h"
#include"Timestamp.h"
// define Logger level INFO ERROR DUBUG FATAL
enum class LOG{
    INFO,
    ERROR,
    FATAL,
    DEBUG
};

#define LOG_INFO(LogmsgFormat,...)\
    do{                         \
        Logger &log=Logger::instance(); \
        log.setLogLevel(LOG::INFO);  \
        char buf[1024]={0};         \
        snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__); \
        log.log(buf);                                \
    }while(0)                   

#define LOG_DEBUG(LogmsgFormat,...)\
    do{                         \
        Logger &log=Logger::instance(); \
        log.setLogLevel(LOG::DEBUG);  \
        char buf[1024]={0};         \
        snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__); \
        log.log(buf);                                \
    }while(0)                   

#define LOG_FATAL(LogmsgFormat,...)\
    do{                         \
        Logger &log=Logger::instance(); \
        log.setLogLevel(LOG::FATAL);  \
        char buf[1024]={0};         \
        snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__); \
        log.log(buf);                                \
    }while(0)                   

#define LOG_ERROR(LogmsgFormat,...)\
    do{                         \
        Logger &log=Logger::instance(); \
        log.setLogLevel(LOG::ERROR);  \
        char buf[1024]={0};         \
        snprintf(buf,1024,LogmsgFormat,##__VA_ARGS__); \
        log.log(buf);                                \
    }while(0)                   


class Logger:public noncopyable{
public:
    static Logger& instance(){
        static Logger log;
        return log; 
    }
    void setLogLevel(LOG logLevel){logLevel_=logLevel;}
    LOG getLogLevel(){return logLevel_;}
    void log(std::string msg);
private:
    LOG logLevel_;
    Logger(){}
};

inline void Logger::log(std::string msg){
    switch(logLevel_){
        case LOG::DEBUG:
            std::cout<<"[DEBUG] : ";
            break;
        case LOG::INFO:
            std::cout<<"[INFO] : ";
            break;
        case LOG::ERROR:
            std::cout<<"[ERROR] : ";
            break;
        case LOG::FATAL:
            std::cout<<"[FATAL] : ";
            break;
        default:
            break;
    }
    std::cout<<Timestamp::now().toString()<<":"<<msg<<std::endl;
}
#pragma once
#include<iostream>
#include<string>
#include<time.h>
class Timestamp{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch){microSecondsSinceEpoch_=microSecondsSinceEpoch;}
    static Timestamp now(){return Timestamp(time(NULL));}
    std::string toString()const;
private:
   int64_t microSecondsSinceEpoch_; 
};
std::string Timestamp::toString()const{
    char buf[128]={0};
    tm *tm_time=localtime(&microSecondsSinceEpoch_);
    snprintf(buf,128,"%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year+1900,
        tm_time->tm_mon+1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec
    );
    return buf;
}
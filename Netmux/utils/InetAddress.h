#pragma once
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string>
class InetAddress{
public:
    //normal constructor 
    explicit InetAddress(uint16_t port=0,std::string ip="0.0.0.0");
    //copy constructor
    explicit InetAddress(const sockaddr_in &addr):addr_(addr){} 
    //getter
    std::string toIp()const;
    std::string toIpPort()const; 
    uint16_t toPort()const;
    const sockaddr_in* getScockAddr()const{return &addr_;}
    //setter
    void setSockAddr(const sockaddr_in &addr){addr_=addr;}
private:
    sockaddr_in addr_;
};
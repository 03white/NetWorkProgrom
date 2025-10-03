#pragma once
#include"../utils/noncopyable.h"
#include"../utils/Timestamp.h"
#include<vector>
#include<unordered_map>

class Channel;
class EventLoop;

class Poller:public noncopyable{
public:
    using ChannelList=std::vector<Channel*>;
    using ChannelMap=std::unordered_map<int,Channel*>;
public:
    //constructor and destoryer
    Poller(EventLoop*loop);
    virtual ~Poller()=default;
    //virtual interfase
    virtual Timestamp poll(int timeoutMs,ChannelList*activeChannels)=0;
    virtual void updateChannel(Channel*channel)=0;
    virtual void removeChannel(Channel*channel)=0;
    //other methods
    bool hasChannel(Channel*channel)const;
    //static factor method
    static Poller* newDefaultPoller(EventLoop*loop);
protected:
    ChannelMap channelHash_;// a HashMap tell us the channel of fd
private:
    EventLoop*ownerLoop_;// oberver /not have
};
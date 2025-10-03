#include"Poller.h"
#include<sys/epoll.h>
#include<vector>

/*********************
epoll_create
epoll_ctl
epool_wairt
*********************/
class Channel;
class EPollPoller:public Poller{
public:
    using EventList=std::vector<epoll_event>;
public:
    //constructor and destroyer
    EPollPoller(EventLoop*loop);
    ~EPollPoller()override;
    //immplement virtual method
    Timestamp poll(int timeoutMs,ChannelList*activeChannels)override;
    void updateChannel(Channel*channel)override;
    void removeChannel(Channel*channel)override;
private:
    int epollfd_;
    EventList Resultevents_;//the container to contain the set of the epoll_event's object(a kind of struct used by linux)
    static const int kInitEventListSize=16;
    //private method
    void fillActiveChannels(int numEvents,ChannelList*activeChannels)const;
    void update(int operation,Channel*channel);
};
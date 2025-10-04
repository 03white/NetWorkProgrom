#include"EPollPoller.h"
#include"../utils/Logger.h"
#include"Channel.h"
#include<errno.h>
#include<unistd.h>
#include<strings.h>

/***********
 satus const var 
 ***********/
const int kNew=-1;
const int kAdded=1;
const int kDeleted=2;
/***********
 public method 
 ***********/
EPollPoller::EPollPoller(EventLoop*loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , Resultevents_(kInitEventListSize)
{
    if(epollfd_<0){
        LOG_FATAL("epoll_create:%d\n",errno);
    }
}

EPollPoller::~EPollPoller(){
    ::close(epollfd_);
}

/*immplement virtual method*/

/*
@author:03white
@target:we call epoll_wait in this method and we checkout the epollfd tree to see
if there are some events have been traggered,then we put information to activeChannels
,give other mudles
*/
Timestamp EPollPoller::poll(int timeoutMs,ChannelList*activeChannels){
    LOG_INFO("func  %s =>fd total count:%u\n",__FUNCTION__,channelHash_.size());
    
    int numEvents=::epoll_wait(epollfd_,&*Resultevents_.begin(),static_cast<int>(Resultevents_.size()),timeoutMs);
    int saveErrno=errno;
    Timestamp now(Timestamp::now());
    if(numEvents>0){
        LOG_INFO("%d events happended \n",numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents==Resultevents_.size()){
            Resultevents_.resize(Resultevents_.size()*2);
        }
    }else if(numEvents==0){
        LOG_DEBUG("%s timeout! \n",__FUNCTION__);
    }else{
        if(saveErrno!=EINTR){
            errno=saveErrno;
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel*channel){
    const int status=channel->status();
    LOG_INFO("func=%s=> fd=%d events=%d status=%d \n",__FUNCTION__,channel->fd(),channel->events(),status);
    if(status==kNew||status==kDeleted){
        if(status==kNew){
            int fd=channel->fd();
            channelHash_[fd]=channel;
        }
        channel->set_status(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }else{  // channel has been registered on poller     
        int fd=channel->fd();
        if(channel->isNonEvent()){
            update(EPOLL_CTL_DEL,channel);
            channel->set_status(kDeleted);
        }else{
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void EPollPoller::removeChannel(Channel*channel){
    int fd=channel->fd();
    channelHash_.erase(fd);
    LOG_INFO("func=%s=> fd=%d\n",__FUNCTION__,fd);
    int status=channel->status();
    if(status==kAdded){
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_status(kNew);
}

/****************
 * prevate method
 ***************/
void EPollPoller::fillActiveChannels(int numEvents,ChannelList*activeChannels)const{
    for(int i=0;i<numEvents;i++){
        Channel*channel=static_cast<Channel*>(Resultevents_[i].data.ptr);
        channel->set_revents(Resultevents_[i].events);
        activeChannels->push_back(channel);// EventLoop get it eventslist
    }
}
void EPollPoller::update(int operation,Channel*channel){
    epoll_event event;
    bzero(&event,sizeof(event));
    int fd=channel->fd();
    event.events=channel->events();
    event.data.fd=fd;
    event.data.ptr=channel;
    if(::epoll_ctl(epollfd_,operation,fd,&event)<0){
        if(operation==EPOLL_CTL_DEL){
            LOG_ERROR("epoll_ctl del error:%d\n",errno);
        }else{
            LOG_FATAL("epoll_ctl_add/mod error:%d\n",errno);
        }

    }
}

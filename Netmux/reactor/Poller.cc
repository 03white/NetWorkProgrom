#include"Poller.h"
#include"Channel.h"
/***************
 * public method
 ***************/
//constructor and destoryer
Poller::Poller(EventLoop*loop):ownerLoop_(loop){

}
bool Poller::hasChannel(Channel*channel)const{
    auto it=channelHash_.find(channel->fd());
    return it!=channelHash_.end()&&it->second==channel;
}
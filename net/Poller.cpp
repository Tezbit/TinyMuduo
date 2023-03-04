//
// Created by ashoreDove on 2023/2/19.
//

#include "Poller.h"
#include "Channel.h"

//Poller *Poller::newDefaultPoller(EventLoop *loop) {
//
//}

bool Poller::hasChannel(Channel *channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {

}

//
// Created by ashoreDove on 2023/2/20.
//
#pragma once
#ifndef TEZBITMUDUO_EPOLLPOLLER_H
#define TEZBITMUDUO_EPOLLPOLLER_H

#include "Poller.h"

#include <vector>
#include <sys/epoll.h>

/*
 * epoll_create epoll_ctl(add/mod/del) epoll_wait
 */
class EPollPoller : public Poller {
public:
    explicit EPollPoller(EventLoop *loop);

    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;

    void updateChannel(Channel *channel) override;

    void removeChannel(Channel *channel) override;

    bool hasChannel(Channel *channel) const override;

private:
    static const int initEventListSize = 16;

    static const char *operationToString(int op);

    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    void update(int op, Channel *channel);

    using EventList = std::vector<epoll_event>;
    int epollfd_;
    EventList events_;

};


#endif //TEZBITMUDUO_EPOLLPOLLER_H

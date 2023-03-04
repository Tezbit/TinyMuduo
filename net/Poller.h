//
// Created by ashoreDove on 2023/2/19.
//
#pragma once
#ifndef TEZBITMUDUO_POLLER_H
#define TEZBITMUDUO_POLLER_H

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "Timestamp.h"

/*
 * 抽象类
 */
class Channel;

class EventLoop;

class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel *>;

    explicit Poller(EventLoop *loop);

    virtual ~Poller() = default; //抽象基类的虚析构函数使析构时正常访问子类的析构（子→父），而不会因为只调用了基类的析构导致的内存泄露
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    virtual bool hasChannel(Channel *channel) const;

    //类似单例模式的getInstance
    //希望提供一个抽象的泛用接口，但返回的Poller对象必须是具体的，基类又不应该include子类
    //中间套了一层DefaultPoller.cpp
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel *>;
    //ChannelMap大小比EventLoop里的ChannelList小，它只关心注册到Poller上的Channel
    ChannelMap channels_;
private:
    EventLoop *ownerLoop_;
};


#endif //TEZBITMUDUO_POLLER_H

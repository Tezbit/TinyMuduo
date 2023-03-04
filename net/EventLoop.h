//
// Created by ashoreDove on 2023/2/19.
//
#pragma once
#ifndef TEZBITMUDUO_EVENTLOOP_H
#define TEZBITMUDUO_EVENTLOOP_H

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include "Channel.h"
#include "Poller.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

//class Channel;
//
//class Poller;

class EventLoop : noncopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();

    ~EventLoop();

    void loop();

    void quit();

    [[nodiscard]] Timestamp pollReturnTime() const { return pollReturnTime_; }

    //在当前loop执行(loop在创建它的线程中)
    void runInLoop(Functor cb);

    //EventLoop如果不在创建它的线程里则把cb回调放入队列中，唤醒loop所在线程执行cb
    void queueInLoop(Functor cb);

    void wakeup();

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool hasChannel(Channel *channel);

    [[nodiscard]] bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }


private:
    void handleRead();

    void doPendingFunctors();

    using ChannelList = std::vector<Channel *>;
    std::atomic_bool looping_;
    std::atomic_bool quit_; //标志退出loop循环
    const pid_t threadId_;
    Timestamp pollReturnTime_; //poller返回的poll方法的发生事件的时间点
    std::unique_ptr<Poller> poller_;
    //当mainLoop获取新用户的Channel，轮询选择一个subLoop后，通过wakeupFd唤醒subLoop
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;
//    Channel* currentActiveChannel_;
    std::atomic_bool callingPendingFunctors_; //标识当前loop是否有需要执行的回调函数
    std::vector<Functor> pendingFunctors_; //存储loop需要执行的所有回调操作
    std::mutex mutex_; //保护vector<Functor>的线程安全
};


#endif //TEZBITMUDUO_EVENTLOOP_H

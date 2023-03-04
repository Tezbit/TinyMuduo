//
// Created by ashoreDove on 2023/2/22.
//
#pragma once
#ifndef TEZBITMUDUO_EVENTLOOPTHREAD_H
#define TEZBITMUDUO_EVENTLOOPTHREAD_H

#include "noncopyable.h"
#include "Thread.h"
#include "EventLoop.h"

#include <mutex>
#include <condition_variable>
#include <string>


class EventLoopThread : noncopyable {
public:
    using ThreadInitCallBack = std::function<void(EventLoop *)>;

    ~EventLoopThread();

    EventLoop *startLoop();

    explicit EventLoopThread(const ThreadInitCallBack &cb = ThreadInitCallBack(), const std::string &name = std::string());

private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallBack callBack_;

};


#endif //TEZBITMUDUO_EVENTLOOPTHREAD_H

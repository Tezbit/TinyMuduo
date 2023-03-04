//
// Created by ashoreDove on 2023/2/22.
//
#pragma once
#ifndef TEZBITMUDUO_EVENTLOOPTHREADPOOL_H
#define TEZBITMUDUO_EVENTLOOPTHREADPOOL_H

#include "noncopyable.h"
#include "EventLoopThread.h"

#include <string>

class EventLoop;
//class EventLoopThread;

class EventLoopThreadPool : noncopyable {
public:

    EventLoopThreadPool(EventLoop *baseLoop, std::string nameArg);

    ~EventLoopThreadPool() = default;;

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    void start(const EventLoopThread::ThreadInitCallBack &cb = EventLoopThread::ThreadInitCallBack());

    //如果工作在多线程中，baseLoop_默认以轮询的方式分配Channel给subLoop
    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    [[nodiscard]] bool started() const { return started_; }

    [[nodiscard]] const std::string &name() const { return name_; }

private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};


#endif //TEZBITMUDUO_EVENTLOOPTHREADPOOL_H

//
// Created by ashoreDove on 2023/2/22.
//

#include "EventLoopThread.h"

#include <memory>

void EventLoopThread::threadFunc() {
    //这里的EventLoop是局部变量，一旦结束工作(走出loop())就会自动析构进行回收
    EventLoop loop;
    if (callBack_) {
        callBack_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

EventLoop *EventLoopThread::startLoop() {
    thread_.start();
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallBack &cb, const std::string &name) : loop_(
        nullptr), exiting_(false),
                                                                                                           thread_(std::bind(
                                                                                                                   &EventLoopThread::threadFunc,
                                                                                                                   this),
                                                                                                                   name),
                                                                                                           callBack_(
                                                                                                                   cb),
                                                                                                           mutex_(),
                                                                                                           cond_() {

}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

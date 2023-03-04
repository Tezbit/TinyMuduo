//
// Created by ashoreDove on 2023/2/22.
//

#include "EventLoopThreadPool.h"

#include <utility>

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    if (loops_.empty()) {
        return std::move(std::vector<EventLoop *>(1, baseLoop_));
    }
    return loops_;
}

//通过轮询获取下一个处理事件的loopThread
EventLoop *EventLoopThreadPool::getNextLoop() {
    EventLoop *loop = baseLoop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

void EventLoopThreadPool::start(const EventLoopThread::ThreadInitCallBack &cb) {
    started_ = true;
    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        auto *t = new EventLoopThread(cb, buf);
        threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
        loops_.emplace_back(t->startLoop());
    }
    if (numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, std::string nameArg) : baseLoop_(baseLoop),
                                                                                            name_(std::move(nameArg)),
                                                                                            started_(false),
                                                                                            numThreads_(0), next_(0) {
}

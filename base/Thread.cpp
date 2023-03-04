//
// Created by ashoreDove on 2023/2/22.
//

#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

#include <memory>
#include <utility>

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, std::string name) : started_(false), joined_(false), tid_(0),
                                                           func_(std::move(func)),
                                                           name_(std::move(name)) {
    setDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_) {
        //分离线程，运行完成后自动回收
        thread_->detach();
    }
}

void Thread::start() {
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::make_shared<std::thread>([&]() {
        //获取线程tid
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();//开一个新线程执行线程函数
    });
    //信号量等待上面的新线程获取到自己的tid
    sem_wait(&sem);
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName() {
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}

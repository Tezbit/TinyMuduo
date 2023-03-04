//
// Created by ashoreDove on 2023/2/19.
//

#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

#include <sys/epoll.h>

const int Channel::noneEvent = 0;
const int Channel::readEvent = EPOLLIN | EPOLLPRI;
const int Channel::writeEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {

}

Channel::~Channel() {
    //析构之前要保证：1.没有在执行的回调 2.调用了remove 3.是在Channel所属的loop以及对应线程内进行的析构
    //todo
}

//fd得到poller通知后处理事件
void Channel::handleEvent(Timestamp receviceTime) {
    //如果TcpConnection注销则不再处理事件
    if (tied_) {
        std::shared_ptr<void> guard;
        guard = tie_.lock();
        if (guard) {
            handleEventWithGuard(receviceTime);
        }
    } else {
        handleEventWithGuard(receviceTime);
    }
}

//绑定TcpConnection
void Channel::tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receviceTime) {
    LOG_INFO("channel handleEvent revents:%d\n", revents_);
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        //fatal
        if (closeEventCallBack_) {
            closeEventCallBack_();
        }
    }
    if (revents_ & EPOLLERR) {
        //error
        if (errorEventCallBack_) {
            errorEventCallBack_();
        }
    }
    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        //read
        if (readEventCallBack_) {}
        readEventCallBack_(receviceTime);
    }
    if (revents_ & EPOLLOUT) {
        //write
        if (writeEventCallBack_) {
            writeEventCallBack_();
        }
    }
}

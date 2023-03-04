//
// Created by ashoreDove on 2023/2/19.
//
#pragma once
#ifndef TEZBITMUDUO_CHANNEL_H
#define TEZBITMUDUO_CHANNEL_H

#include <functional>
#include <memory>

#include "Timestamp.h"
#include "noncopyable.h"

//以指针类型访问时可用前置声明，访问对象则必须include
//编译的时候指针大小是固定的
class EventLoop;

/*
 * 连接socket_fd与对应事件event(感兴趣的,实际发生的)的通道
 */
class Channel : noncopyable {
public:
    using EventCallBack = std::function<void()>;

    using ReadEventCallBack = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);

    ~Channel();

    void handleEvent(Timestamp receviceTime);

    void setReadCallBack(ReadEventCallBack cb) { readEventCallBack_ = std::move(cb); }

    void setWriteCallBack(EventCallBack cb) { writeEventCallBack_ = std::move(cb); }

    void setCloseCallBack(EventCallBack cb) { closeEventCallBack_ = std::move(cb); }

    void setErrorCallBack(EventCallBack cb) { errorEventCallBack_ = std::move(cb); }

    //防止channel被remove时，还在执行回调
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }

    int events() const { return events_; }

    void set_revents(int revt) { revents_ = revt; }

    //设置fd相应的事件状态  并更新到epoll上(需要EventLoop参与)
    void enableReading() {
        events_ |= readEvent;
        update();
    }

    void disableReading() {
        events_ &= ~readEvent;
        update();
    }

    void enableWriting() {
        events_ |= writeEvent;
        update();
    }

    void disableWriting() {
        events_ &= ~writeEvent;
        update();
    }

    void disableAll() {
        events_ = noneEvent;
        update();
    }

    //返回fd当前是事件状态
    //是否有注册感兴趣的事件
    bool isNoneEvent() const { return events_ == noneEvent; }
    //对写事件感兴趣（可写）
    [[nodiscard]] bool isWriting() const { return events_ & writeEvent; }

    [[nodiscard]] bool isReading() const { return events_ & readEvent; }

    //for poller
    [[nodiscard]] int index() const { return index_; }

    void set_index(int idx) { index_ = idx; }

    EventLoop* ownerLoop(){return loop_;}
    void remove();
private:
    static const int noneEvent;
    static const int readEvent;
    static const int writeEvent;

    EventLoop *loop_; //Channel所属的事件循环
    const int fd_;
    int events_; //注册fd感兴趣的事件
    int revents_; //返回的具体发生的事件
    int index_;

    //
    std::weak_ptr<void> tie_;
    bool tied_;

    //fd的回调操作由Channel来做
    ReadEventCallBack readEventCallBack_;
    EventCallBack writeEventCallBack_;
    EventCallBack closeEventCallBack_;
    EventCallBack errorEventCallBack_;

    void update();
    void handleEventWithGuard(Timestamp receviceTime);
};


#endif //TEZBITMUDUO_CHANNEL_H

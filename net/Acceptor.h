//
// Created by ashoreDove on 2023/2/22.
//
#pragma once
#ifndef TEZBITMUDUO_ACCEPTOR_H
#define TEZBITMUDUO_ACCEPTOR_H

#include "noncopyable.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"

class EventLoop;

class Acceptor : noncopyable {
public:
    using NewConnectionCallBack = std::function<void(int sockfd, const InetAddress &)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);

    ~Acceptor();

    void setNewConnectionCallBack(const NewConnectionCallBack &cb) { newConnectionCallBack_ = cb; }

    [[nodiscard]] bool listenning() const { return listenning_; }

    void listen();

private:
    void handleRead();

    EventLoop *loop_; //baseLoop或称作mainLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallBack newConnectionCallBack_;
    bool listenning_;
};


#endif //TEZBITMUDUO_ACCEPTOR_H

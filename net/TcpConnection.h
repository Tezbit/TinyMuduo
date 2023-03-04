//
// Created by ashoreDove on 2023/2/23.
//
#pragma once
#ifndef TEZBITMUDUO_TCPCONNECTION_H
#define TEZBITMUDUO_TCPCONNECTION_H

#include "noncopyable.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"
#include "EventLoop.h"

#include <memory>
#include <string>
#include <atomic>

/*
 * TcpServer → Acceptor → 新连接connfd → TcpConnection设置回调 → Channel → Poller
 */


class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    enum StateE {
        cDisconnected, cConnecting, cConnected, cDisconnecting
    };

    TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                  const InetAddress &peerAddr);

    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }

    const std::string &name() const { return name_; }

    const InetAddress &localAddress() const { return localAddr_; }

    const InetAddress &peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == cConnected; }

    bool disconnected() const { return state_ == cDisconnected; }

    //发送数据
    void send(const std::string &buf);

    void send(Buffer *buf);

//    void send(Buffer &&message);
    //关闭连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

    //连接建立
    void connectEstablished();

    //连接销毁
    void connectDestroyed();

    void setState(StateE state) { state_ = state; }

private:
    void handleRead(Timestamp receiveTime);

    void handleWrite();

    void handleClose();

    void handleError();

    void sendInLoop(const void *message, size_t len);

    void shutdownInLoop();

    EventLoop *loop_; //connLoop
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    ConnectionCallback connectionCallback_; //连接状态变化
    MessageCallback messageCallback_; //有读写消息
    WriteCompleteCallback writeCompleteCallback_; //消息发送完成后
    CloseCallback closeCallback_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};


#endif //TEZBITMUDUO_TCPCONNECTION_H

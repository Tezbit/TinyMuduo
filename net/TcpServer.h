//
// Created by ashoreDove on 2023/2/19.
//
#pragma once
#ifndef TEZBITMUDUO_TCPSERVER_H
#define TEZBITMUDUO_TCPSERVER_H

#include "Acceptor.h"
#include "EventLoopThreadPool.h"
#include "TcpConnection.h"

class TcpServer : noncopyable {
public:
    enum Option {
        noReusePort,
        reusePort
    };

    TcpServer(EventLoop *loop, const InetAddress &listenAddr, std::string nameArg, Option option = noReusePort);

    ~TcpServer();

    void setThreadNum(int numThreads);

    void setThreadInitCallback(const EventLoopThread::ThreadInitCallBack &cb) { threadInitCallBack_ = cb; }

    void start();

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    std::string name() { return name_; }

    std::string ipPort() { return ipPort_; }

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);

    void removeConnection(const TcpConnectionPtr &conn);

    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    EventLoop *loop_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback connectionCallback_; //有新连接
    MessageCallback messageCallback_; //有读写消息
    WriteCompleteCallback writeCompleteCallback_; //消息发送完成后
    EventLoopThread::ThreadInitCallBack threadInitCallBack_; //loop线程初始化
    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_; //保存所有的连接
};


#endif //TEZBITMUDUO_TCPSERVER_H

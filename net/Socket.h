//
// Created by ashoreDove on 2023/2/22.
//
#pragma once
#ifndef TEZBITMUDUO_SOCKET_H
#define TEZBITMUDUO_SOCKET_H

#include "noncopyable.h"
#include "InetAddress.h"

/*
 * 封装Socket Fd
 */
class Socket : noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}

    ~Socket();

    [[nodiscard]] int fd() const { return sockfd_; }

    void bindAddress(const InetAddress &localaddr);

    void listen();

    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);

private:
    const int sockfd_;
};


#endif //TEZBITMUDUO_SOCKET_H

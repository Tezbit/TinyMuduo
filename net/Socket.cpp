//
// Created by ashoreDove on 2023/2/22.
//

#include "Socket.h"
#include "Logger.h"

#include <unistd.h>
#include <strings.h>
#include <netinet/tcp.h>

Socket::~Socket() {
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr) {
    if (0 != ::bind(sockfd_, (sockaddr *) localaddr.getSockAddr(), sizeof(sockaddr_in))) {
        LOG_FATAL("bind sockfd:%d fail \n", sockfd_);
    }
}

void Socket::listen() {
    if (0 != ::listen(sockfd_, 1024)) {
        LOG_FATAL("listen sockfd:%d fail \n", sockfd_);
    }
}

int Socket::accept(InetAddress *peeraddr) {
    sockaddr_in addr{};
    bzero(&addr, sizeof(addr));
    socklen_t len;
    int connfd = ::accept(sockfd_, (sockaddr *) &addr, &len);
    if (connfd >= 0) {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::shutdownWrite() {
    //设置不可写
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR("sockets:shutdownWrite error\n");
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

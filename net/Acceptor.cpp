//
// Created by ashoreDove on 2023/2/22.
//

#include "Acceptor.h"
#include "Logger.h"

#include <cerrno>
#include <unistd.h>

static int createNonblocking() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_FATAL("%s:%s:%d listen socket create err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

void Acceptor::handleRead() {
    //新用户连接→fd→Channel→SubLoop
    InetAddress peerAddr = InetAddress(0);
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallBack_) {
            //轮询并唤醒subLoop,分发Channel
            newConnectionCallBack_(connfd, peerAddr);
        } else {
            ::close(connfd);
        }
    } else {
        LOG_ERROR("%s:%s:%d accept socket create err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE) {
            //文件描述符资源用完了
            LOG_ERROR("%s:%s:%d sockfd reached limit!\n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}

void Acceptor::listen() {
    listenning_ = true;
    acceptSocket_.listen();
    //在这一步将Channel注册到EventLoop的Poller上
    acceptChannel_.enableReading();
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport) : loop_(loop),
                                                                                     acceptSocket_(createNonblocking()),
                                                                                     acceptChannel_(loop,
                                                                                                    acceptSocket_.fd()),
                                                                                     listenning_(false) {
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    //处理新用户连接的回调
    acceptChannel_.setReadCallBack(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

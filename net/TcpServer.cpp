//
// Created by ashoreDove on 2023/2/19.
//

#include "TcpServer.h"
#include "Logger.h"

#include <strings.h>
#include <utility>

static EventLoop *CheckLoopNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL("mainLoop is null! \n");
    }
    return loop;
}


TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, std::string nameArg,
                     TcpServer::Option option) : loop_(CheckLoopNotNull(loop)), ipPort_(listenAddr.toIpPort()),
                                                 name_(std::move(nameArg)),
                                                 acceptor_(new Acceptor(loop, listenAddr, option = reusePort)),
                                                 threadPool_(new EventLoopThreadPool(loop, name_)),
                                                 connectionCallback_(),
                                                 started_(0),
                                                 messageCallback_(), nextConnId_(1) {
    acceptor_->setNewConnectionCallBack(
            std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for (auto &item:connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (started_++ == 0) { //防止一个TcpServer被start多次
        threadPool_->start(threadInitCallBack_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

//对新连接connFd进行处理
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    //根据轮询选择subLoop并唤醒
    EventLoop *ioLoop = threadPool_->getNextLoop();
    //给名字
    char buff[64] = {0};
    snprintf(buff, sizeof(buff), "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buff;
    LOG_INFO("TcpServer::newConnection[%s] - new connection [%s] from %s\n", name_.c_str(), connName.c_str(),
             peerAddr.toIpPort().c_str());
    //通过sockfd获取其绑定的本机ip和端口信息
    sockaddr_in local{};
    bzero(&local, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (getsockname(sockfd, (sockaddr *) &local, &addrlen) < 0) {
        LOG_ERROR("sockets::getLocalAddr");
    }
    InetAddress localAddr(local);
    //创建TcpConnection
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s\n", name_.c_str(), conn->name().c_str());
    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

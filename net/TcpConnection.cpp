//
// Created by ashoreDove on 2023/2/23.
//

#include "TcpConnection.h"
#include "Logger.h"


static EventLoop *CheckLoopNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL("TcpConnection Loop is null! \n");
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr) : loop_(CheckLoopNotNull(loop)), name_(name),
                                                            state_(cConnecting),
                                                            reading_(true), socket_(new Socket(sockfd)),
                                                            channel_(new Channel(loop, sockfd)),
                                                            localAddr_(localAddr), peerAddr_(peerAddr) {
    channel_->setReadCallBack(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnect::dtor[%s] at fd=%d state=%d", name_.c_str(), channel_->fd(), (int) state_);
}

void TcpConnection::send(const std::string &buf) {
    if (state_ == cConnected) {
        //已经建立连接
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            loop_->queueInLoop(std::bind(&TcpConnection::sendInLoop, shared_from_this(), buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == cConnected) {
        setState(cDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::connectEstablished() {
    setState(cConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if (state_ == cConnected) {
        setState(cDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
//        loop_->queueInLoop(std::bind(messageCallback_, shared_from_this(), &inputBuffer_, receiveTime));
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

//send发送数据时，如果内核缓冲区满了则把剩下的数据放入outputBuffer,当内核缓冲区有空闲的时候则触发可写事件调用handleWrite
void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                //无可读 → 全部发送完成（无剩余
                //不再关注可写事件
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    //不论是否唤醒，都会添加入Loop的Functor队列里
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));

                }
                if (state_ == cDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    } else {
        LOG_ERROR("TcpConnection fd=%d is down,no more writing\n", channel_->fd());
    }
}

void TcpConnection::handleClose() {
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n", channel_->fd(), (int) state_);
    setState(cDisconnected);
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR=%d\n", name_.c_str(), err);
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {
        //outputbuffer_中没有残余数据未发送
        //关闭写端会触发Channel的handleClose
        socket_->shutdownWrite();
    }
}


void TcpConnection::sendInLoop(const void *message, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == cDisconnected) {
        LOG_ERROR("disconnected, give up writing");
    }
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        //缓冲区没有待发送数据
        nwrote = ::write(channel_->fd(), message, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }
    if (!faultError && remaining > 0) {
        //写不下了，给OOUPUTBUFFER,设置Channel关注可写事件
        size_t oldLen = outputBuffer_.readableBytes();
        outputBuffer_.append((char *) message + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::send(Buffer *buf) {
    if (state_ == cConnected) {
        //已经建立连接
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        } else {
            std::string s=buf->retrieveAllAsString();
            loop_->queueInLoop(std::bind(&TcpConnection::sendInLoop, shared_from_this(), s.c_str(), s.size()));
        }
    }
}



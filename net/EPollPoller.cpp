//
// Created by ashoreDove on 2023/2/20.
//

#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <cerrno>
#include <unistd.h>
#include <strings.h>

//Channel在Poller中的状态：未添加/已添加/已删除
const int cNew = -1; //对应Channel的成员index_
const int cAdded = 1;
const int cDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop) : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
                                            events_(initEventListSize) {
    if (epollfd_ < 0) {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EPollPoller::~EPollPoller() {
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels) {
    //todo LOG_DEBUG
    LOG_INFO("func=%s => fd total count:%lu\n", __FUNCTION__, channels_.size());
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int savedErr = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        //有事件发生
        LOG_INFO("%d events happened\n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == static_cast<int>(events_.size())) {
            //如果发生是事件数量等于关注的所有事件数量
            //有可能是EventList容量不足，进行扩容
            events_.resize(events_.size() * 2);

        }
    } else if (numEvents == 0) {
        //没有事件发生
        LOG_DEBUG("%s timeout!\n", __FUNCTION__);
    } else {
        if (savedErr != EINTR) {
            //非中断错误
            errno = savedErr;
            LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel) {
    //获取channel在Poller中的状态：cNew/cAdded/cDeleted
    const int index = channel->index();
    int fd = channel->fd();
    LOG_INFO("func=%s fd=%d \n", __FUNCTION__, fd);
    if (index == cNew || index == cDeleted) {
        //Channel不在Poller里
        if (index == cNew) {
            channels_[fd] = channel;
        }
        channel->set_index(cAdded);
        update(EPOLL_CTL_ADD, channel);
    } else { //Channel已经在Poller里注册了
        if (channel->isNoneEvent()) {
            //已经没有感兴趣的事件，所以从epoll移除，不再进行关注
            //但没有进行erase彻底删除，因为走到这边有可能是线程的销毁，程序的结束也有可能是客户端断开连接
            //不进行erase下次建立连接就不需要再添加到channels_里。是一种软删除的做法
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(cDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }

}

void EPollPoller::removeChannel(Channel *channel) {
    int fd = channel->fd();
    const int index = channel->index();
    LOG_INFO("func=%s fd=%d events=%d index=%d \n", __FUNCTION__, fd, channel->events(), index);
    channels_.erase(fd);
    if (index == cAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    //只有wackUpFd和acceptorfd被注销，也就是线程被销毁的时候才会调用这个
    channel->set_index(cNew);
}

bool EPollPoller::hasChannel(Channel *channel) const {
    return Poller::hasChannel(channel);
}

const char *EPollPoller::operationToString(int op) {
    return nullptr;
}

void EPollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        auto *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::update(int op, Channel *channel) {
    epoll_event event{};
    bzero(&event, sizeof(event));
    event.events = channel->events();
    //告诉epoll：fd的绑定对象，这样fd上发生读写事件都能找到对应Channel处理
    event.data.ptr = channel;
    int fd = channel->fd();
    event.data.fd = fd;
    if (::epoll_ctl(epollfd_, op, fd, &event) < 0) {
        if (op == EPOLL_CTL_DEL) {
            //del没有删掉
            LOG_ERROR("epoll_ctl del error:%d \n", errno);
        } else {
            //add或者mod失败，则后续操作无法进行，直接fatal
            LOG_FATAL("epoll_ctl add/mod error:%d \n", errno);
        }
    }
}

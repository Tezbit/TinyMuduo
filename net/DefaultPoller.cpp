//
// Created by ashoreDove on 2023/2/20.
//

#include "Poller.h"
#include "EPollPoller.h"

Poller *Poller::newDefaultPoller(EventLoop *loop) {
    return new EPollPoller(loop);
}

//
// Created by ashoreDove on 2023/2/19.
//

#include "EventLoop.h"
#include "Logger.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <cerrno>

//保证一个线程只能有一个EventLoop对象
__thread EventLoop *t_loopInThisThread = nullptr;
//定义默认IO接口超时时间
const int pollTimeMs = 10000;

//创建wakeupFd，用来notify唤醒subReactor处理新来的Channel
int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_FATAL("eventfd error:%d \n", errno);
    }
    return evtfd;
}


EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

EventLoop::EventLoop() : looping_(false), quit_(false), callingPendingFunctors_(false), threadId_(CurrentThread::tid()),
                         poller_(Poller::newDefaultPoller(this)), wakeupFd_(createEventfd()),
                         wakeupChannel_(new Channel(this, wakeupFd_)) {
    LOG_INFO("EventLoop created %p in thread %d \n", this, threadId_);
    if (t_loopInThisThread) {
        //如果当前线程已经有一个EventLoop
        LOG_FATAL("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, threadId_);
    } else {
        t_loopInThisThread = this;
    }
    //通过给wakeupFd发送读事件来唤醒阻塞在poll方法的subReactor
    wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    //设置对读事件感兴趣
    wakeupChannel_->enableReading();
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping \n", this);
    while (!quit_) {
        activeChannels_.clear();
        //监听两类fd:一种是clientfd，一种是wakeupfd
        pollReturnTime_ = poller_->poll(pollTimeMs, &activeChannels_);
        //对于subEventLoop来说，activeChannels需要处理的事件只有wakeup的唤醒读事件
        //真正需要它去处理的Channel回调在vector<Functor>里
        for (Channel *channel:activeChannels_) {
            channel->handleEvent(pollReturnTime_);
        }
        //mainLoop 派发新的Channel以及回调，需要subReactor将其fd注册到poll上
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping. \n", this);
    looping_ = false;
}

void EventLoop::quit() {
    //如果EventLoop在自己线程中调用quit方法则表明EventLoop并没有阻塞在poll
    //反之如果是在其他loop中调用了quit方法说明quit对应的EventLoop需要被唤醒
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}


void EventLoop::runInLoop(EventLoop::Functor cb) {
    //使用场景：TcpServer::start()
    //非常确定：1.在对应线程中且loop并没有在处理上一轮回调 2.一定不在对应线程中
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(EventLoop::Functor cb) {
    //queueInLoop两种被使用情况：1.不在loop所在线程调用该方法 2.在loop所在线程调用但并不清楚loop的工作情况
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    //当callingPendingFunctors_=false的时候 ，且调用在loop所在线程内，说明尚未执行到doPendingFunctors，只需要 添加到vector中等待doPendingFunctors处理就好了，并不需要唤醒
    //只有调用在loop所在线程内，且当前线程正在处理上一轮回调队列的时候才需要 wakeup写入 ，使得上一轮处理完以后不会阻塞 在poll而是尽快处理新的回调队列
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

//向EventLoop的wakeupFd写数据唤醒
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::wakeup() writes %zd bytes instead of utf-8", n);
    }
}

void EventLoop::updateChannel(Channel *channel) {
    poller_->updateChannel(channel);
}

//只有acceptorChannel和wakeupChannel用
void EventLoop::removeChannel(Channel *channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return poller_->hasChannel(channel);
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::handleRead() reads %zd bytes instead of utf-8", n);
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        //空瓶换满瓶
        functors.swap(pendingFunctors_);
    }
    for (const Functor &func:functors) {
        func();
    }
    callingPendingFunctors_ = false;
}

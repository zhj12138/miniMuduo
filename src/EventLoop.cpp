#include "EventLoop.hpp"

#include "Channel.hpp"
#include "Poller.hpp"
#include "EPoller.hpp"
#include "TimerQueue.hpp"

#include <glog/logging.h>
#include <cassert>
#include <poll.h>
#include <csignal>
#include <sys/eventfd.h>

using namespace mymuduo;

/* 一个线程一个EventLoop */
thread_local EventLoop *t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

static int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG(ERROR) << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
  }
};

IgnoreSigPipe initObj;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(std::this_thread::get_id()),
      poller_(new PollerImpl(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)) {
  LOG(INFO) << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG(FATAL) << "Another EventLoop " << t_loopInThisThread
               << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback([this](auto &&PH1) { handleRead(std::forward<decltype(PH1)>(PH1)); });
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  assert(!looping_);
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    for (const auto &channel: activeChannels_) {
      channel->handleEvent(pollReturnTime_);
    }
    doPendingFunctors();  // 调用pending functions
  }

  LOG(INFO) << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(const Functor &cb) {
  if (isInLoopThread()) { // 在当前IO线程中调用runInLoop
    cb(); // 同步调用该函数
  } else {  // 在其他线程中调用runInLoop
    queueInLoop(cb);  // 将cb加入到EventLoop的pending functions中，然后尝试唤醒该EventLoop对象
  }
}

void EventLoop::queueInLoop(const Functor &cb) {
  {
    std::lock_guard<std::mutex> lg(mutex_);
    pendingFunctors_.push_back(cb);
  }
  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

TimerId EventLoop::runAt(const time_point &time, const TimerCallback &cb) {
  return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback &cb) {
  return runAt(get_now() + to_microseconds(delay), cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback &cb) {
  return timerQueue_->addTimer(cb, get_now() + to_microseconds(interval), interval);
}

void EventLoop::cancel(TimerId timerId) {
  return timerQueue_->cancel(timerId);
}

void EventLoop::updateChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
             << " was created in threadId_ = " << threadId_
             << ", current thread id = " << std::this_thread::get_id();
}

void EventLoop::wakeup() const {
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG(ERROR) << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead(time_point _receiveTime) const {
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG(ERROR) << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;
  {
    std::lock_guard<std::mutex> lg(mutex_);
    functors.swap(pendingFunctors_);
  }
//  LOG(INFO) << functors.size() << " pending function(s) going to be called\n";
  for (auto &functor : functors) {
    functor();
  }
  callingPendingFunctors_ = false;
}

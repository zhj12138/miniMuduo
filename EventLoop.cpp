#include "EventLoop.hpp"
#include "Channel.hpp"
#include "Poller.hpp"
#include "TimerQueue.hpp"

#include <glog/logging.h>
#include <cassert>
#include <poll.h>

using namespace mymuduo;

thread_local EventLoop *t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      threadId_(std::this_thread::get_id()),
      poller_(new Poller(this)),
      timerQueue_(new TimerQueue(this)) {
  LOG(INFO) << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG(FATAL) << "Another EventLoop " << t_loopInThisThread
               << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
}

EventLoop::~EventLoop() {
  assert(!looping_);
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
      channel->handleEvent();
    }
  }

  LOG(INFO) << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
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

void EventLoop::updateChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
             << " was created in threadId_ = " << threadId_
             << ", current thread id = " << std::this_thread::get_id();
}



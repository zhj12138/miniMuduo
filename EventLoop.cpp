#include "EventLoop.hpp"
#include <glog/logging.h>
#include <cassert>
#include <poll.h>

using namespace mymuduo;

thread_local EventLoop *t_loopInThisThread = nullptr;

EventLoop::EventLoop()
    : looping_(false),
      threadId_(std::this_thread::get_id()) {
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
  ::poll(nullptr, 0, 5 * 1000);
  LOG(INFO) << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::abortNotInLoopThread() {
  LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
             << " was created in threadId_ = " << threadId_
             << ", current thread id = " << std::this_thread::get_id();
}



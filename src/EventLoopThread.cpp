#include "EventLoopThread.hpp"
#include "EventLoop.hpp"

#include <cassert>

using namespace mymuduo;

EventLoopThread::EventLoopThread()
    : loop_(nullptr),
      exiting_(false),
      thread_([this] { threadFunc(); }) {
}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  loop_->quit();
  thread_.join();
}

EventLoop *EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();
  {
    std::unique_lock<std::mutex> ul(mutex_);
    cond_.wait(ul, [this] { return loop_ != nullptr; });
  }
  return loop_;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;
  {
    std::lock_guard<std::mutex> lg(mutex_);
    loop_ = &loop;
  }
  cond_.notify_one();
  loop.loop();
}

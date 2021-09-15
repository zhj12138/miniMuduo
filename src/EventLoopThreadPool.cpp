#include "EventLoopThreadPool.hpp"

#include "EventLoop.hpp"
#include "EventLoopThread.hpp"

#include <cassert>

using namespace mymuduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(0),
      next_(0) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
  // Don't delete loop
}

void EventLoopThreadPool::start() {
  assert(!started_);
  baseLoop_->assertInLoopThread();
  started_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    auto t = std::make_shared<EventLoopThread>();
    threads_.push_back(t);
    loops_.push_back(t->startLoop());
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  auto loop = baseLoop_;
  if (!loops_.empty()) {  // 轮询
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

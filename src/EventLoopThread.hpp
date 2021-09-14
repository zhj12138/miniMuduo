#ifndef MYMUDUO_SRC_EVENTLOOPTHREAD_HPP_
#define MYMUDUO_SRC_EVENTLOOPTHREAD_HPP_

#include "noncopyable.hpp"
#include "Thread.hpp"
#include <mutex>
#include <condition_variable>

namespace mymuduo {

class EventLoop;

class EventLoopThread : noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop *startLoop();
 private:
  void threadFunc();

  EventLoop *loop_;
  bool exiting_;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

}
#endif //MYMUDUO_SRC_EVENTLOOPTHREAD_HPP_

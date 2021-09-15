#ifndef MYMUDUO__EVENTLOOPTHREADPOOL_HPP_
#define MYMUDUO__EVENTLOOPTHREADPOOL_HPP_

#include <vector>
#include <memory>

namespace mymuduo {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  explicit EventLoopThreadPool(EventLoop *baseLoop);
  ~EventLoopThreadPool();

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start();
  EventLoop *getNextLoop();

 private:
  EventLoop *baseLoop_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::shared_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_;
};

}
#endif //MYMUDUO__EVENTLOOPTHREADPOOL_HPP_

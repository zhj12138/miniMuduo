#ifndef MYMUDUO__THREAD_HPP_
#define MYMUDUO__THREAD_HPP_

#include <future>
#include <thread>
#include <utility>

namespace mymuduo {

class Thread {
 public:
  using ThreadFunc = std::function<void ()>;
  explicit Thread(ThreadFunc func) : func_(std::move(func)) {}
  void start();
  void join();
  bool started() const { return started_; }
 private:
  ThreadFunc func_;
  std::thread thread_;
  bool started_{false};
};

}
#endif //MYMUDUO__THREAD_HPP_

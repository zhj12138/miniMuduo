#ifndef MYMUDUO_SRC_THREAD_HPP_
#define MYMUDUO_SRC_THREAD_HPP_

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
#endif //MYMUDUO_SRC_THREAD_HPP_

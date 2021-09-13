#ifndef MYMUDUO__EVENTLOOP_HPP_
#define MYMUDUO__EVENTLOOP_HPP_

#include <boost/core/noncopyable.hpp>
#include <thread>

namespace mymuduo {

class EventLoop : boost::noncopyable {
 public:
  EventLoop();
  ~EventLoop();

  void loop();

  bool isInLoopThread() const {
    return threadId_ == std::this_thread::get_id();
  }
  void assertInLoopThread() {
    if (!isInLoopThread()) {
      abortNotInLoopThread();
    }
  }
 private:
  void abortNotInLoopThread();

  bool looping_;  // atomic
  const std::thread::id threadId_;
};

}
#endif //MYMUDUO__EVENTLOOP_HPP_

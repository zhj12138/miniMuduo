#ifndef MYMUDUO__EVENTLOOP_HPP_
#define MYMUDUO__EVENTLOOP_HPP_

#include <thread>
#include <vector>
#include "noncopyable.hpp"

namespace mymuduo {

class Channel;
class Poller;

class EventLoop : noncopyable {
 public:
  EventLoop();
  ~EventLoop();

  void loop();

  void quit();

  // internal use only
  void updateChannel(Channel *channel);

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

  using ChannelVec = std::vector<Channel *>;

  bool looping_;  // atomic
  bool quit_;     // atomic
  const std::thread::id threadId_;
  std::unique_ptr<Poller> poller_;
  ChannelVec activeChannels_;
};

}
#endif //MYMUDUO__EVENTLOOP_HPP_

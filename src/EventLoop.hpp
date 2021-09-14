#ifndef MYMUDUO__EVENTLOOP_HPP_
#define MYMUDUO__EVENTLOOP_HPP_

#include <thread>
#include <vector>
#include "noncopyable.hpp"
#include "TimeUtil.hpp"
#include "TimerId.hpp"
#include "Callbacks.hpp"

namespace mymuduo {

class Channel;
class Poller;
class TimerQueue;

class EventLoop : noncopyable {
 public:
  EventLoop();
  ~EventLoop();

  void loop();

  void quit();

  time_point pollReturnTime() const { return pollReturnTime_; };
  TimerId runAt(const time_point &time, const TimerCallback &cb);
  TimerId runAfter(double delay, const TimerCallback &cb);
  TimerId runEvery(double interval, const TimerCallback &cb);

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
  time_point pollReturnTime_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;
  ChannelVec activeChannels_;
};

}
#endif //MYMUDUO__EVENTLOOP_HPP_

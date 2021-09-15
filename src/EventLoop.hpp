#ifndef MYMUDUO__EVENTLOOP_HPP_
#define MYMUDUO__EVENTLOOP_HPP_

#include <thread>
#include <vector>
#include <mutex>
#include "noncopyable.hpp"
#include "TimeUtil.hpp"
#include "TimerId.hpp"
#include "Callbacks.hpp"

namespace mymuduo {

class Channel;
class Poller;
class EPoller;
class TimerQueue;

using PollerImpl = EPoller;

class EventLoop : noncopyable {
 public:
  using Functor = std::function<void ()>;

  EventLoop();
  ~EventLoop();

  void loop();

  void quit();

  time_point pollReturnTime() const { return pollReturnTime_; };

  void runInLoop(const Functor &cb);
  void queueInLoop(const Functor &cb);

  TimerId runAt(const time_point &time, const TimerCallback &cb);
  TimerId runAfter(double delay, const TimerCallback &cb);
  TimerId runEvery(double interval, const TimerCallback &cb);

  void cancel(TimerId timerId);

  // internal use only
  void wakeup() const;
  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);

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
  void handleRead(time_point receiveTime) const;  // waked up
  void doPendingFunctors();

  using ChannelVec = std::vector<Channel *>;

  bool looping_;  // atomic
  bool quit_;     // atomic
  bool callingPendingFunctors_; // atomic
  const std::thread::id threadId_;
  time_point pollReturnTime_;
  std::unique_ptr<PollerImpl> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;
  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_;
  ChannelVec activeChannels_;
  std::mutex mutex_;
  std::vector<Functor> pendingFunctors_;  // @Guarded by mutex
};

}
#endif //MYMUDUO__EVENTLOOP_HPP_

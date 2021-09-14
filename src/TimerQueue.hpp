#ifndef MYMUDUO__TIMERQUEUE_HPP_
#define MYMUDUO__TIMERQUEUE_HPP_

#include <vector>
#include <set>

#include "noncopyable.hpp"
#include "TimeUtil.hpp"
#include "Channel.hpp"
#include "Callbacks.hpp"

namespace mymuduo {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : noncopyable {
 public:
  explicit TimerQueue(EventLoop *loop);
  ~TimerQueue();

  TimerId addTimer(const TimerCallback &cb, time_point when, double interval);

 private:
  using Entry = std::pair<time_point, Timer *>;
  using TimerSet = std::set<Entry>;
  using EntryVec = std::vector<Entry>;

  void handleRead();  // called when timerfd alarms
  EntryVec getExpired(time_point now);  // move out all expired timers
  void reset(const EntryVec &expired, time_point now);

  bool insert(Timer *timer);

  EventLoop *loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  TimerSet timers_;
};

}
#endif //MYMUDUO__TIMERQUEUE_HPP_

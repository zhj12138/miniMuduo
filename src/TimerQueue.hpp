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

  void cancel(TimerId timerId);

 private:
  using Entry = std::pair<time_point, Timer *>;
  using TimerSet = std::set<Entry>;
  using EntryVec = std::vector<Entry>;
  using ActiveTimer = std::pair<Timer *, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void addTimerInLoop(Timer *timer);
  void cancelInLoop(TimerId timerId);
  void handleRead(time_point receiveTime);  // called when timerfd alarms
  EntryVec getExpired(time_point now);  // move out all expired timers
  void reset(const EntryVec &expired, time_point now);

  bool insert(Timer *timer);

  EventLoop *loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  TimerSet timers_;

  // for cancel()
  bool callingExpiredTimers_;
  ActiveTimerSet activeTimers_;
  ActiveTimerSet cancelingTimers_;
};

}
#endif //MYMUDUO__TIMERQUEUE_HPP_

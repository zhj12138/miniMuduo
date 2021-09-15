#include "TimerQueue.hpp"
#include "EventLoop.hpp"
#include "Timer.hpp"

#include <sys/timerfd.h>
#include <glog/logging.h>
#include <cassert>

namespace mymuduo {

namespace detail {

int createTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOG(FATAL) << "Failed in timerfd_create";
  }
  return timerfd;
}

struct timespec howMuchTimeFromNow(time_point when) {
  int64_t microseconds = std::chrono::duration_cast<std::chrono::microseconds>(when - get_now()).count();
  if (microseconds < 100) {
    microseconds = 100;
  }
  struct timespec ts{};
  ts.tv_sec = static_cast<time_t>(microseconds / MicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>((microseconds % MicroSecondsPerSecond) * 1000);
  return ts;
}

void readTimerfd(int timerfd, time_point now) {
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
  LOG(INFO) << "TimerQueue::handleRead() " << howmany << " at " << to_string(now);
  if (n != sizeof(howmany)) {
    LOG(ERROR) << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, time_point expiration) {
  struct itimerspec newValue{};
  struct itimerspec oldValue{};
  bzero(&newValue, sizeof(newValue));
  bzero(&oldValue, sizeof(oldValue));
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret) {
    LOG(ERROR) << "timerfd_settime()";
  }
}

}
}

using namespace mymuduo;
using namespace mymuduo::detail;

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false) {
  timerfdChannel_.setReadCallback([this](auto &&PH1) { handleRead(std::forward<decltype(PH1)>(PH1)); });
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
  ::close(timerfd_);
  for (auto &timer: timers_) {
    delete timer.second;
  }
}

TimerId TimerQueue::addTimer(const TimerCallback &cb, time_point when, double interval) {
  auto timer = new Timer(cb, when, interval);
  loop_->runInLoop([this, timer] { addTimerInLoop(timer); });
  return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId) {
  loop_->runInLoop([this, timerId] { cancelInLoop(timerId); });
}

void TimerQueue::addTimerInLoop(Timer *timer) {
  loop_->assertInLoopThread();
  bool earliestChanged = insert(timer);
  if (earliestChanged) {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  ActiveTimer timer(timerId.timer_, timerId.seq_);
  auto it = activeTimers_.find(timer);
  if (it != activeTimers_.end()) {
    size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
    assert(n == 1);
    (void) n;
    delete it->first;
    activeTimers_.erase(it);
  } else if (callingExpiredTimers_) {
    cancelingTimers_.insert(timer);
  }
  assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead(time_point _receiveTime) {
  loop_->assertInLoopThread();
  time_point now(get_now());
  readTimerfd(timerfd_, now);

  EntryVec expired = getExpired(now);

  callingExpiredTimers_ = true;
  cancelingTimers_.clear();
  for (auto &entry : expired) {
    entry.second->run();
  }
  callingExpiredTimers_ = false;

  reset(expired, now);
}

TimerQueue::EntryVec TimerQueue::getExpired(time_point now) {
  assert(timers_.size() == activeTimers_.size());
  EntryVec expired;
  Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
  auto it = timers_.lower_bound(sentry);
  assert(it == timers_.end() || now < it->first);
  std::copy(timers_.begin(), it, std::back_inserter(expired));
  timers_.erase(timers_.begin(), it);

  for (auto entry: expired) {
    ActiveTimer timer(entry.second, entry.second->sequence());
    size_t n = activeTimers_.erase(timer);
    assert(n == 1);
    (void) n;
  }

  assert(timers_.size() == activeTimers_.size());
  return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, time_point now) {
  time_point nextExpire;
  for (auto &ex: expired) {
    ActiveTimer timer(ex.second, ex.second->sequence());
    if (ex.second->repeat() &&
        cancelingTimers_.find(timer) == cancelingTimers_.end()) {
      ex.second->restart(now);
      insert(ex.second);
    } else {
      delete ex.second;
    }
  }
  if (!timers_.empty()) {
    nextExpire = timers_.begin()->second->expiration();
  }
  if (nextExpire != time_point{}) {
    resetTimerfd(timerfd_, nextExpire);
  }
}

bool TimerQueue::insert(Timer *timer) {
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  bool earliestChanged = false;
  time_point when = timer->expiration();
  auto it = timers_.begin();
  if (it == timers_.end() || when < it->first) {
    earliestChanged = true;
  }
  {
    auto result = timers_.insert(std::make_pair(when, timer));
    assert(result.second);
  }
  {
    auto result = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second);
  }

  assert(timers_.size() == activeTimers_.size());
  return earliestChanged;
}

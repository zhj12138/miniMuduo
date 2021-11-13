#ifndef MYMUDUO__TIMER_HPP_
#define MYMUDUO__TIMER_HPP_

#include <utility>
#include <atomic>

#include "noncopyable.hpp"
#include "Callbacks.hpp"
#include "TimeUtil.hpp"

namespace mymuduo {

class Timer : noncopyable {
 public:
  Timer(TimerCallback cb, time_point when, double interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0),
        sequence_(++s_numCreated_) {
  }

  void run() const { callback_(); }

  time_point expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; }

  void restart(time_point now);

 private:
  const TimerCallback callback_;  // 回调函数
  time_point expiration_; // 过期时间
  const double interval_; // 重复的时间间隔
  const bool repeat_;     // 是否重复
  const int64_t sequence_;// 代表第sequence_个创建的Timer

  static std::atomic<int64_t> s_numCreated_;
};

}

#endif //MYMUDUO__TIMER_HPP_

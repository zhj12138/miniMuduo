#ifndef MYMUDUO__TIMERID_HPP_
#define MYMUDUO__TIMERID_HPP_

namespace mymuduo {

class Timer;

class TimerId {
 public:
  explicit TimerId(Timer *timer = nullptr, int64_t seq = 0)
  : timer_(timer),
    seq_(seq) {
  }
  friend class TimerQueue;
 private:
  Timer *timer_;
  int64_t seq_;
};

}

#endif //MYMUDUO__TIMERID_HPP_

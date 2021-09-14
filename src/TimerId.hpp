#ifndef MYMUDUO__TIMERID_HPP_
#define MYMUDUO__TIMERID_HPP_

namespace mymuduo {

class Timer;

class TimerId {
 public:
  explicit TimerId(Timer *timer) : value_(timer) {}
 private:
  Timer *value_;
};

}

#endif //MYMUDUO__TIMERID_HPP_

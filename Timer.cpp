#include "Timer.hpp"

using namespace mymuduo;

void Timer::restart(time_point now) {
  if (repeat_) {
    expiration_ = now + to_microseconds(interval_);
  } else {
    expiration_ = time_point{};
  }
}

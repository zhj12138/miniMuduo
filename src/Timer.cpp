#include "Timer.hpp"

using namespace mymuduo;

std::atomic<int64_t> Timer::s_numCreated_;

void Timer::restart(time_point now) {
  if (repeat_) {
    expiration_ = now + to_microseconds(interval_);
  } else {
    expiration_ = time_point{};
  }
}

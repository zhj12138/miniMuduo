#include "TimeUtil.hpp"

namespace mymuduo {

time_point get_now() {
  return std::chrono::system_clock::now();
}

std::chrono::microseconds to_microseconds(double seconds) {
  return std::chrono::microseconds(static_cast<int64_t>(seconds * MicroSecondsPerSecond));
}

std::string to_string(const time_point &tp) {
  std::time_t t = std::chrono::system_clock::to_time_t(tp);
  std::string ts = std::ctime(&t);
  ts.resize(ts.size() - 1); // skip newline

  return ts;
}

std::string now_string() {
  return to_string(get_now());
}

}
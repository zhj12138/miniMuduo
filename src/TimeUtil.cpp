#include "TimeUtil.hpp"
#include <iomanip>
#include <sstream>

namespace mymuduo {

time_point get_now() {
  return std::chrono::system_clock::now();
}

std::chrono::microseconds to_microseconds(double seconds) {
  return std::chrono::microseconds(static_cast<int64_t>(seconds * MicroSecondsPerSecond));
}

std::string to_string(const time_point &tp) {
  auto tt = std::chrono::system_clock::to_time_t(tp);
  auto microseconds =
      std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()) % MicroSecondsPerSecond;
  std::stringstream ss;
  ss << std::put_time(std::localtime(&tt), "%Y%m%d %H:%M:%S")
     << "." << std::setfill('0') << std::setw(6) << microseconds.count();
  return ss.str();
}

std::string now_string() {
  return to_string(get_now());
}

}
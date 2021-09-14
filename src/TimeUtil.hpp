#ifndef MYMUDUO__TIMEUTIL_HPP_
#define MYMUDUO__TIMEUTIL_HPP_

#include <chrono>
#include <string>

namespace mymuduo {

using time_point = std::chrono::system_clock::time_point;

const int MicroSecondsPerSecond = 1000 * 1000;

time_point get_now();
std::chrono::microseconds to_microseconds(double seconds);
std::string to_string(const time_point &tp);
std::string now_string();

}

#endif //MYMUDUO__TIMEUTIL_HPP_

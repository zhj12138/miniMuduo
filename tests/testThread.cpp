#include <thread>
#include <future>
#include <unistd.h>
#include <iostream>
#include "../src/TimeUtil.hpp"

using namespace mymuduo;

void run5seconds() {
  sleep(5);
}

int main() {
//  std::cout << now_string() << "\n";
//  auto ft1 = std::async(std::launch::deferred, run5seconds);
//  std::cout << now_string() << "\n";
//  auto ft2 = std::async(std::launch::async, run5seconds);
//  std::cout << now_string() << "\n";
//  ft1.get();
  std::cout << now_string() << "\n";
  std::packaged_task<void ()> pt(run5seconds);
  std::cout << now_string() << "\n";
  auto f = pt.get_future();
  std::cout << now_string() << "\n";
  std::thread t(std::move(pt));
  std::cout << now_string() << "\n";
  sleep(2);
  std::cout << now_string() << "\n";
  t.join();
  std::cout << now_string() << "\n";
  return 0;
}


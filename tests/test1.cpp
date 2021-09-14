#include "EventLoop.hpp"
#include <iostream>
#include <unistd.h>

void threadFunc() {
  std::cout << "threadFunc(): pid = " << getpid()
            << ", tid = " << std::this_thread::get_id() << "\n";
  mymuduo::EventLoop loop;
  loop.loop();
}

int main() {
  std::cout << "main(): pid = " << getpid()
            << ", tid = " << std::this_thread::get_id() << "\n";
  mymuduo::EventLoop loop;

  std::thread t(threadFunc);
  loop.loop();
  pthread_exit(nullptr);
}

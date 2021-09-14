#include "EventLoop.hpp"
#include "EventLoopThread.hpp"
#include <iostream>
#include <unistd.h>

void runInThread() {
  std::cout << "runInThread(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << "\n";
}

int main() {
  std::cout << "main(): pid = " << getpid() << ", tid = " << std::this_thread::get_id() << "\n";
  mymuduo::EventLoopThread loopThread;
  auto loop = loopThread.startLoop();
  loop->runInLoop(runInThread);
  sleep(1);
  loop->runAfter(2, runInThread);
  sleep(3);
  loop->quit();
  std::cout << "exit main().\n";
  return 0;
}


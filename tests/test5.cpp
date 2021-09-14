#include "EventLoop.hpp"
#include <iostream>
#include <unistd.h>

mymuduo::EventLoop *g_loop;
int g_flag = 0;

void run4() {
  std::cout << "run4(): pid = " << getpid() << ", flag = " << g_flag << "\n";
  g_loop->quit();
}

void run3() {
  std::cout << "run3(): pid = " << getpid() << ", flag = " << g_flag << "\n";
  g_loop->runAfter(3, run4);
  g_flag = 3;
}

void run2() {
  std::cout << "run2(): pid = " << getpid() << ", flag = " << g_flag << "\n";
  g_loop->queueInLoop(run3);
}

void run1() {
  g_flag = 1;
  std::cout << "run1(): pid = " << getpid() << ", flag = " << g_flag << "\n";
  g_loop->runInLoop(run2);
  g_flag = 2;
}

int main() {
  std::cout << "main(): pid = " << getpid() << ", flag = " << g_flag << "\n";
  mymuduo::EventLoop loop;
  g_loop = &loop;

  loop.runAfter(2, run1);
  loop.loop();

  std::cout << "main(): pid = " << getpid() << ", flag = " << g_flag << "\n";
  return 0;
}

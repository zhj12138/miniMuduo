#include "EventLoop.hpp"
#include <iostream>
#include <unistd.h>

int cnt = 0;
mymuduo::EventLoop *g_loop;

void printTid() {
  std::cout << "pid = " << getpid() << ", tid = " << std::this_thread::get_id() << "\n";
  std::cout << "now " << mymuduo::now_string() << "\n";
}

void print(const char *msg) {
  std::cout << "msg " << mymuduo::now_string() << " " << msg << "\n";
  if (++cnt == 20) {
    g_loop->quit();
  }
}

mymuduo::TimerId toCancel;
void cancelSelf() {
  print("cancelSelf()");
  g_loop->cancel(toCancel);
}

int main() {
  printTid();
  mymuduo::EventLoop loop;
  g_loop = &loop;
  print("main");
  loop.runAfter(1, [] { return print("once1"); });
  loop.runAfter(1.5, [] { return print("once1.5"); });
  loop.runAfter(2.5, [] { return print("once2.5"); });
  loop.runAfter(3.5, [] { return print("once3.5"); });
  mymuduo::TimerId t = loop.runEvery(2, [] { return print("every2"); });
  loop.runEvery(3, [] { return print("every3"); });
  toCancel = loop.runEvery(5, cancelSelf);

  loop.loop();
  print("main loop exits");
  sleep(1);
  return 0;
}

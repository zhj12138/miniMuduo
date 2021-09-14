#include "../src/EventLoop.hpp"

mymuduo::EventLoop *g_loop;

void threadFunc() {
  g_loop->loop();
}

int main() {
  mymuduo::EventLoop loop;
  g_loop = &loop;
  std::thread t(threadFunc);
  t.join();
}

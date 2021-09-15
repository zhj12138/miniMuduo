#include "Channel.hpp"
#include "EventLoop.hpp"

#include <iostream>
#include <sys/timerfd.h>
#include <cstring>
#include <unistd.h>

mymuduo::EventLoop *g_loop;

void timeout(mymuduo::time_point receiveTime) {
  std::cout << mymuduo::to_string(receiveTime) << " Timeout!\n";
  g_loop->quit();
}

int main() {
  mymuduo::EventLoop loop;
  g_loop = &loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  mymuduo::Channel channel(&loop, timerfd);
  channel.setReadCallback(timeout);
  channel.enableReading();

  struct itimerspec howlong{};
  bzero(&howlong, sizeof(howlong));
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &howlong, nullptr);

  loop.loop();
  ::close(timerfd);
}

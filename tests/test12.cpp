#include "Connector.hpp"
#include "EventLoop.hpp"

#include <iostream>

mymuduo::EventLoop *g_loop;

void connectCallback(int sockfd) {
  std::cout << "connected.\n";
  g_loop->quit();
}

int main(int argc, char *argv[]) {
  mymuduo::EventLoop loop;
  g_loop = &loop;
  mymuduo::InetAddress addr("127.0.0.1", 9987);
  auto connector = std::make_shared<mymuduo::Connector>(&loop, addr);
  connector->setNewConnectionCallback(connectCallback);
  connector->start();
  loop.loop();
  return 0;
}

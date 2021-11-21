#include "TcpServer.hpp"
#include "EventLoop.hpp"
#include "InetAddress.hpp"
#include <iostream>
#include <unistd.h>

std::string message1, message2;

void onConnection(const mymuduo::TcpConnectionPtr &conn) {
  if (conn->connected()) {
    std::cout << "onConnection: tid=" << std::this_thread::get_id() << " new connection [" << conn->name()
              << "] from " << conn->peerAddress().toHostPort() << "\n";
    if (!message1.empty()) conn->send(message1);
    if (!message2.empty()) conn->send(message2);
    conn->shutdown();
  } else {
    std::cout << "onConnection: tid=" << std::this_thread::get_id() << " connection [" << conn->name()
              << "] is done\n";
  }
}

void onMessage(const mymuduo::TcpConnectionPtr &conn,
               mymuduo::Buffer *buf,
               mymuduo::time_point receiveTime) {
  std::cout << "onMessage(): tid=" << std::this_thread::get_id() << " received " << buf->readableBytes()
            << " bytes from connection ["
            << conn->name() << "] at " << mymuduo::to_string(receiveTime) << "\n";
//  conn->send(buf->retrieveAsString());
  buf->retrieveAll();
}

int main(int argc, char *argv[]) {
  std::cout << "main(): pid = %d\n" << getpid();

  int len1 = 100, len2 = 100;
  if (argc > 2) {
    len1 = atoi(argv[1]);
    len2 = atoi(argv[2]);
  }
  message1.resize(len1);
  message2.resize(len2);
  std::fill(message1.begin(), message1.end(), 'A');
  std::fill(message2.begin(), message2.end(), 'B');

  mymuduo::InetAddress listenAddr(9987);
  mymuduo::EventLoop loop;
  mymuduo::TcpServer server(&loop, listenAddr, "server");
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  if (argc > 3) {
    server.setThreadNum(atoi(argv[3]));
  }
  server.start();
  loop.loop();
  return 0;
}

#include "TcpServer.hpp"
#include "EventLoop.hpp"
#include "InetAddress.hpp"

#include <iostream>
#include <unistd.h>

void onConnection(const mymuduo::TcpConnectionPtr &conn) {
  if (conn->connected()) {
    std::cout << "onConnection: tid=" << std::this_thread::get_id() << " new connection [" << conn->name()
              << "] from " << conn->peerAddress().toHostPort() << "\n";
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
  conn->send(buf->retrieveAsString());
}

int main(int argc, char *argv[]) {
  std::cout << "main(): pid = %d\n" << getpid();
  mymuduo::InetAddress listenAddr(9987);
  mymuduo::EventLoop loop;
  mymuduo::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();
  loop.loop();
  return 0;
}

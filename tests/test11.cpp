#include "TcpServer.hpp"
#include "EventLoop.hpp"
#include "InetAddress.hpp"
#include <iostream>
#include <unistd.h>

std::string message;

void onConnection(const mymuduo::TcpConnectionPtr &conn) {
  if (conn->connected()) {
    std::cout << "onConnection: new connection [" << conn->name()
              << "] from " << conn->peerAddress().toHostPort() << "\n";
    conn->send(message);
  } else {
    std::cout << "onConnection: connection [" << conn->name()
              << "] is done\n";
  }
}

void onWriteComplete(const mymuduo::TcpConnectionPtr &conn) {
  conn->send(message);
}

void onMessage(const mymuduo::TcpConnectionPtr &conn,
               mymuduo::Buffer *buf,
               mymuduo::time_point receiveTime) {
  std::cout << "onMessage(): received " << buf->readableBytes() << " bytes from connection ["
            << conn->name() << "] at " << mymuduo::to_string(receiveTime) << "\n";
//  conn->send(buf->retrieveAsString());
  buf->retrieveAll();
}

int main(int argc, char *argv[]) {
  std::cout << "main(): pid = %d\n" << getpid();

  std::string line;
  for (int i = 33; i < 127; ++i) {
    line.push_back(char(i));
  }
  line += line;

  for (size_t i = 0; i < 127 - 33; ++i) {
    message += line.substr(i, 72) + '\n';
  }

  mymuduo::InetAddress listenAddr(9987);
  mymuduo::EventLoop loop;
  mymuduo::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setWriteCompleteCallback(onWriteComplete);
  server.setMessageCallback(onMessage);
  server.start();
  loop.loop();
  return 0;
}

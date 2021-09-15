#include "EventLoop.hpp"
#include "InetAddress.hpp"
#include "TcpClient.hpp"

#include <iostream>

std::string message = "Hello\n";

void onConnection(const mymuduo::TcpConnectionPtr &conn) {
  if (conn->connected()) {
    std::cout << "onConnection: new connection [" << conn->name()
              << "] from " << conn->peerAddress().toHostPort() << "\n";
    conn->send(message);
  } else {
    std::cout << "onConnection: connection [" << conn->name()
              << "] is down\n";
  }
}

void onMessage(const mymuduo::TcpConnectionPtr &conn,
               mymuduo::Buffer *buf,
               mymuduo::time_point receiveTime) {
  std::cout << "onMessage(): received " << buf->readableBytes()
            << " bytes from connection ["
            << conn->name() << "] at " << mymuduo::to_string(receiveTime) << "\n";
  std::cout << "onMessage(): [" << buf->retrieveAsString() << "]\n";
}

int main(int argc, char *argv[]) {
  mymuduo::EventLoop loop;
  mymuduo::InetAddress serverAddr("localhost", 9987);
  mymuduo::TcpClient client(&loop, serverAddr);
  client.setConnectionCallback(onConnection);
  client.setMessageCallback(onMessage);
  client.enableRetry();
  client.connect();
  loop.loop();
  return 0;
}

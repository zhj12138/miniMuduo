#include "TcpServer.hpp"
#include "EventLoop.hpp"

void onMessage(const mymuduo::TcpConnectionPtr &conn, mymuduo::Buffer *buffer, mymuduo::time_point tp) {
  conn->send(buffer->retrieveAsString());
}

int main() {
  mymuduo::EventLoop loop;
  mymuduo::InetAddress listen_addr(9987);
  mymuduo::TcpServer server(&loop, listen_addr, "echo_server");
  server.setMessageCallback(onMessage);
  server.start();
  loop.loop();
  return 0;
}

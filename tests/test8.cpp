#include "TcpServer.hpp"
#include "EventLoop.hpp"
#include "InetAddress.hpp"

#include <cstdio>
#include <unistd.h>

void onConnection(const mymuduo::TcpConnectionPtr &conn) {
  if (conn->connected()) {
    printf("onConnection(): new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
  } else {
    printf("onConnection(): connection [%s] is down\n",
           conn->name().c_str());
  }
}

void onMessage(const mymuduo::TcpConnectionPtr &conn,
               const char *data,
               ssize_t len) {
  printf("onMessage(): received %zd bytes from connection [%s]\n",
         len, conn->name().c_str());
}

int main() {
  printf("main(): pid = %d\n", getpid());

  mymuduo::InetAddress listenAddr(9987);
  mymuduo::EventLoop loop;

  mymuduo::TcpServer server(&loop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.start();

  loop.loop();
  return 0;
}

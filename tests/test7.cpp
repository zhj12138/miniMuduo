#include "Acceptor.hpp"
#include "EventLoop.hpp"
#include "InetAddress.hpp"
#include "SocketsOps.hpp"

#include <iostream>
#include <unistd.h>

void newConnection(const std::string &msg, int sockfd, const mymuduo::InetAddress &peerAddr) {
  std::cout << "newConnection(): accepted a new connection from " << peerAddr.toHostPort() << "\n";
  ::write(sockfd, msg.c_str(), msg.size());
  mymuduo::sockets::close(sockfd);
}

mymuduo::Acceptor::NewConnectionCallback newConnectionWithMsg(const std::string &msg) {
  return [msg](auto &&PH1, auto &&PH2) {
    return newConnection(msg,
                         std::forward<decltype(PH1)>(PH1),
                         std::forward<decltype(PH2)>(PH2));
  };
}

int main() {
  std::cout << "main(): pid = " << getpid() << "\n";
  mymuduo::InetAddress listenAddr(9987);
  mymuduo::InetAddress listenAddr2(9988);
  mymuduo::EventLoop loop;

  mymuduo::Acceptor acceptor(&loop, listenAddr);
  acceptor.setNewConnectionCallback(newConnectionWithMsg("How are you?\n"));
  acceptor.listen();

  mymuduo::Acceptor acceptor2(&loop, listenAddr2);
  acceptor2.setNewConnectionCallback(newConnectionWithMsg("Good morning!\n"));
  acceptor2.listen();

  loop.loop();
}

#include "Acceptor.hpp"

#include "EventLoop.hpp"
#include "InetAddress.hpp"
#include "SocketsOps.hpp"

using namespace mymuduo;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false) {
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback([this] { handleRead(); });
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  listenning_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
  loop_->assertInLoopThread();
  InetAddress peerAddr(0);
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
    if (newConectionCallback_) {
      newConectionCallback_(connfd, peerAddr);
    } else {
      sockets::close(connfd);
    }
  }
}

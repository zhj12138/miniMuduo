#include "TcpServer.hpp"

#include "Acceptor.hpp"
#include "EventLoop.hpp"
#include "SocketsOps.hpp"

#include <glog/logging.h>
#include <cstdio>

using namespace mymuduo;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr)
    : loop_(loop),
      name_(listenAddr.toHostPort()),
      acceptor_(new Acceptor(loop, listenAddr)),
      started_(false),
      nextConnId_(1) {
  if (loop == nullptr) {
    LOG(FATAL) << "loop is nullptr\n";
  }
  acceptor_->setNewConnectionCallback([this](auto &&PH1, auto &&PH2) {
    newConnection(std::forward<decltype(PH1)>(PH1),
                  std::forward<decltype(PH2)>(PH2));
  });
}

TcpServer::~TcpServer() {
}

void TcpServer::start() {
  if (!started_) {
    started_ = true;
  }
  if (!acceptor_->listenning()) {
    loop_->runInLoop([capture0 = acceptor_.get()] { capture0->listen(); });
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
  loop_->assertInLoopThread();
  char buf[32];
  snprintf(buf, sizeof(buf), "#%d", nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;
  LOG(INFO) << "TcpServer::newConnection [" << name_
            << "] - new connection [" << connName
            << "] from " << peerAddr.toHostPort();
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->connectEstablished();
}


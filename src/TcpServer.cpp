#include "TcpServer.hpp"

#include "Acceptor.hpp"
#include "EventLoop.hpp"
#include "EventLoopThreadPool.hpp"
#include "SocketsOps.hpp"

#include <cassert>
#include <glog/logging.h>
#include <cstdio>

using namespace mymuduo;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,
                     std::string nameArg,
                     Option option)
    : loop_(loop),
      ipPort_(listenAddr.toHostPort()),
      name_(std::move(nameArg)),
      acceptor_(new Acceptor(loop, listenAddr)),
      threadPool_(new EventLoopThreadPool(loop)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      nextConnId_(1) {
  if (loop == nullptr) {
    LOG(FATAL) << "loop is nullptr\n";
  }
  acceptor_->setNewConnectionCallback([this](auto &&PH1, auto &&PH2) {
    newConnection(std::forward<decltype(PH1)>(PH1),
                  std::forward<decltype(PH2)>(PH2));
  });
}

TcpServer::~TcpServer() = default;

void TcpServer::setThreadNum(int numThreads) {
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
  if (!started_.exchange(true)) {
    threadPool_->start();
    assert(!acceptor_->listenning());
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
  EventLoop *ioLoop = threadPool_->getNextLoop();
  TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback([this](auto &&PH1) { removeConnection(std::forward<decltype(PH1)>(PH1)); });
  ioLoop->runInLoop([conn] { conn->connectEstablished(); });
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
  loop_->runInLoop([this, conn] { removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  LOG(INFO) << "TcpServer::removeConnection [" << name_
            << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  assert(n == 1);
  (void) n;
  EventLoop *ioLoop = conn->getLoop();
  ioLoop->queueInLoop([conn] { conn->connectDestroyed(); });
}


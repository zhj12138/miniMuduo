#include "TcpConnection.hpp"

#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"

#include <cassert>
#include <iostream>
#include <glog/logging.h>

using namespace mymuduo;

TcpConnection::TcpConnection(EventLoop *loop,
                             std::string name,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop),
      name_(std::move(name)),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr) {
  if (loop == nullptr) {
    LOG(FATAL) << "loop is nullptr";
  }
  LOG(INFO) << "TcpConnection::ctor[" << name_ << "] at " << this << " fd=" << sockfd;
  channel_->setReadCallback([this] { handleRead(); });
}

TcpConnection::~TcpConnection() {
  LOG(INFO) << "TcpConnection::dtor[" << name_ << "] at " << this << " fd=" << channel_->fd();
}

void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead() {
  char buf[65536];
  ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
  messageCallback_(shared_from_this(), buf, n);
}

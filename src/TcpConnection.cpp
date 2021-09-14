#include "TcpConnection.hpp"

#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"
#include "SocketsOps.hpp"

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
  channel_->setWriteCallback([this] { handleWrite(); });
  channel_->setCloseCallback([this] { handleClose(); });
  channel_->setErrorCallback([this] { handleError(); });
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

void TcpConnection::connectDestroyed() {
  loop_->assertInLoopThread();
  assert(state_ == kConnected);
  setState(kDisconnected);
  channel_->disableAll();
  connectionCallback_(shared_from_this());
  loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead() {
  char buf[65536];
  ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
  if (n > 0) {
    messageCallback_(shared_from_this(), buf, n);
  } else if (n == 0) {
    handleClose();
  } else {
    handleError();
  }
}

void TcpConnection::handleWrite() {
}

void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOG(INFO) << "TcpConnection::handleClose state = " << state_;
  channel_->disableAll();
  closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  LOG(ERROR) << "TcpConnection::handleError [" << name_
             << "] - SO_ERROR = " << err << " " << strerror(err);
}

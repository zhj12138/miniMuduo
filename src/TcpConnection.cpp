#include "TcpConnection.hpp"

#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"
#include "SocketsOps.hpp"
#include "Cast.hpp"

#include <cassert>
#include <iostream>
#include <glog/logging.h>

using namespace mymuduo;

void defaultConnectionCallback(const TcpConnectionPtr &conn) {
  LOG(INFO) << conn->localAddress().toHostPort() << " -> "
            << conn->peerAddress().toHostPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
}
void defaultMessageCallback(const TcpConnectionPtr &, Buffer *buf, time_point) {
  buf->retrieveAll();
}

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
  channel_->setReadCallback([this](auto &&PH1) { handleRead(std::forward<decltype(PH1)>(PH1)); });
  channel_->setWriteCallback([this] { handleWrite(); });
  channel_->setCloseCallback([this] { handleClose(); });
  channel_->setErrorCallback([this] { handleError(); });
}

TcpConnection::~TcpConnection() {
  LOG(INFO) << "TcpConnection::dtor[" << name_ << "] at " << this << " fd=" << channel_->fd();
}

void TcpConnection::send(const std::string &message) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(message);
    } else {
      loop_->runInLoop([this, message] { sendInLoop(message); });
    }
  }
}

void TcpConnection::shutdown() {
  if (state_ == kConnected) {
    setState(kDisconnecting);
    loop_->runInLoop([this] { shutdownInLoop(); });
  }
}

void TcpConnection::setTcpNoDelay(bool on) {
  socket_->setTcpNoDelay(on);
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
  assert(state_ == kConnected || state_ == kDisconnecting);
  setState(kDisconnected);
  channel_->disableAll();
  connectionCallback_(shared_from_this());
  loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead(time_point receiveTime) {
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    handleClose();
  } else {
    handleError();
  }
}

void TcpConnection::handleWrite() {
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
    ssize_t n = ::write(channel_->fd(),
                        outputBuffer_.peek(),
                        outputBuffer_.readableBytes());
    if (n > 0) {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0) {
        channel_->disableWriting();
        if (writeCompleteCallback_) {
          loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
        }
        if (state_ == kDisconnecting) {
          shutdownInLoop();
        }
      } else {
        LOG(INFO) << "I am going to write more data";
      }
    } else {
      LOG(ERROR) << "TcpConnection::handleWrite";
    }
  } else {
    LOG(INFO) << "Connection is down, no more writing";
  }
}

void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOG(INFO) << "TcpConnection::handleClose state = " << state_;
  assert(state_ == kConnected || state_ == kDisconnecting);
  channel_->disableAll();
  closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  LOG(ERROR) << "TcpConnection::handleError [" << name_
             << "] - SO_ERROR = " << err << " " << strerror(err);
}

void TcpConnection::sendInLoop(const std::string &message) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = ::write(channel_->fd(), message.data(), message.size());
    if (nwrote >= 0) {
      if (implicit_cast<size_t>(nwrote) < message.size()) {
        LOG(INFO) << "I am going to write more data";
      } else if (writeCompleteCallback_) {
        loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG(ERROR) << "TcpConnection::sendInLoop";
      }
    }
  }
  assert(nwrote >= 0);
  if (implicit_cast<size_t>(nwrote) < message.size()) { // 还没有发送完，放到outputBuffer中
    outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }
}

void TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();
  if (!channel_->isWriting()) {
    socket_->shutdownWrite();
  }
}

void TcpConnection::send(Buffer *buf) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    } else {
      // TODO
    }
  }
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == kDisconnected) {
    LOG(WARNING) << "disconnected, give up writing";
    return;
  }
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = sockets::write(channel_->fd(), data, len);
    if (nwrote >= 0) {
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_) {
        loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG(ERROR) << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET) {
          faultError = true;
        }
      }
    }
  }
  assert(remaining <= len);
  if (!faultError && remaining > 0) {
    size_t oldLen = outputBuffer_.readableBytes();
    if (oldLen + remaining >= highWaterMark_ &&
        oldLen < highWaterMark_ &&
        highWaterMarkCallback_) {
      loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
    }
    outputBuffer_.append(static_cast<const char *>(data) + nwrote, remaining);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }
}

#include "Connector.hpp"

#include "Channel.hpp"
#include "EventLoop.hpp"
#include "SocketsOps.hpp"

#include <glog/logging.h>
#include <cerrno>
#include <cassert>
#include <memory>

using namespace mymuduo;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs) {
  LOG(INFO) << "ctor[" << this << "]";
}

Connector::~Connector() {
  LOG(INFO) << "dtor[" << this << "]";
  loop_->cancel(timerId_);
  assert(!channel_);
}

void Connector::start() {
  connect_ = true;
  loop_->runInLoop([this] { startInLoop(); });
}

void Connector::startInLoop() {
  loop_->assertInLoopThread();
  assert(state_ == kDisconnected);
  if (connect_) {
    connect();
  } else {
    LOG(INFO) << "do not connect";
  }
}

void Connector::connect() {
  int sockfd = sockets::createNonblockingOrDie();
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddrInet());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN: //
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH://
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK://
      LOG(ERROR) << "connect error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      break;

    default://
      LOG(ERROR) << "Unexpected error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}

void Connector::restart() {
  loop_->assertInLoopThread();
  setState(kDisconnected);
  retryDelayMs_ = kInitRetryDelayMs;
  connect_ = true;
  startInLoop();
}

void Connector::stop() {
  connect_ = false;
  loop_->cancel(timerId_);
}

void Connector::connecting(int sockfd) {
  setState(kConnecting);
  assert(!channel_);
  channel_ = std::make_unique<Channel>(loop_, sockfd);
  channel_->setWriteCallback([this] { handleWrite(); });
  channel_->setErrorCallback([this] { handleError(); });
  channel_->enableWriting();
}

int Connector::removeAndResetChannel() {
  channel_->disableAll();
  loop_->removeChannel(channel_.get());
  int sockfd = channel_->fd();
  loop_->queueInLoop([this] { resetChannel(); });
  return sockfd;
}

void Connector::resetChannel() {
  channel_.reset();
}

void Connector::handleWrite() {
  LOG(INFO) << "Connector::handleWrite " << state_;
  if (state_ == kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    if (err) {
      LOG(WARNING) << "Connector::handleWrite - SO_ERROR = "
                   << err << " " << strerror(err);
      retry(sockfd);
    } else if (sockets::isSelfConnect(sockfd)) {
      LOG(WARNING) << "Connector::handleWrite = Self Connect";
      retry(sockfd);
    } else {
      setState(kConnected);
      if (connect_) {
        newConnectionCallback_(sockfd);
      } else {
        sockets::close(sockfd);
      }
    }
  } else {
    assert(state_ == kDisconnected);
  }
}

void Connector::handleError() {
  LOG(ERROR) << "Connector::handleError";
  assert(state_ == kConnecting);

  int sockfd = removeAndResetChannel();
  int err = sockets::getSocketError(sockfd);
  LOG(INFO) << "SO_ERROR = " << err << " " << strerror(err);
  retry(sockfd);
}

void Connector::retry(int sockfd) {
  sockets::close(sockfd);
  setState(kDisconnected);
  if (connect_) {
    LOG(INFO) << "Connector::retry - Retry connecting to "
              << serverAddr_.toHostPort() << " in "
              << retryDelayMs_ << " milliseconds. ";
    timerId_ = loop_->runAfter(retryDelayMs_ / 1000.0,
                               [this] { startInLoop(); });
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  } else {
    LOG(INFO) << "do not connect";
  }
}

#include "TcpClient.hpp"

#include "Connector.hpp"
#include "EventLoop.hpp"
#include "SocketsOps.hpp"

#include <glog/logging.h>
#include <cstdio>

using namespace mymuduo;

namespace mymuduo {

namespace detail {

void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn) {
  loop->queueInLoop([conn] { conn->connectDestroyed(); });
}

void removeConnector(const ConnectorPtr &connector) {
  // ?
}

}
}

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr)
    : loop_(loop),
      connector_(new Connector(loop, serverAddr)),
      retry_(false),
      connect_(true),
      nextConnId_(1) {
  if (loop == nullptr) {
    LOG(FATAL) << "loop is nullptr";
  }
  connector_->setNewConnectionCallback([this](auto &&PH1) { newConnection(std::forward<decltype(PH1)>(PH1)); });
  LOG(INFO) << "TcpClient::TcpClient[" << this << "] - connector " << connector_.get();
}

TcpClient::~TcpClient() {
  LOG(INFO) << "TcpClient::~TcpClient[" << this
            << "] - connector " << connector_.get();
  TcpConnectionPtr conn;
  {
    std::lock_guard<std::mutex> lg(mutex_);
    conn = connection_;
  }
  if (conn) {
    CloseCallback cb = [this](auto &&PH1) {
      detail::removeConnection(loop_, std::forward<decltype(PH1)>(PH1));
    };
    loop_->runInLoop([conn, cb] { conn->setCloseCallback(cb); });
  } else {
    connector_->stop();
    loop_->runAfter(1, [this] { detail::removeConnector(connector_); });
  }
}

void TcpClient::connect() {
  LOG(INFO) << "TcpClient::connect[" << this << "] - connecting to "
            << connector_->serverAddress().toHostPort();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  connect_ = false;
  {
    std::lock_guard<std::mutex> lg(mutex_);
    if (connection_) {
      connection_->shutdown();
    }
  }
}

void TcpClient::stop() {
  connect_ = false;
  connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
  loop_->assertInLoopThread();
  InetAddress peerAddr(sockets::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
  ++nextConnId_;
  std::string connName = buf;

  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback([this](const TcpConnectionPtr &conn) { removeConnection(conn); });
  {
    std::lock_guard<std::mutex> lg(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());
  {
    std::lock_guard<std::mutex> lg(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }
  loop_->queueInLoop([conn] { conn->connectDestroyed(); });
  if (retry_ && connect_) {
    LOG(INFO) << "TcpClient::connect[" << this << "] - Reconnecting to "
              << connector_->serverAddress().toHostPort();
    connector_->restart();
  }
}

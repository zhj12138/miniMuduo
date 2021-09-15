#ifndef MYMUDUO__TCPCLIENT_HPP_
#define MYMUDUO__TCPCLIENT_HPP_

#include "noncopyable.hpp"
#include "TcpConnection.hpp"

#include <mutex>

namespace mymuduo {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : noncopyable {
 public:
  TcpClient(EventLoop *loop, const InetAddress &serverAddr);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    std::lock_guard<std::mutex> lg(mutex_);
    return connection_;
  }

  bool retry() const;
  void enableRetry() { retry_ = true; }

  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }
  void setMessageCallback(const MessageCallback &cb) {
    messageCallback_ = cb;
  }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }
 private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr &conn);

  EventLoop *loop_;
  ConnectorPtr connector_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_;
  bool connect_;
  int nextConnId_;
  mutable std::mutex mutex_;
  TcpConnectionPtr connection_;
};

}
#endif //MYMUDUO__TCPCLIENT_HPP_

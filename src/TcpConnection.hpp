#ifndef MYMUDUO__TCPCONNECTION_HPP_
#define MYMUDUO__TCPCONNECTION_HPP_

#include "Callbacks.hpp"
#include "InetAddress.hpp"
#include "noncopyable.hpp"
#include "Buffer.hpp"

#include <any>

namespace mymuduo {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop *loop,
                std::string name,
                int sockfd,
                const InetAddress &localAddr,
                const InetAddress &peerAddr);
  ~TcpConnection();

  EventLoop *getLoop() const { return loop_; }
  const std::string &name() const { return name_; }
  const InetAddress &localAddress() { return localAddr_; }
  const InetAddress &peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }
  bool disconnected() const { return state_ == kDisconnected; }

  void send(const std::string &message);
  void send(Buffer *buf);
  void shutdown();
  void setTcpNoDelay(bool on);

  void setContext(const std::any &context) {
    context_ = context;
  }
  std::any *getMutableContext() {
    return &context_;
  }

  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }
  void setMessageCallback(const MessageCallback &cb) {
    messageCallback_ = cb;
  }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
  }
  void setCloseCallback(const CloseCallback &cb) {
    closeCallback_ = cb;
  }
  void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) {
    highWaterMarkCallback_ = cb;
  }
  void connectEstablished();  // should be called only once
  void connectDestroyed();
 private:
  enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected, };

  void setState(StateE s) { state_ = s; }
  void handleRead(time_point receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const std::string &message);
  void sendInLoop(const void *message, size_t len);
  void shutdownInLoop();

  EventLoop *loop_;
  std::string name_;
  StateE state_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  CloseCallback closeCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
  size_t highWaterMark_;
  Buffer inputBuffer_;
  Buffer outputBuffer_;
  std::any context_;
};

void defaultConnectionCallback(const TcpConnectionPtr &conn);
void defaultMessageCallback(const TcpConnectionPtr &, Buffer *buf, time_point);

}
#endif //MYMUDUO__TCPCONNECTION_HPP_

#ifndef MYMUDUO__TCPCONNECTION_HPP_
#define MYMUDUO__TCPCONNECTION_HPP_

#include "Callbacks.hpp"
#include "InetAddress.hpp"
#include "noncopyable.hpp"

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
  bool connected() const  { return state_ == kConnected; }

  void setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
  }
  void setMessageCallback(const MessageCallback &cb) {
    messageCallback_ = cb;
  }
  void setCloseCallback(const CloseCallback &cb) {
    closeCallback_ = cb;
  }
  void connectEstablished();  // should be called only once
  void connectDestroyed();
 private:
  enum StateE { kConnecting, kConnected, kDisconnected, };

  void setState(StateE s) { state_ = s; }
  void handleRead();
  void handleWrite();
  void handleClose();
  void handleError();

  EventLoop *loop_;
  std::string name_;
  StateE state_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  CloseCallback closeCallback_;
};

}
#endif //MYMUDUO__TCPCONNECTION_HPP_
#ifndef MYMUDUO__ACCEPTOR_HPP_
#define MYMUDUO__ACCEPTOR_HPP_

#include "noncopyable.hpp"
#include "Channel.hpp"
#include "Socket.hpp"

namespace mymuduo {

class EventLoop;
class InetAddress;

class Acceptor : noncopyable {
 public:
  using NewConnectionCallback = std::function<void (int sockfd, const InetAddress &)>;
  Acceptor(EventLoop *loop, const InetAddress &listenAddr);

  void setNewConnectionCallback(const NewConnectionCallback &cb) { newConectionCallback_ = cb; }
  bool listenning() const { return listenning_; }
  void listen();

 private:
  void handleRead();
  EventLoop *loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConectionCallback_;
  bool listenning_;
};

}
#endif //MYMUDUO__ACCEPTOR_HPP_

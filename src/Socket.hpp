#ifndef MYMUDUO__SOCKET_HPP_
#define MYMUDUO__SOCKET_HPP_

#include "noncopyable.hpp"

namespace mymuduo {

class InetAddress;

class Socket : noncopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}
  ~Socket();

  int fd() const { return sockfd_; }

  void bindAddress(const InetAddress &localaddr);
  void listen();
  int accept(InetAddress *peeraddr);
  void setReuseAddr(bool on);
 private:
  const int sockfd_;
};

}

#endif //MYMUDUO__SOCKET_HPP_

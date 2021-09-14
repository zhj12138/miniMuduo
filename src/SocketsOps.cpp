#include "SocketsOps.hpp"

#include "Cast.hpp"

#include <glog/logging.h>
#include <cerrno>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>

using namespace mymuduo;

namespace {

using SA = struct sockaddr;

const SA *sockaddr_cast(const struct sockaddr_in *addr) {
  return static_cast<const SA *>(implicit_cast<const void *>(addr));
}

SA *sockaddr_cast(struct sockaddr_in *addr) {
  return static_cast<SA *>(implicit_cast<void *>(addr));
}

void setNonBlockAndCloseOnExec(int sockfd) {
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (::fcntl(sockfd, F_SETFL, flags) < 0) {
    LOG(FATAL) << "set sockfd: " << sockfd << " NONBLOCK failed";
  }

  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  if (::fcntl(sockfd, F_SETFD, flags) < 0) {
    LOG(FATAL) << "set sockfd: " << sockfd << " CLOEXEC failed";
  }
}

}

int sockets::createNonblockingOrDie() {
#if VALGRIND
  int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    LOG(FATAL) << "sockets::createNonblockingOrDie";
  }
  setNonBlockAndCloseOnExec(sockfd);
#else
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                        IPPROTO_TCP);
  if (sockfd < 0) {
    LOG(FATAL) << "sockets::createNonblockingOrDie";
  }
#endif
  return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in &addr) {
  int ret = ::bind(sockfd, sockaddr_cast(&addr), sizeof(addr));
  if (ret < 0) {
    LOG(FATAL) << "sockets::bindOrDie";
  }
}

void sockets::listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0) {
    LOG(FATAL) << "sockets::listenOrDie";
  }
}

int sockets::accept(int sockfd, struct sockaddr_in *addr) {
  socklen_t addrlen = sizeof(*addr);
#if VALGRIND
  int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
#else
  int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
  if (connfd < 0) {
    int savedErrno = errno;
    LOG(ERROR) << "Socket::accept";
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO: // ???
      case EPERM:
      case EMFILE: // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG(FATAL) << "unexpected error of ::accept " << savedErrno;
        break;
      default:LOG(FATAL) << "unknown error of ::accept " << savedErrno;
        break;
    }
  }
  return connfd;
}

void sockets::close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG(ERROR) << "sockets::close";
  }
}

void sockets::toHostPort(char *buf, size_t size, const struct sockaddr_in &addr) {
  char host[INET_ADDRSTRLEN] = "INVALID";
  ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host));
  uint16_t port = sockets::networkToHost16(addr.sin_port);
  snprintf(buf, size, "%s:%u", host, port);
}

void sockets::fromHostPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    LOG(ERROR) << "sockets::fromHostPort";
  }
}

struct sockaddr_in sockets::getLocalAddr(int sockfd) {
  struct sockaddr_in localaddr{};
  bzero(&localaddr, sizeof(localaddr));
  socklen_t addrlen = sizeof(localaddr);
  if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
    LOG(ERROR) << "sockets::getLocalAddr";
  }
  return localaddr;
}

int sockets::getSocketError(int sockfd) {
  int optval;
  socklen_t optlen = sizeof(optval);
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

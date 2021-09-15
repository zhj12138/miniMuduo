#include "Buffer.hpp"

#include "Cast.hpp"

#include <sys/uio.h>

using namespace mymuduo;

void Buffer::makeSpace(size_t len) {
  if (writeableBytes() + prependableBytes() < len + kCheapPrepend) {
    buffer_.resize(writerIndex_ + len);
  } else {  // move readable data to the front
    assert(kCheapPrepend < readerIndex_);
    size_t readable = readableBytes();
    std::copy(begin() + readerIndex_,
              begin() + writerIndex_,
              begin() + kCheapPrepend);
    readerIndex_ = kCheapPrepend;
    writerIndex_ = readerIndex_ + readable;
    assert(readable == readableBytes());
  }
}

ssize_t Buffer::readFd(int fd, int *savedErrno) {
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writeble = writeableBytes();
  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writeble;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);
  const ssize_t n = readv(fd, vec, 2);
  if (n < 0) {
    *savedErrno = errno;
  } else if (implicit_cast<size_t>(n) <= writeble) {
    writerIndex_ += n;
  } else {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writeble);
  }
  return n;
}

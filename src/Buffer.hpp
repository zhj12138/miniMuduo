#ifndef MYMUDUO__BUFFER_HPP_
#define MYMUDUO__BUFFER_HPP_

#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <cstring>

namespace mymuduo {

class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;
  Buffer()
      : buffer_(kCheapPrepend + kInitialSize),
        readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend) {
  }

  void swap(Buffer &rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }

  size_t readableBytes() const { return writerIndex_ - readerIndex_; }
  size_t writeableBytes() const { return buffer_.size() - writerIndex_; }
  size_t prependableBytes() const { return readerIndex_; }

  void hasWritten(size_t len) { writerIndex_ += len; }

  const char *peek() const { return begin() + readerIndex_; }
  char *beginWrite() { return begin() + writerIndex_; }
  const char *beginWrite() const { return begin() + writerIndex_; }

  void ensureWriteableBytes(size_t len) {
    if (writeableBytes() < len) {
      makeSpace(len);
    }
    assert(writeableBytes() >= len);
  }

  const char *findCRLF() const {
    const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
  }
  const char *findCRLF(const char *start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
  }
  const char *findEOL() const {
    const void *eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char *>(eol);
  }
  const char *findEOL(const char *start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void *eol = memchr(start, '\n', beginWrite() - start);
    return static_cast<const char *>(eol);
  }

  void retrieve(size_t len) {
    assert(len <= readableBytes());
    readerIndex_ += len;
  }

  void retrieveUntil(const char *end) {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
  }

  void retrieveAll() {
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
  }

  std::string retrieveAsString() {
    std::string str(peek(), readableBytes());
    retrieveAll();
    return str;
  }

  void append(const char *data, size_t len) {
    ensureWriteableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
  }
  void append(const void *data, size_t len) {
    append(static_cast<const char *>(data), len);
  }
  void append(const std::string &str) {
    append(str.data(), str.length());
  }

  void prepend(const void *data, size_t len) {
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    auto d = static_cast<const char *>(data);
    std::copy(d, d + len, begin() + readerIndex_);
  }

  void shrink(size_t reserve) {
    std::vector<char> buf(kCheapPrepend + readableBytes() + reserve);
    std::copy(peek(), peek() + readableBytes(), buf.begin() + kCheapPrepend);
    buf.swap(buffer_);
  }

  ssize_t readFd(int fd, int *savedErrno);

 private:
  char *begin() {
    return &*buffer_.begin();
  }
  const char *begin() const {
    return &*buffer_.begin();
  }
  void makeSpace(size_t len);
  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;

  static const char kCRLF[];
};

}
#endif //MYMUDUO__BUFFER_HPP_

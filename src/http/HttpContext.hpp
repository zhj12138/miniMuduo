#ifndef MYMUDUO_SRC_HTTP_HTTPCONTEXT_HPP_
#define MYMUDUO_SRC_HTTP_HTTPCONTEXT_HPP_

#include "HttpRequest.hpp"

namespace mymuduo {

class Buffer;

class HttpContext {
 public:
  enum class HttpRequestParseState {
    ExpectRequestLine,
    ExpectHeaders,
    ExpectBody,
    GotAll,
  };
  HttpContext() : state_(HttpRequestParseState::ExpectRequestLine) {}

  bool parseRequest(Buffer *buf, time_point receiveTime);
  bool gotAll() const { return state_ == HttpRequestParseState::GotAll; }
  void reset() {
    state_ = HttpRequestParseState::ExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }
  const HttpRequest &request() const { return request_; }
  HttpRequest &request() { return request_; }
 private:
  bool processRequestLine(const char *begin, const char *end);
  HttpRequestParseState state_;
  HttpRequest request_;
};

}

#endif //MYMUDUO_SRC_HTTP_HTTPCONTEXT_HPP_

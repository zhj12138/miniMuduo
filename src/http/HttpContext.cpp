#include "HttpContext.hpp"

#include "Buffer.hpp"

#include <algorithm>

using namespace mymuduo;

// Http请求示例：
// GET /somedir/page.html HTTP/1.1
// Host: www.someschool.edu
// Connection: close
// User-agent: Mozilla/5.0
// Accept-language: zh

bool HttpContext::processRequestLine(const char *begin, const char *end) {
  bool succeed = false;
  const char *start = begin;
  const char *space = std::find(start, end, ' ');
  if (space != end && request_.setMethod(start, space)) {
    start = space + 1;
    space = std::find(start, end, ' ');
    if (space != end) {
      const char *question = std::find(start, space, '?');
      // 看URL中有没有参数
      if (question != space) {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      } else {
        request_.setPath(start, space);
      }
      start = space + 1;
      succeed = (end - start == 8) && std::equal(start, end - 1, "HTTP/1.");
      if (succeed) {
        if (*(end - 1) == '1') {
          request_.setVersion(HttpRequest::Version::Http11);
        } else if (*(end - 1) == '0') {
          request_.setVersion(HttpRequest::Version::Http10);
        } else {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

bool HttpContext::parseRequest(Buffer *buf, time_point receiveTime) {
  bool ok = true;
  bool hasMore = true;
  while (hasMore) {
    if (state_ == HttpRequestParseState::ExpectRequestLine) {
      const char *crlf = buf->findCRLF();
      if (crlf) {
        ok = processRequestLine(buf->peek(), crlf);
        if (ok) {
          request_.setReceiveTime(receiveTime);
          buf->retrieveUntil(crlf + 2);
          state_ = HttpRequestParseState::ExpectHeaders;
        } else {
          hasMore = false;
        }
      } else {
        hasMore = false;
      }
    } else if (state_ == HttpRequestParseState::ExpectHeaders) {
      const char *crlf = buf->findCRLF();
      if (crlf) {
        const char *colon = std::find(buf->peek(), crlf, ';');
        if (colon != crlf) {
          request_.addHeader(buf->peek(), colon, crlf);
        } else {
          state_ = HttpRequestParseState::GotAll;
          hasMore = false;
        }
        buf->retrieveUntil(crlf + 2);
      } else {
        hasMore = false;
      }
    } else if (state_ == HttpRequestParseState::ExpectBody) {
      // TODO
    }
  }
  return ok;
}

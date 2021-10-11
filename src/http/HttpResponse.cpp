#include "HttpResponse.hpp"

#include "Buffer.hpp"

using namespace mymuduo;

// 响应报文示例：
// HTTP/1.1 200 OK
// Connection: close
// Date: Tue, 18 Aug 2015 15:44:04 GMT
// Server: Apache/2.2.3(CentOS)
// Last-Modified: Tue, 18, Agu 2015 15:11:03 GMT
// Content-Length: 6821
// Content-Type: text/html
// (data data data data ...)

void HttpResponse::appendToBuffer(Buffer *output) const {
  char buf[32];
  snprintf(buf, sizeof(buf), "HTTP/1.1 %d", statusCode_);
  output->append(buf);
  output->append(statusMessage_);
  output->append("\r\n");
  if (closeConnection_) {
    output->append("Connection: close\r\n");
  } else {
    snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
    output->append(buf);
    output->append("Connection: Keep-Alive\r\n");
  }
  for (const auto &header : headers_) {
    output->append(header.first);
    output->append(": ");
    output->append(header.second);
    output->append("\r\n");
  }
  output->append("\r\n");
  output->append(body_);
}

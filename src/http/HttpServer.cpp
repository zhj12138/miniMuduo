#include "HttpServer.hpp"

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpContext.hpp"

#include <glog/logging.h>

using namespace mymuduo;

namespace mymuduo {
namespace detail {

void defaultHttpCallback(const HttpRequest &, HttpResponse *resp) {
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

}
}

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const std::string &name,
                       TcpServer::Option option)
    : server_(loop, listenAddr, name, option),
      httpCallback_(detail::defaultHttpCallback) {
  server_.setConnectionCallback(
      [this](auto &&PH1) { onConnection(std::forward<decltype(PH1)>(PH1)); });
  server_.setMessageCallback(
      [this](auto &&PH1, auto &&PH2, auto &&PH3) {
        onMessage(std::forward<decltype(PH1)>(PH1),
                  std::forward<decltype(PH2)>(PH2),
                  std::forward<decltype(PH3)>(PH3));
      });
}

void HttpServer::start() {
  LOG(WARNING) << "HttpServer[" << server_.name()
               << "] starts listenning on " << server_.ipPort();
  server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
  if (conn->connected()) {
    conn->setContext(HttpContext());
  }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, time_point receiveTime) {
  auto *context = std::any_cast<HttpContext>(conn->getMutableContext());
  if (!context->parseRequest(buf, receiveTime)) {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }
  if (context->gotAll()) {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &req) {
  const std::string &connection = req.getHeader("Connection");
  bool close = (connection == "close") ||
      (req.getVersion() == HttpRequest::Version::Http10 && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(&buf);
  if (response.closeConnection()) {
    conn->shutdown();
  }
}






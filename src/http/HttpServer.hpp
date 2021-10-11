#ifndef MYMUDUO_SRC_HTTP_HTTPSERVER_HPP_
#define MYMUDUO_SRC_HTTP_HTTPSERVER_HPP_

#include "noncopyable.hpp"
#include "TcpServer.hpp"

namespace mymuduo {

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable {
 public:
  using HttpCallback = std::function<void(const HttpRequest &, HttpResponse *)>;
  HttpServer(EventLoop *loop,
             const InetAddress &listenAddr,
             const std::string &name,
             TcpServer::Option option = TcpServer::kNoReusePort);
  EventLoop *getLoop() const { return server_.getLoop(); }

  void setHttpCallback(const HttpCallback &cb) { httpCallback_ = cb; }
  void setThreadNum(int numThreads) {
    server_.setThreadNum(numThreads);
  }
  void start();
 private:
  void onConnection(const TcpConnectionPtr &conn);
  void onMessage(const TcpConnectionPtr &conn, Buffer *buf, time_point receiveTime);
  void onRequest(const TcpConnectionPtr &, const HttpRequest &);
  TcpServer server_;
  HttpCallback httpCallback_;
};

}

#endif //MYMUDUO_SRC_HTTP_HTTPSERVER_HPP_

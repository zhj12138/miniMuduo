#include "EventLoop.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpServer.hpp"

#include <iostream>

void httpCallback(const mymuduo::HttpRequest &request, mymuduo::HttpResponse *response) {
  std::cout << request.path() << std::endl;
  response->setStatusCode(mymuduo::HttpResponse::k200Ok);
  response->setBody("hello, world!");
}

int main(int argc, char *argv[]) {
  mymuduo::EventLoop loop;
  mymuduo::InetAddress address("localhost", 9987);
  mymuduo::HttpServer server(&loop, address, "http_server");
  server.setHttpCallback(httpCallback);
  server.start();
  loop.loop();
  return 0;
}

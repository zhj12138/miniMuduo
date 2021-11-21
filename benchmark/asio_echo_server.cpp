#include <boost/asio.hpp>
#include <memory>

namespace asio = boost::asio;
using asio::ip::tcp;
using boost::system::error_code;

class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(tcp::socket socket) : socket_(std::move(socket)) {}
  void start() {
    do_read();
  }
 private:
  void do_read() {
    socket_.async_read_some(asio::buffer(data_, kMaxLength),
                            [this, self = shared_from_this()](error_code ec, std::size_t length) {
                              if (!ec) {
                                do_write(length);
                              }
                            });
  }
  void do_write(std::size_t length) {
    asio::async_write(socket_,
                      asio::buffer(data_, length),
                      [this, self = shared_from_this()](error_code ec, std::size_t _length) {
                        if (!ec) {
                          do_read();
                        }
                      });
  }
  static constexpr int kMaxLength = 1024;
  tcp::socket socket_;
  char data_[kMaxLength]{};
};

class TcpServer {
 public:
  TcpServer(asio::io_context &io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    do_accept();
  }
 private:
  void do_accept() {
    acceptor_.async_accept([this](error_code ec, tcp::socket socket) {
      if (!ec) {
        std::make_shared<Session>(std::move(socket))->start();
      }
      do_accept();
    });
  }
  tcp::acceptor acceptor_;
};

int main() {
  asio::io_context context;
  TcpServer server(context, 9988);
  context.run();
  return 0;
}


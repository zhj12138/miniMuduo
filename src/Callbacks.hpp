#ifndef MYMUDUO__CALLBACKS_HPP_
#define MYMUDUO__CALLBACKS_HPP_

#include <functional>
#include <memory>

namespace mymuduo {

class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &,
                                           const char *data, ssize_t len)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
}

#endif //MYMUDUO__CALLBACKS_HPP_

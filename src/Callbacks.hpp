#ifndef MYMUDUO__CALLBACKS_HPP_
#define MYMUDUO__CALLBACKS_HPP_

#include <functional>
#include <memory>

#include "TimeUtil.hpp"

namespace mymuduo {

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &,
                                           Buffer *buf,
                                           time_point)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
}

#endif //MYMUDUO__CALLBACKS_HPP_

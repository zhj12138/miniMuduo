#ifndef MYMUDUO__POLLER_HPP_
#define MYMUDUO__POLLER_HPP_

#include <map>
#include <vector>

#include "EventLoop.hpp"
#include "TimeUtil.hpp"

struct pollfd;

namespace mymuduo {

class Channel;

class Poller : noncopyable {
 public:
  using ChannelVec = std::vector<Channel *>;

  explicit Poller(EventLoop *loop) : ownerLoop_(loop) {}
  ~Poller() = default;

  time_point poll(int timeoutMs, ChannelVec *activeChannels);

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);

  void assertInLoopThread() {}
 private:
  void fillActiveChannels(int numEvents,
                          ChannelVec *activeChannels) const;
  using PollFdVec = std::vector<struct pollfd>;
  using ChannelMap = std::map<int, Channel *>;

  EventLoop *ownerLoop_;
  PollFdVec pollfds_;
  ChannelMap channels_;
};

}
#endif //MYMUDUO__POLLER_HPP_

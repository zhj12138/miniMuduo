#ifndef MYMUDUO__EPOLLER_HPP_
#define MYMUDUO__EPOLLER_HPP_

#include <map>
#include <vector>

#include "EventLoop.hpp"
#include "TimeUtil.hpp"

struct epoll_event;

namespace mymuduo {

class Channel;

class EPoller : noncopyable {
 public:
  using ChannelVec = std::vector<Channel *>;

  explicit EPoller(EventLoop *loop);
  ~EPoller();

  time_point poll(int timeoutMs, ChannelVec *activeChannels);

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);

  void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

 private:
  static const int kInitEventListSize = 16;

  void fillActiveChannels(int numEvents,
                          ChannelVec *activeChannels) const;
  void update(int operation, Channel *channel);

  using EventVec = std::vector<struct epoll_event>;
  using ChannelMap = std::map<int, Channel *>;

  EventLoop *ownerLoop_;
  int epollfd_;
  EventVec events_;
  ChannelMap channels_;
};

}
#endif //MYMUDUO__EPOLLER_HPP_

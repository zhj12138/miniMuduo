#include "Poller.hpp"

#include "Channel.hpp"
#include "Cast.hpp"

#include <poll.h>
#include <glog/logging.h>
#include <cassert>

using namespace mymuduo;

time_point Poller::poll(int timeoutMs, Poller::ChannelVec *activeChannels) {
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
  time_point now(get_now());
  if (numEvents > 0) {
    LOG(INFO) << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {
    LOG(INFO) << " nothing happended";
  } else {
    LOG(ERROR) << "Poller::poll()";
  }
  return now;
}

void Poller::fillActiveChannels(int numEvents, Poller::ChannelVec *activeChannels) const {
  for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
    if (pfd->revents > 0) {
      --numEvents;
      auto ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      auto channel = ch->second;
      channel->set_revents(pfd->revents);
      // pfd->revents = 0;
      activeChannels->push_back(channel);
    }
  }
}

void Poller::updateChannel(Channel *channel) {
  assertInLoopThread();
  LOG(INFO) << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0) { // 不存在，添加新channel
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd{channel->fd(), static_cast<short>(channel->events()), 0};
    pollfds_.push_back(pfd);
    int idx = static_cast<int>(pollfds_.size()) - 1;
    channel->set_index(idx);
    channels_[pfd.fd] = channel;
  } else {  // 更新现有的channel
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd &pfd = pollfds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);

    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->isNoneEvent()) { // ignore this pollfd
      pfd.fd = -channel->fd() - 1;
    }
  }
}

void Poller::removeChannel(Channel *channel) {
  assertInLoopThread();
  LOG(INFO) << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent());
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
  const struct pollfd &pfd = pollfds_[idx];
  assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(n == 1);
  (void) n;
  if (implicit_cast<size_t>(idx) == pollfds_.size() - 1) {
    pollfds_.pop_back();
  } else {
    int channelAtEnd = pollfds_.back().fd;
    std::iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
    if (channelAtEnd < 0) {
      channelAtEnd = -channelAtEnd - 1;
    }
    channels_[channelAtEnd]->set_index(idx);
    pollfds_.pop_back();
  }
}

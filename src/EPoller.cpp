#include "EPoller.hpp"

#include "Channel.hpp"
#include "Cast.hpp"

#include <poll.h>
#include <sys/epoll.h>
#include <glog/logging.h>
#include <cassert>

using namespace mymuduo;

static_assert(EPOLLIN == POLLIN, "");
static_assert(EPOLLPRI == POLLPRI, "");
static_assert(EPOLLOUT == POLLOUT, "");
static_assert(EPOLLRDHUP == POLLRDHUP, "");
static_assert(EPOLLERR == POLLERR, "");
static_assert(EPOLLHUP == POLLHUP, "");

namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

EPoller::EPoller(EventLoop *loop)
    : ownerLoop_(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG(FATAL) << "EPoller::EPoller";
  }
}

EPoller::~EPoller() {
  ::close(epollfd_);
}

time_point EPoller::poll(int timeoutMs, EPoller::ChannelVec *activeChannels) {
  int numEvents = ::epoll_wait(epollfd_,
                               &*events_.begin(),
                               static_cast<int>(events_.size()),
                               timeoutMs);
  time_point now(get_now());
  if (numEvents > 0) {
    LOG(INFO) << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
    if (implicit_cast<size_t>(numEvents) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    LOG(INFO) << " nothing happended";
  } else {
    LOG(ERROR) << "EPoller::poll()";
  }
  return now;
}

void EPoller::fillActiveChannels(int numEvents, EPoller::ChannelVec *activeChannels) const {
  assert(implicit_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; ++i) {
    auto channel = static_cast<Channel *>(events_[i].data.ptr);
#ifndef NDEBUG
    int fd = channel->fd();
    auto it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}

void EPoller::updateChannel(Channel *channel) {
  assertInLoopThread();
  LOG(INFO) << "fd = " << channel->fd() << " events = " << channel->events();
  const int index = channel->index();
  if (index == kNew || index == kDeleted) { // 不存在，添加新channel
    int fd = channel->fd();
    if (index == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else {
      assert(channels_.find(channel->fd()) != channels_.end());
      assert(channels_[fd] == channel);
    }
    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {  // 更新现有的channel
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent()) { // ignore this pollfd
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPoller::removeChannel(Channel *channel) {
  assertInLoopThread();
  int fd = channel->fd();
  LOG(INFO) << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);
  size_t n = channels_.erase(fd);
  assert(n == 1);
  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

void EPoller::update(int operation, Channel *channel) {
  struct epoll_event event{};
  bzero(&event, sizeof(event));
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG(ERROR) << "epoll_ctl op=" << operation << " fd=" << fd;
    } else {
      LOG(FATAL) << "epoll_ctl op=" << operation << " fd=" << fd;
    }
  }
}

#include "Channel.hpp"

#include "EventLoop.hpp"

#include <poll.h>
#include <glog/logging.h>
#include <cassert>

using namespace mymuduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      eventHandling_(false) {
}

Channel::~Channel() {
  assert(!eventHandling_);
}

void Channel::update() {
  loop_->updateChannel(this);
}

void Channel::handleEvent() {
  eventHandling_ = true;
  if (revents_ & POLLNVAL) {
    LOG(WARNING) << "Channel::handle_event() POLLNVAL";
  }
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG(WARNING) << "Channel::handle_event() POLLHUP";
    if (closeCallback_) {
      closeCallback_();
    }
  }
  if (revents_ & (POLLERR | POLLNVAL)) {
    if (errorCallback_) {
      errorCallback_();
    }
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (readCallback_) {
      readCallback_();
    }
  }
  if (revents_ & POLLOUT) {
    if (writeCallback_) {
      writeCallback_();
    }
  }
  eventHandling_ = false;
}



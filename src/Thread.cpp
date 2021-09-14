#include "Thread.hpp"

using namespace mymuduo;

void Thread::start() {
  thread_ = std::thread(func_);
  started_ = true;
}

void Thread::join() {
  thread_.join();
}

#ifndef MYMUDUO__NONCOPYABLE_HPP_
#define MYMUDUO__NONCOPYABLE_HPP_

class noncopyable {
 public:
  noncopyable() = default;
  ~noncopyable() = default;
  noncopyable(const noncopyable &) = delete;
  const noncopyable &operator=(const noncopyable &) = delete;
};

#endif //MYMUDUO__NONCOPYABLE_HPP_

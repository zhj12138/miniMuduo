#ifndef MYMUDUO__NONCOPYABLE_HPP_
#define MYMUDUO__NONCOPYABLE_HPP_

class noncopyable {
 public:
  noncopyable(const noncopyable &) = delete;
  const noncopyable &operator=(const noncopyable &) = delete;
 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

#endif //MYMUDUO__NONCOPYABLE_HPP_

#ifndef MYMUDUO__CAST_HPP_
#define MYMUDUO__CAST_HPP_

namespace mymuduo {

template<typename To, typename From>
inline To implicit_cast(From const &f) {
  return f;
}

}
#endif //MYMUDUO__CAST_HPP_

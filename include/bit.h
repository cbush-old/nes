#ifndef BIT_H
#define BIT_H

#include <cctype>

// sexy bisqwit
template<size_t position, size_t length=1, typename T=uint32_t>
struct bit {
  T value;
  constexpr unsigned mask(){ return (1u << length) - 1u; };
  constexpr unsigned not_mask_at_pos(){ return ~(mask() << position); };
  template<typename T2>
  bit& operator= (T2 that){
    value &= not_mask_at_pos();
    value |= (that & mask()) << position;
  }
  operator unsigned() const {
    return (value >> position) & mask();
  }
  bit& operator++(){
    return *this = *this + 1;
  }
  unsigned operator++(int){
    unsigned r = *this;
    ++*this;
    return r;
  }
};

#endif


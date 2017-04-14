#ifndef BIT_H
#define BIT_H

#include <cctype>
#include <cstdint>

// Highly bisqwit-inspired bit class.

template <size_t position, size_t length = 1, typename T = uint32_t>
class bit
{
public:
    constexpr T mask() const
    {
        return (1u << length) - 1u;
    };

    constexpr T not_mask_at_pos() const
    {
        return ~(mask() << position);
    };

    template <typename T2>
    bit &operator=(T2 that)
    {
        _value &= not_mask_at_pos();
        _value |= (that & mask()) << position;
        return *this;
    }

    inline operator T() const
    {
        return (_value >> position) & mask();
    }

    bit &operator++()
    {
        return *this = *this + 1;
    }

    T operator++(int)
    {
        T r = *this;
        ++*this;
        return r;
    }
    
private:
    T _value;
};

#endif

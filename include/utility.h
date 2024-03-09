#ifndef _UTILITY_H
#define _UTILITY_H

#include "types.h"

namespace evo {

struct in_place_t {
    explicit in_place_t() = default;
};
constexpr in_place_t in_place {};

template <size_t N>
struct in_place_index_t {
    explicit in_place_index_t() = default;
};

template <size_t N>
inline constexpr in_place_index_t<N> in_place_index {};

}

#endif

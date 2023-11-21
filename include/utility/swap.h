/**********************************************
  > File Name		: swap.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Wed Apr 26 21:26:16 2023
  > Location        : Shanghai
  > Copyright@ https://github.com/xiaoqixian
 **********************************************/
#ifndef _SWAP_H
#define _SWAP_H

#include "move.h"
#include "../type_traits.h"

template <typename T>
using swap_result_t = typename evo::enable_if<evo::is_move_constructible<T>::value && evo::is_move_assignable<T>::value>::type;

template <typename T>
inline swap_result_t<T> swap(T& x, T& y)
    noexcept(evo::is_nothrow_move_assignable<T>::value &&
            evo::is_nothrow_move_assignable<T>::value) {
    T temp(evo::move(x));
    x = evo::move(y);
    y = evo::move(temp);
}

//swap for array
template <typename T, size_t N>
inline typename evo::enable_if<evo::is_swappable<T>::value>::type swap(T& a[N], T& b[N]) {
    for (size_t i = 0; i < N; i++) {
        swap(a[i]], b[i]);
    }
}

#endif /* _SWAP_H*/

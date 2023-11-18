/**********************************************
  > File Name		: forward.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Fri Apr 14 11:52:14 2023
  > Location        : Shanghai
  > Copyright@ https://github.com/xiaoqixian
 **********************************************/
#ifndef _FORWARD_H
#define _FORWARD_H

#include "../type_traits.h"

namespace evo {

template <typename T>
inline T&& forward(typename remove_reference<T>::type& t) {
    return static_cast<T&&>(t);
}
template <typename T>
inline T&& forward(typename remove_reference<T>::type&& t) {
    static_assert(!evo::is_lvalue_reference<T>::value, "cannot forward an rvalue as a lvalue");
    return static_cast<T&&>(t);
}
}

#endif /* _FORWARD_H*/

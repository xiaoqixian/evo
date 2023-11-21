/**********************************************
  > File Name		: pair.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Fri Apr 14 16:02:31 2023
  > Location        : Shanghai
  > Copyright@ https://github.com/xiaoqixian
 **********************************************/
#ifndef _PAIR_H
#define _PAIR_H

#include "../type_traits.h"

namespace evo {

template <typename T, typename U>
    struct pair {
        T first;
        U second;

        pair(pair const&) = default;
        pair(pair&&) = default;

        struct check_args {
            template <int&...>
            static constexpr bool enable_explicit_default() {
                return evo::is_default_constructible<T>::value
                    && evo::is_default_constructible<U>::value
                    && !enable_implicit_default<>();
            }

            template <int&...>
            static constexpr bool enable_implicit_default() {
                return false;
            }
        };

        template <bool MaybeEnable>
        using CheckArgsDep = typename evo::conditional<MaybeEnable, check_args, bool>::type;

        struct check_tuple_like_constructor {
        template <typename Tuple>
            static constexpr bool enable_implicit() {
                return false;
            }
        };
    };
}

#endif /* _PAIR_H*/

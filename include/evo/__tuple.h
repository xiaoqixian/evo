/**********************************************
  > File Name		: tuple.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sat Apr 15 15:34:27 2023
  > Location        : Shanghai
  > Copyright@ https://github.com/xiaoqixian
 **********************************************/
#ifndef _TUPLE_H
#define _TUPLE_H

#include "type_traits.h"
#include <type_traits>

namespace evo {

template <typename... T>
class tuple;

/*tuple_size ================== */
template <typename T>
struct tuple_size;

//enable_if_tuple_size_imp essentially is T
//the parameter list is for conditional compiling
template <typename T, typename...>
using enable_if_tuple_size_imp = T;

//the actual tuple_size implementation
template <typename... T>
struct tuple_size<tuple<T...>>
    : public evo::integral_constant<size_t, sizeof...(T)> {};

//const T partial specialization
//to remove const specifier and make sure no volatile specifier
template <typename T>
struct tuple_size<enable_if_tuple_size_imp<
    const T,
    typename evo::enable_if<!std::is_volatile<T>::value>::type,
    evo::integral_constant<size_t, sizeof(tuple_size<T>)>>>
    : public evo::integral_constant<size_t, tuple_size<T>::value> {};

//volatile T partial specialization
//same thing
template <typename T>
struct tuple_size<enable_if_tuple_size_imp<
    volatile T,
    typename evo::enable_if<!std::is_const<T>::value>::type,
    evo::integral_constant<size_t, sizeof(tuple_size<T>)>>>
    : public evo::integral_constant<size_t, tuple_size<T>::value> {};

//const volatile T
template <typename T>
struct tuple_size<enable_if_tuple_size_imp<
    const volatile T,
    evo::integral_constant<size_t, sizeof(tuple_size<T>)>>>
    : public evo::integral_constant<size_t, tuple_size<T>::value> {};


/*tuple specialization=============*/
template <size_t...> struct tuple_indices {};

template <typename IdxType, IdxType... values>
struct integer_sequence {
    template <template <typename OIdxType, OIdxType...> class ToIndexSeq, class ToIndexType>
    using convert = ToIndexSeq<ToIndexType, values...>;

    template <size_t sp>
    using to_tuple_indices = tuple_indices<(values + sp)...>;
};

namespace detail {
template <typename T, size_t... extra> struct repeat;
template <typename T, T... np, size_t... extra>
    struct repeat<integer_sequence<T, np...>, extra...> {
        typedef integer_sequence<T, np...,
                sizeof...(np) + np...,
                2 * sizeof...(np) + np...,
                3 * sizeof...(np) + np...,
                4 * sizeof...(np) + np...,
                5 * sizeof...(np) + np...,
                6 * sizeof...(np) + np...,
                7 * sizeof...(np) + np...,
                extra...> type;
    };
}

/*tuple_element=================*/
//primary template
template <int I, typename Head>
struct tuple_element;

template <int I, typename Head, typename... Tails>
struct tuple_element<I, evo::tuple<Head, Tails...>>
: public tuple_element<I-1, evo::tuple<Tails...>> {};

template <typename Head>
struct tuple_element<0, Head> {typedef Head type;};


//tuple_like
template <typename T>
struct tuple_like: evo::false_type {};
template <typename T>
struct tuple_like<const T>: public tuple_like<T> {};
template <typename T>
struct tuple_like<volatile T>: public tuple_like<T> {};
template <typename T>
struct tuple_like<const volatile T>: public tuple_like<T> {};

template <typename... T>
struct tuple_like<tuple<T...>>: evo::true_type {};

//tuple convertible
}

#endif /* _TUPLE_H*/

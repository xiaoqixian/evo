/**********************************************
  > File Name		: tuple.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sun Apr 16 22:48:25 2023
  > Location        : Shanghai
  > Copyright@ https://github.com/xiaoqixian
 **********************************************/
#ifndef _TUPLE_H
#define _TUPLE_H

#include "types.h"
#include "type_traits.h"
#include "utility/forward.h"
#include "utility/move.h"
#include "memory/allocator_arg_t.h"

//tuple declaration
template <typename... Ts>
struct tuple;

//tuple_element declaration
template <size_t, typename>
class tuple_element;

//tuple_indices
//an empty struct to catch indices
template <size_t...>
struct tuple_indices {};

template <typename>
struct make_tuple_indices_impl;

template <size_t... indices>
struct make_tuple_indices_impl<evo::integer_sequence<indices...>> {
    typedef tuple_indices<indices...> type;
};

//make_tuple_indices
template <size_t N>
struct make_tuple_indices {
    typedef typename make_tuple_indices_impl<typename evo::make_integer_sequence<N>::type>::type type;
};

//tuple_types
template <typename...>
struct tuple_types {};

//maek_tuple_types
template <typename T>
struct make_tuple_types;
template <typename... Ts>
struct make_tuple_types<tuple<Ts...>> {
    typedef tuple_types<Ts...> type;
};

//tuple_constructible
//can a tuple constructed with another one
template <typename, typename>
struct test_tuple_constructible: evo::false_type {};

template <typename... Ts, typename... Us>
struct test_tuple_constructible<tuple_types<Ts...>, tuple_types<Us...>>: evo::And<evo::is_convertible<Ts, Us>...> {};

template <typename T, typename U>
struct tuple_constructible: test_tuple_constructible<
                            typename make_tuple_types<T>::type, 
                            typename make_tuple_types<U>::type> {};


//tuple_leaf
template <size_t, typename T>
struct tuple_leaf {
    T value;
public:
    template <typename U, typename = evo::enable_if<
        evo::And<
            evo::is_not_same<typename evo::remove_cv_ref<U>::type, tuple_leaf>, 
            evo::is_constructible<T, U>
        >::value
    >>
    tuple_leaf(U&& u): value(evo::forward(u)) {}

    template <typename Alloc>
    constexpr tuple_leaf(evo::integral_constant<int, 0>, Alloc const&) {}

    template <typename Alloc>
    constexpr tuple_leaf(evo::integral_constant<int, 1>, Alloc const& a)
        : value(a) {}

    tuple_leaf(tuple_leaf const&) = default;
    tuple_leaf(tuple_leaf&&) = default;

    T& get() {
        return value;
    }

    T const& get() const {
        return value;
    }

    int swap(tuple_leaf& t) 
        noexcept(evo::is_nothrow_swappable<tuple_leaf>::value) {
        evo::swap(*this, t);
        return 0;
    }
};


//primary template
template <typename, typename...>
struct tuple_impl;

template <size_t... indices, typename... Ts>
struct tuple_impl<tuple_indices<indices...>, Ts...>: public tuple_leaf<indices, Ts>... {
    constexpr tuple_impl() noexcept(evo::all<evo::is_nothrow_default_constructible<Ts>::value...>::value) {}

    //partial construction
    template <size_t... N1, typename... Ts1,
              size_t... N2, typename... Ts2,
              typename... U>
    explicit tuple_impl(tuple_indices<N1...>, tuple_types<Ts1...>,
                        tuple_indices<N2...>, tuple_types<Ts2...>,
                        U&&... u)
        noexcept(evo::all<evo::is_nothrow_constructible<Ts1, U>::value...>::value &&
                evo::all<evo::is_nothrow_default_constructible<Ts2>::value...>::value):
        tuple_leaf<N1, Ts1>(evo::forward(u))...,
        tuple_leaf<N2, Ts2>()... {}

    //custom allocator, partial construction
    //template <typename Alloc,
             //size_t... N1, typename... Ts1,
             //size_t... N2, typename... Ts2,
             //typename... U>
    //explicit tuple_impl(evo::allocator_arg_t, Alloc const& a,
            //tuple_indices<N1...>, tuple_types<Ts1...>,
            //tuple_indices<N2...>, tuple_types<Ts2...>,
            //U&&... u):
        //tuple_leaf<N1, Ts1>()

    //construct tuple with another Tuple
    //every type in this tuple must can be constructed with the type 
    //in another tuple at the corresponding index.
    template <typename Tuple, typename = typename evo::enable_if<tuple_constructible<tuple<Ts...>, Tuple>::value>::type>
    tuple_impl(Tuple&& t)
        noexcept(evo::all<evo::is_nothrow_constructible<Ts, typename tuple_element<indices, typename make_tuple_types<Tuple>::type>::type>::value...>::value) 
        : tuple_leaf<indices, Ts>(evo::forward<typename tuple_element<indices, typename make_tuple_types<Tuple>::type>::type>(get<indices>(t)))... {}

    tuple_impl(tuple_impl const&) = default;
    tuple_impl(tuple_impl&&) = default;

    void swap(tuple_impl& t)
        noexcept(evo::all<evo::is_nothrow_swappable<Ts>::value...>::value) {
        swallow(tuple_leaf<indices, Ts>::swap(static_cast<tuple_leaf<indices, Ts>&>(t))...);
    }
};

//just expand a pack and not doing anything
template <typename... T>
constexpr void swallow(T...) {}

template <typename Dest, typename Source, typename... Us, size_t... N>
void memberwise_forward_assign(Dest& dest, Source const& src, tuple_types<Us...>, tuple_indices<N...>) {
    swallow(((get<N>(dest) = evo::forward<Us>(get<N>(src))))...);
}

template <typename Dest, typename Source, size_t... N>
void memberwise_copy_assign(Dest& dest, Source const& src, tuple_indices<N...>) {
    swallow(((get<N>(dest) = get<N>(src)))...);
}

template <typename... Ts>
class tuple {
private:
    typedef tuple_impl<evo::make_integer_sequence<sizeof...(Ts)>, Ts...> Base;
    Base base;

public:
    //tuple() constructor
    //requires all types to be implicitly default constructible
    template <template <typename...> typename IsImplDefault = evo::is_implicitly_default_constructible, evo::enable_if<evo::And<IsImplDefault<Ts>...>::value, int> = 0>
    tuple() 
        noexcept(evo::And<evo::is_nothrow_default_constructible<Ts>...>::value) {}

    //explicit constructor
    //make sure all types are default constructible but not implicitly default constructible
    template <
        template <typename...> typename IsImplDefault = evo::is_implicitly_default_constructible, 
        template <typename...> typename IsDefault = evo::is_default_constructible, 
        evo::enable_if<evo::And<IsDefault<Ts>..., evo::Not<evo::And<IsImplDefault<Ts>...>>>::value, int> = 0> //explicit check
    explicit tuple()
        noexcept(evo::And<evo::is_nothrow_default_constructible<Ts>...>::value) {}

    //tuple(Ts&...) constructor
    template <template <typename...> class And = evo::And, evo::enable_if<evo::And<
        evo::bool_constant<sizeof...(Ts) >= 1>,
        evo::is_copy_constructible<Ts>...,
        evo::is_convertible<const Ts&, Ts>...
        >::value, int> = 0> 
    tuple(const Ts&... t)
    noexcept(evo::And<evo::is_nothrow_copy_constructible<Ts>...>::value)
        : base(typename make_tuple_indices<sizeof...(Ts)>::type(),
                typename make_tuple_types<tuple<Ts...>>::type(),
                typename make_tuple_indices<0>::type(),
                typename make_tuple_types<tuple<>>::type(),
                t...) {}

    //tuple(U&&...) constructor
    //as U may be a tuple, so we have differ it from inner types.
    template <typename...> struct is_tuple: evo::false_type {};
    template <typename U> struct is_tuple<U>: evo::is_same<typename evo::remove_cv_ref<U>::type, tuple> {};

    template <typename... Us, evo::enable_if<
        evo::And<
            evo::bool_constant<sizeof...(Us) == sizeof...(Ts)>,
            evo::And<
                evo::bool_constant<sizeof...(Ts) >= 1>,
                evo::Not<is_tuple<Us...>>,
                evo::is_constructible<Ts, Us>...
            >,
            evo::is_convertible<Ts, Us>...
        >::value, int> = 0>
    tuple(Us&&... u) noexcept(evo::And<evo::is_nothrow_constructible<Ts, Us>...>::value)
    : base(typename make_tuple_indices<sizeof...(Us)>::type(),
            typename make_tuple_types<tuple<Us...>>::type(),
            typename make_tuple_indices<0>::type(),
            typename make_tuple_types<tuple<>>::type(),
            evo::forward<Us>(u)...) {}

    tuple(const tuple&) = default;
    tuple(tuple&&) = default;

    template <typename... Us>
    struct EnableCopyFromOtherTuple: evo::And<
        evo::Not<evo::is_same<tuple<Ts...>, tuple<Us...>>>,
        evo::Or<
            evo::bool_constant<sizeof...(Ts) != 1>,
            evo::And<
                evo::Not<evo::is_convertible<const tuple<Us>&, Ts>>...,
                evo::Not<evo::is_constructible<Ts, const tuple<Us>&>>...
            >
        >,
        evo::is_constructible<Ts, const Us&>...
    > {};

    //tuple(tuple const&) constructor
    //if all const Us& is convertible to Ts
    template <typename... Us, evo::enable_if<
        evo::And<
            evo::bool_constant<sizeof...(Us) == sizeof...(Ts)>,
            EnableCopyFromOtherTuple<Us...>,
            evo::is_convertible<const Us&, Ts>...
        >::value
    , int> = 0>
    tuple(const tuple<Us...>& t) 
        noexcept(evo::And<evo::is_nothrow_constructible<Ts, const Us&>...>::value): base(t) {}

    //tuple(tuple const&) constructor
    //if not all const Us& is convertible to Ts
    template <typename... Us, evo::enable_if<
        evo::And<
            evo::bool_constant<sizeof...(Us) == sizeof...(Ts)>,
            EnableCopyFromOtherTuple<Us...>,
            evo::Not<evo::And<evo::is_convertible<const Us&, Ts>...>>
        >::value
    , int> = 0>
    explicit tuple(const tuple<Us...>& t)
        noexcept(evo::And<evo::is_nothrow_constructible<Ts, const Us&>...>::value): base(t) {}

    template <typename... Us>
    struct EnableMoveFromOtherTuple: evo::And<
        evo::Not<evo::is_same<tuple<Ts...>, tuple<Us...>>>,
        evo::Or<
            evo::bool_constant<sizeof...(Ts) != 1>,
            evo::And<
                evo::Not<evo::is_convertible<tuple<Us>, Ts>>...,
                evo::Not<evo::is_constructible<Ts, tuple<Us>>>...
            >
        >,
        evo::is_constructible<Ts, Us>...
    > {};

    //tuple(tuple&&) constructor
    template <typename... Us, evo::enable_if<
        evo::And<
            evo::bool_constant<sizeof...(Us) == sizeof...(Ts)>,
            EnableMoveFromOtherTuple<Us...>,
            evo::is_convertible<Us, Ts>...
        >::value
    , int> = 0>
    tuple(tuple<Us...>&& t)
        noexcept(evo::And<evo::is_nothrow_constructible<Ts, Us>...>::value): base(evo::move(t)) {}

    //explicit tuple(tuple&&) constructor
    template <typename... Us, evo::enable_if<
        evo::And<
            evo::bool_constant<sizeof...(Us) == sizeof...(Ts)>,
            EnableMoveFromOtherTuple<Us...>,
            evo::Not<evo::And<evo::is_convertible<Us, Ts>...>>
        >::value
    , int> = 0>
    explicit tuple(tuple<Us...>&& t)
        noexcept(evo::And<evo::is_nothrow_constructible<Ts, Us>...>::value): base(evo::move(t)) {}

    //[tuple.assign]
    //memberwise copy assign
    //can't use SFINAE, so use evo::nat if failed.
    tuple& operator=(evo::conditional<evo::And<evo::is_copy_assignable<Ts>...>::value, tuple, evo::nat> const& t) 
        noexcept(evo::And<evo::is_nothrow_copy_assignable<Ts>...>::value) {
        memberwise_copy_assign(*this, t, typename make_tuple_indices<sizeof...(Ts)>::type());
        return *this;
    }

    //memberwise move assign
    tuple& operator=(evo::conditional<evo::And<evo::is_move_assignable<Ts>...>::value, tuple, evo::nat> const& t)
        noexcept(evo::And<evo::is_nothrow_move_assignable<Ts>...>::value) {
        memberwise_forward_assign(*this, t, typename make_tuple_indices<sizeof...(Ts)>::type());
        return *this;
    }

    //memberwise copy assign
    //but with different types
    template <typename... Us, evo::enable_if<
        evo::And<
            evo::bool_constant<sizeof...(Ts) == sizeof...(Us)>,
            evo::Not<evo::is_same<tuple<Ts...>, tuple<Us...>>>,
            evo::is_assignable<Ts&, Us const&>...
        >::value, int> = 0>
    tuple& operator=(tuple<Us...> const& t)
    noexcept(evo::And<evo::is_nothrow_assignable<Ts&, Us const&>...>::value) {
        memberwise_copy_assign(*this, t, typename make_tuple_indices<sizeof...(Ts)>::type());
        return *this;
    }

    //memberwise move assign
    //but with different types
    template <typename... Us, evo::enable_if<
        evo::And<
            evo::bool_constant<sizeof...(Ts) == sizeof...(Us)>,
            evo::is_assignable<Ts&, Us>...
        >::value, int> = 0>
    tuple& operator=(tuple<Us...>&& t)
    noexcept(evo::And<evo::is_nothrow_assignable<Ts&, Us>...>::value) {
        memberwise_forward_assign(*this, t, typename make_tuple_indices<sizeof...(Ts)>::type());
        return *this;
    }

    //[tuple.swap]
    void swap(tuple& t) noexcept(evo::all<evo::is_nothrow_swappable<Ts>::value...>::value) {
        base.swap(t.base);
    }
};

//tuple_element
template <size_t, typename T>
struct tuple_element_leaf {
    typedef T type;
};

template <typename, typename...>
struct tuple_element_impl;

template <size_t... indices, typename... Ts>
struct tuple_element_impl<evo::integer_sequence<indices...>, Ts...>: tuple_element_leaf<indices, Ts>... {};

template <size_t index, typename T>
static typename tuple_element_leaf<index, T>::type at_index_type(tuple_element_leaf<index, T> const&);

template <size_t index, typename... Ts>
class tuple_element<index, tuple<Ts...>> {
    typedef decltype(at_index_type<index>(tuple_element_impl<evo::make_integer_sequence<sizeof...(Ts)>, Ts...>{})) type;
};

//get
template <size_t index, typename... Ts>
static typename tuple_element<index, Ts...>::type& get(tuple<Ts...>& t) {
    typedef typename tuple_element<index, Ts...>::type type;
    return t.base.tuple_leaf<index, type>.get();
}

template <size_t index, typename... Ts>
static typename tuple_element<index, Ts...>::type const& get(tuple<Ts...> const& t) {
    typedef typename tuple_element<index, Ts...>::type type;
    return t.base.tuple_leaf<index, type>::get();
}
#endif /* _TUPLE_H*/

/**********************************************
  > File Name		: type_traits.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Fri Apr 14 11:50:15 2023
  > Location        : Shanghai
  > Copyright@ https://github.com/xiaoqixian
 **********************************************/
#ifndef _TYPE_TRAITS_H
#define _TYPE_TRAITS_H

#include "utility/declval.h"
#include "types.h"
#include <type_traits>

namespace evo {

//true/false type
template <bool b>
struct bool_constant {
    static const bool value = b;
};

typedef bool_constant<true> true_type;
typedef bool_constant<false> false_type;

//integral_constant
template <typename T, T v>
struct integral_constant {
    static const T value = v;
    typedef T value_type;
    typedef integral_constant type;
    operator value_type() const {return value;}
    constexpr value_type operator ()() const {return value;}
};

//enable_if
template <bool, typename T = void>
struct enable_if {};
template <typename T>
struct enable_if<true, T> {typedef T type;};

//if two types is the same
template <typename T, typename U>
struct is_same: false_type {};
template <typename T>
struct is_same<T, T>: true_type {};

template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;
template <typename T, typename U>
inline constexpr bool is_not_same_v = !is_same<T, U>::value;

// And trait, inherit true_type if all type parameters 
// are true_type, otherwise inherit false_type.
template <bool p1, typename... Rest>
struct AndImpl;

template <>
struct AndImpl<true>: true_type {};
template <typename P1, typename... Rest>
struct AndImpl<true, P1, Rest...>: AndImpl<P1::value, Rest...> {};
template <typename... Rest>
struct AndImpl<false, Rest...>: false_type {};

template <typename... Pred>
struct And;

template <>
struct And<>: true_type {};
template <typename P, typename... Rest>
struct And<P, Rest...>: AndImpl<P::value, Rest...> {};

template <typename... Ts>
struct Xor;

template <typename A, typename B>
struct Xor<A, B> {
    static const bool value = A::value ^ B::value;
};

template <typename A, typename B, typename... Ts>
struct Xor<A, B, Ts...> {
    static const bool value = A::value ^ Xor<B, Ts...>::value;
};

//and helper
//like multiple enable_if
//template <typename...>
//using expand_to_true = true_type;
////every Pred::value has to be true.
//template <typename... Pred>
//expand_to_true<enable_if<Pred::value>...> and_helper(int);
//template <typename...>
//false_type and_helper(...);

//template <typename... Pred>
//using And = decltype(and_helper<Pred...>(0));

//is_not_same
template <typename T, typename U>
struct is_not_same: bool_constant<!is_same<T, U>::value> {};

//Not
template <typename Pred>
struct Not: bool_constant<!Pred::value> {};

//Or
template <typename... Pred>
struct Or: evo::Not<evo::And<evo::Not<Pred>...>> {};

//void_t
template <typename>
struct void_t {typedef void type;};

//is_referenceable
struct is_referenceable_impl {
    template <typename T> static T& test(int);
    template <typename T> static void test(...);
};

template <typename T>
struct is_referenceable: bool_constant<!is_same<decltype(is_referenceable_impl::test<T>(0)), void>::value> {};

//add lvalue reference
template <typename T, bool = is_referenceable<T>::value>
struct add_lvalue_reference {
    typedef T type;
};
template <typename T>
struct add_lvalue_reference<T, true> {
    typedef T& type;
};
template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

//add rvalue reference
template <typename T, bool = is_referenceable<T>::value>
struct add_rvalue_reference {
    typedef T type;
};
template <typename T>
struct add_rvalue_reference<T, true> {
    typedef T&& type;
};
template <typename T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

//remove reference
template <typename T>
struct remove_reference {typedef T type;};
template <typename T>
struct remove_reference<T&> {typedef T type;};
template <typename T>
struct remove_reference<T&&> {typedef T type;};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

//remove const specifier
template <typename T>
struct remove_const {typedef T type;};
template <typename T>
struct remove_const<const T> {typedef T type;};

//remove volatile specifier
template <typename T>
struct remove_volatile {typedef T type;};
template <typename T>
struct remove_volatile<volatile T> {typedef T type;};

//remove CV specifier
template <typename T>
struct remove_cv {typedef T type;};
template <typename T>
struct remove_cv<const T> {typedef T type;};
template <typename T>
struct remove_cv<volatile T> {typedef T type;};
template <typename T>
struct remove_cv<const volatile T> {typedef T type;};

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

//remove CV specifier and reference
template <typename T>
struct remove_cv_ref {
    typedef typename remove_cv<typename remove_reference<T>::type>::type type;
};
template <typename T>
using remove_cv_ref_t = typename remove_cv<T>::type;

template <typename T>
struct remove_cvref {
    typedef typename remove_cv<typename remove_reference<T>::type>::type type;
};
template <typename T>
using remove_cvref_t = typename remove_cv<T>::type;

//remove_extent
template <typename T>
struct remove_extent {typedef T type;};
template <typename T>
struct remove_extent<T[]> {typedef T type;};
template <typename T, size_t N>
struct remove_extent<T[N]> {typedef T type;};

template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

//remove all extents
template <typename T>
struct remove_all_extents {typedef T type;};
template <typename T>
struct remove_all_extents<T[]> {
    typedef typename remove_all_extents<T>::type type;
};
template <typename T, size_t N>
struct remove_all_extents<T[N]> {
    typedef typename remove_all_extents<T>::type type;
};

//add pointer 
template <typename T, bool = 
    is_referenceable<T>::value ||
    is_same<typename remove_cv<T>::type, void>::value>
struct add_pointer_impl {typedef T* type;};

template <typename T>
struct add_pointer_impl<T, false> {typedef T type;};

template <typename T>
struct add_pointer {
    typedef typename add_pointer_impl<T>::type type;
};

template <typename T>
using add_pointer_t = typename add_pointer<T>::type;

//conditional
template <bool b, typename If, typename Then>
struct conditional {typedef If type;};
template <typename If, typename Then>
struct conditional<false, If, Then> {typedef Then type;};

template <bool b, typename If, typename Then>
using conditional_t = typename conditional<b, If, Then>::type;

//add const
template <typename T>
struct add_const {typedef const T type;};

//all
//true_type if all Pred is true
template <bool... Pred>
struct all_helper {};

template <bool... Pred>
using all = is_same<all_helper<Pred...>, all_helper<((void)Pred, true)...>>;

//is_lvalue_reference
template <typename T>
struct is_lvalue_reference: bool_constant<std::is_lvalue_reference_v<T>> {};
template <typename T>
constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

//is_rvalue_reference
template <typename T>
struct is_rvalue_reference: bool_constant<std::is_rvalue_reference_v<T>> {};

//make_integer_sequence
template <size_t... values>
struct integer_sequence {
    using type = integer_sequence;
    //using to_tuple_indices = evo::tuple_indices<values...>;
};

template <typename, typename>
struct concat_integer_sequence;
template <size_t... N1, size_t... N2>
struct concat_integer_sequence<integer_sequence<N1...>, integer_sequence<N2...>>: integer_sequence<N1..., (sizeof...(N1)+N2)...> {};

template <size_t N>
struct make_integer_sequence: concat_integer_sequence<integer_sequence<N/2>, integer_sequence<N-N/2>>::type {};
template <>
struct make_integer_sequence<0>: integer_sequence<> {};
template <>
struct make_integer_sequence<1>: integer_sequence<0> {};


//is_signed_integer
template <typename> struct is_signed_integer : public false_type {};
template <> struct is_signed_integer<signed char>      : public true_type {};
template <> struct is_signed_integer<signed short>     : public true_type {};
template <> struct is_signed_integer<signed int>       : public true_type {};
template <> struct is_signed_integer<signed long>      : public true_type {};

//is_unsigned_integer
template <typename> struct is_unsigned_integer : public false_type {};
template <> struct is_unsigned_integer<unsigned char>      : public true_type {};
template <> struct is_unsigned_integer<unsigned short>     : public true_type {};
template <> struct is_unsigned_integer<unsigned int>       : public true_type {};
template <> struct is_unsigned_integer<unsigned long>      : public true_type {};

//is_integer
template <typename T>
struct _is_integral: Or<is_signed_integer<T>, is_unsigned_integer<T>> {};
template <typename T>
struct is_integral: _is_integral<typename remove_cv<T>::type> {};

//is floating point
template <typename> struct _is_floating_point: public false_type {};
template <> struct _is_floating_point<float>: public true_type {};
template <> struct _is_floating_point<double>: public true_type {};
template <> struct _is_floating_point<long double>: public true_type {};

template <typename T>
struct is_floating_point: _is_floating_point<typename remove_cv<T>::type> {};

//TODO not finished
template <typename T>
struct is_enum {
    static const bool value = true;
};

//is_array
template <typename>
struct is_array: false_type {};
template <typename T>
struct is_array<T[]>: true_type {};
template <size_t N, typename T>
struct is_array<T[N]>: true_type {};

template <typename T>
inline constexpr bool is_array_v = is_array<T>::value;

//is_cv_array
template <typename T>
struct is_cv_array: std::false_type {};

template <typename T>
struct is_cv_array<T const[]>: std::true_type {};
template <typename T>
struct is_cv_array<T volatile[]>: std::true_type {};
template <typename T>
struct is_cv_array<T const volatile[]>: std::true_type {};

template <typename T, size_t N>
struct is_cv_array<T const[N]>: std::true_type {};
template <typename T, size_t N>
struct is_cv_array<T volatile[N]>: std::true_type {};
template <typename T, size_t N>
struct is_cv_array<T const volatile[N]>: std::true_type {};

template <typename T>
constexpr bool is_cv_array_v = is_cv_array<T>::value;

//is_void
template <typename>
struct is_void: false_type {};
template <>
struct is_void<void>: true_type {};

template <typename T>
constexpr bool is_void_v = is_void<T>::value;

//is_const
template <typename>
struct is_const: false_type {};
template <typename T>
struct is_const<const T>: true_type {};

//is_reference
template <typename>
struct is_reference: false_type {};
template <typename T>
struct is_reference<T&>: true_type {};
template <typename T>
struct is_reference<T&&>: true_type {};

template <typename T>
inline constexpr bool is_reference_v = is_reference<T>::value;

//is_function
//as function is not referenceable and cannot put const specifier on it.
template <typename T>
struct is_function: bool_constant<!(is_reference<T>::value || is_const<const T>::value)> {};

//is_pointer
template <typename>
struct is_pointer: false_type {};
template <typename T>
struct is_pointer<T*>: true_type {};

// -------- is_member_pointer ---------------------
template <typename>
struct libevo_is_member_pointer {
    enum {
        is_member = false,
        is_func = false,
        is_obj = false
    };
};

template <typename T, typename U>
struct libevo_is_member_pointer<T U::*> {
    enum {
        is_member = true,
        is_func = is_function<T>::value,
        is_obj = !is_func
    };
};

template <typename T>
struct is_member_pointer: 
    bool_constant<libevo_is_member_pointer<remove_cv_t<T>>::is_member> {};

//is_member_function_pointer
template <typename T>
struct is_member_function_pointer: 
    bool_constant<libevo_is_member_pointer<remove_cv_t<T>>::is_func> {};

//is_member_object_pointer
template <typename T>
struct is_member_object_pointer: 
    bool_constant<libevo_is_member_pointer<remove_cv_t<T>>::is_obj> {};

//is arithmetic
template <typename T>
struct is_arithmetic: bool_constant<is_integral<T>::value || is_floating_point<T>::value> {};

//is_scalar
//TODO: requires is_null_pointer, is_enum
template <typename T>
struct is_scalar: bool_constant<
                  is_arithmetic<T>::value ||
                  is_member_pointer<T>::value ||
                  is_pointer<T>::value
                  > {};

//is_object
//TODO: requires is_class, is_union
template <typename T>
struct is_object: bool_constant<
                  is_scalar<T>::value ||
                  is_array<T>::value
                  > {};


// ------- constructible --------------------------------------

// TODO: without is_constructible feature version
// #if __has_feature(is_constructible)
template <typename T, typename... Args>
struct is_constructible: bool_constant<std::is_constructible_v<T, Args...>> {};
template <typename T, typename... Args>
inline constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

//is_default_constructible
template <typename T>
struct is_default_constructible: public is_constructible<T> {};
template <typename T>
constexpr bool is_default_constructible_v = is_default_constructible<T>::value;

//is_nothrow_constructible
template <typename T, typename... Args>
struct is_nothrow_constructible: public bool_constant<std::is_nothrow_constructible_v<T, Args...>> {};
template <typename T, typename... Args>
constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T, Args...>::value;

//is_move_constructible
template <typename T>
struct is_move_constructible: public is_constructible<T, typename add_rvalue_reference<T>::type> {};
template <typename T>
constexpr bool is_move_constructible_v = is_move_constructible<T>::value;

//is_copy_constructible
template <typename T>
struct is_copy_constructible: public is_constructible<T, typename add_lvalue_reference<typename add_const<T>::type>::type> {};
template <typename T>
constexpr bool is_copy_constructible_v = is_copy_constructible<T>::value;

//is_nothrow_copy_constructible
template <typename T>
struct is_nothrow_copy_constructible: public is_nothrow_constructible<T, typename add_lvalue_reference<typename add_const<T>::type>::type> {};
template <typename T>
constexpr bool is_nothrow_copy_constructible_v = is_nothrow_copy_constructible<T>::value;

//is_nothrow_default_constructible
template <typename T>
struct is_nothrow_default_constructible: public is_nothrow_constructible<T> {};

//is_nothrow_move_constructible
template <typename T>
struct is_nothrow_move_constructible: public is_nothrow_constructible<
                                      typename add_lvalue_reference<T>::type,
                                      typename add_rvalue_reference<T>::type> {};

template <typename T>
constexpr bool is_nothrow_move_constructible_v = is_nothrow_move_constructible<T>::value;

//is_implicitly_default_constructible
template <typename T>
void test_implicit_default_constructible(T const&);

template <typename T, typename = void, typename = typename is_default_constructible<T>::type>
struct is_implicitly_default_constructible: false_type {};

template <typename T>
struct is_implicitly_default_constructible<T, 
    decltype(test_implicit_default_constructible<T const&>({})), true_type>
        : true_type {};

template <typename T>
struct is_implicitly_default_constructible<T, 
    decltype(test_implicit_default_constructible<T const&>({})), false_type>
        : false_type {};

//is_trivially_destructible
template <typename T, typename... Args>
struct is_trivially_constructible: false_type {};

template <typename T>
struct is_trivially_constructible<T>: bool_constant<is_scalar<T>::value> {};

//is_trivially_copy_constructible
template <typename T>
struct is_trivially_copy_constructible: is_trivially_constructible<T, typename add_lvalue_reference<T>::type> {};
template <typename T>
constexpr bool is_trivially_copy_constructible_v = is_trivially_copy_constructible<T>::value;

//is_trivially_move_constructible
template <typename T>
struct is_trivially_move_constructible: is_trivially_constructible<T, typename add_rvalue_reference<T>::type> {};
template <typename T>
constexpr bool is_trivially_move_constructible_v = is_trivially_move_constructible<T>::value;

// #endif // #if __has_feature(is_constructible)


// #if __has_feature(__is_destructible)

template <typename T>
using is_destructible = bool_constant<std::is_destructible_v<T>>;

template <typename T>
using is_destructible_v = bool_constant<std::is_destructible_v<T>>::value;

// #endif // #if __has_keyword(is_destructible)

//is_trivially_destructible
template <typename T>
struct trivial_destructor: bool_constant<is_scalar<T>::value || is_reference<T>::value> {};

template <typename T>
struct is_trivially_destructible: trivial_destructor<typename remove_all_extents<T>::type> {};
template <typename T>
struct is_trivially_destructible<T[]>: false_type {};

/// ----------- move -------------------
template <typename T>
inline typename remove_reference<T>::type&&
move(T&& t) noexcept {
    typedef typename remove_reference<T>::type U;
    return static_cast<U&&>(t);
}
template <typename T>
inline typename remove_reference<T>::type&&
move(T& t) noexcept {
    typedef typename remove_reference<T>::type U;
    return static_cast<U&&>(t);
}

/// -------- forward ------------------
template <typename T>
inline T&& forward(typename remove_reference<T>::type& t) {
    return static_cast<T&&>(t);
}
template <typename T>
inline T&& forward(typename remove_reference<T>::type&& t) {
    static_assert(!evo::is_lvalue_reference<T>::value, "cannot forward an rvalue as a lvalue");
    return static_cast<T&&>(t);
}

//is_convertible
//if convertible, the test function is valid.
template <typename T>
void is_convertible_test(T);

template <typename, typename, typename = void>
struct is_convertible_impl: false_type {};

template <typename From, typename To>
struct is_convertible_impl<From, To, decltype(is_convertible_test<To>(declval<From>()))>: true_type {};

template <typename From, typename To>
struct is_convertible: is_convertible_impl<From, To> {};

template <typename From, typename To>
inline constexpr bool is_convertible_v = is_convertible<From, To>::value;


//is_assignable
template <typename T, typename U>
struct is_assignable: bool_constant<std::is_assignable_v<T, U>> {};
template <typename T, typename U>
constexpr bool is_assignable_v = is_assignable<T, U>::value;

//is_nothrow_assignable
template <typename T, typename Arg>
struct is_nothrow_assignable: public bool_constant<std::is_nothrow_assignable_v<T, Arg>> {};

//is_move_assignable
template <typename T>
struct is_move_assignable: is_assignable<
                           typename add_lvalue_reference<T>::type,
                           typename add_rvalue_reference<T>::type> {};
template <typename T>
constexpr bool is_move_assignable_v = is_move_assignable<T>::value;

//is_nothrow_move_assignable
template <typename T>
struct is_nothrow_move_assignable: public bool_constant<is_nothrow_constructible<
                                   typename add_lvalue_reference<T>::type, 
                                   typename add_rvalue_reference<T>::type
                                   >::value> {};
template <typename T>
constexpr bool is_nothrow_move_assignable_v = is_nothrow_move_assignable<T>::value;

//is_copy_assignable
template <typename T>
struct is_copy_assignable: is_assignable<
                           typename add_lvalue_reference<T>::type, 
                           typename add_lvalue_reference<T>::type> {};
template <typename T>
constexpr bool is_copy_assignable_v = is_copy_assignable<T>::value;

//is_nothrow_copy_assignable
template <typename T>
struct is_nothrow_copy_assignable: bool_constant<is_nothrow_constructible<
                                   typename add_lvalue_reference<T>::type,
                                   typename add_lvalue_reference<T>::type
                                   >::value> {};
template <typename T>
constexpr bool is_nothrow_copy_assignable_v = is_nothrow_copy_assignable<T>::value;

//is_trivially_assignable
template <typename T, typename Arg>
struct is_trivially_assignable: false_type {};

template <typename T>
struct is_trivially_assignable<T&, T>: bool_constant<is_scalar<T>::value> {};
template <typename T>
struct is_trivially_assignable<T&, T&>: bool_constant<is_scalar<T>::value> {};
template <typename T>
struct is_trivially_assignable<T&, T const&>: bool_constant<is_scalar<T>::value> {};
template <typename T>
struct is_trivially_assignable<T&, T&&>: bool_constant<is_scalar<T>::value> {};

//is_trivially_copy_assignable
template <typename T>
struct is_trivially_copy_assignable: is_trivially_assignable<
                                     typename add_lvalue_reference<T>::type,
                                     typename add_lvalue_reference<typename add_const<T>::type>::type> {};

//is_trivially_move_assignable
template <typename T>
struct is_trivially_move_assignable: is_trivially_assignable<
                                     typename add_lvalue_reference<T>::type,
                                     typename add_rvalue_reference<T>::type> {};

//swappable
template <typename T>
struct is_swappable;
template <typename T>
struct is_nothrow_swappable;

//swap
template <typename T>
using swap_result_t = typename enable_if<is_move_constructible<T>::value && is_move_assignable<T>::value>::type;

//implementation in utility/swap.h
template <typename T>
inline swap_result_t<T> swap(T& x, T& y) noexcept(is_nothrow_move_constructible<T>::value && is_nothrow_move_assignable<T>::value);

template <typename T, size_t N>
inline typename enable_if<is_swappable<T>::value>::type swap(T (&a)[N], T(&b)[N])
    noexcept(is_nothrow_swappable<T>::value);

//swappable with
struct nat {
    nat() = delete;
    nat(nat const&) = delete;
    nat& operator=(nat const&) = delete;
    ~nat() = delete;
};

template <typename T, typename U = T, bool = !is_void<T>::value && !is_void<U>::value>
struct swappable_with {
    template <typename L, typename R>
    static decltype(swap(declval<L>(), declval<R>())) test_swap(int);
    template <typename, typename>
    static nat test_swap(long);

    typedef decltype(test_swap<T, U>(0)) swap1;
    typedef decltype(test_swap<U, T>(0)) swap2;

    static const bool value = is_not_same<swap1, nat>::value && is_not_same<swap2, nat>::value;
};

template <typename T, typename U>
struct swappable_with<T, U, false>: false_type {};

//nothrow_swappable_with
template <typename T, typename U = T, bool swappable = swappable_with<T, U>::value>
struct nothrow_swappable_with {
    static const bool value = 
        noexcept(swap(declval<T>(), declval<U>()))
    &&  noexcept(swap(declval<U>(), declval<T>()));
};

template <typename T, typename U>
struct nothrow_swappable_with<T, U, false>: false_type {};

template <typename T>
T* address_of(T& t) noexcept {
    return reinterpret_cast<T*>(const_cast<char*>(&reinterpret_cast<const volatile char&>(t)));
}

//sfinae constructor base
template <bool can_copy, bool can_move>
struct sfinae_ctor_base;

template <>
struct sfinae_ctor_base<true, true> {
    sfinae_ctor_base() = default;
    sfinae_ctor_base(sfinae_ctor_base const&) = default;
    sfinae_ctor_base(sfinae_ctor_base&&) = default;
    sfinae_ctor_base& operator=(sfinae_ctor_base const&) = default;
    sfinae_ctor_base& operator=(sfinae_ctor_base&&) = default;
};

template <>
struct sfinae_ctor_base<false, true> {
    sfinae_ctor_base() = default;
    sfinae_ctor_base(sfinae_ctor_base const&) = delete;
    sfinae_ctor_base(sfinae_ctor_base&&) = default;
    sfinae_ctor_base& operator=(sfinae_ctor_base const&) = default;
    sfinae_ctor_base& operator=(sfinae_ctor_base&&) = default;
};
template <>
struct sfinae_ctor_base<true, false> {
    sfinae_ctor_base() = default;
    sfinae_ctor_base(sfinae_ctor_base const&) = default;
    sfinae_ctor_base(sfinae_ctor_base&&) = delete;
    sfinae_ctor_base& operator=(sfinae_ctor_base const&) = default;
    sfinae_ctor_base& operator=(sfinae_ctor_base&&) = default;
};
template <>
struct sfinae_ctor_base<false, false> {
    sfinae_ctor_base() = default;
    sfinae_ctor_base(sfinae_ctor_base const&) = delete;
    sfinae_ctor_base(sfinae_ctor_base&&) = delete;
    sfinae_ctor_base& operator=(sfinae_ctor_base const&) = default;
    sfinae_ctor_base& operator=(sfinae_ctor_base&&) = default;
};

//sfinae assign base
template <bool can_copy, bool can_move>
struct sfinae_assign_base;

template <>
struct sfinae_assign_base<true, true> {
    sfinae_assign_base() = default;
    sfinae_assign_base(sfinae_assign_base const&) = default;
    sfinae_assign_base(sfinae_assign_base&&) = default;
    sfinae_assign_base& operator=(sfinae_assign_base const&) = default;
    sfinae_assign_base& operator=(sfinae_assign_base&&) = default;
};
template <>
struct sfinae_assign_base<false, true> {
    sfinae_assign_base() = default;
    sfinae_assign_base(sfinae_assign_base const&) = default;
    sfinae_assign_base(sfinae_assign_base&&) = default;
    sfinae_assign_base& operator=(sfinae_assign_base const&) = delete;
    sfinae_assign_base& operator=(sfinae_assign_base&&) = default;
};
template <>
struct sfinae_assign_base<true, false> {
    sfinae_assign_base() = default;
    sfinae_assign_base(sfinae_assign_base const&) = default;
    sfinae_assign_base(sfinae_assign_base&&) = default;
    sfinae_assign_base& operator=(sfinae_assign_base const&) = default;
    sfinae_assign_base& operator=(sfinae_assign_base&&) = delete;
};
template <>
struct sfinae_assign_base<false, false> {
    sfinae_assign_base() = default;
    sfinae_assign_base(sfinae_assign_base const&) = default;
    sfinae_assign_base(sfinae_assign_base&&) = default;
    sfinae_assign_base& operator=(sfinae_assign_base const&) = delete;
    sfinae_assign_base& operator=(sfinae_assign_base&&) = delete;
};

template <typename T, bool = is_integral<T>::value || is_enum<T>::value>
struct make_unsigned {typedef T type;};

template <> struct make_unsigned<signed int, true> {typedef unsigned int type;};
template <> struct make_unsigned<unsigned int, true> {typedef unsigned int type;};
template <> struct make_unsigned<signed short, true> {typedef unsigned short type;};
template <> struct make_unsigned<unsigned short, true> {typedef unsigned short type;};
template <> struct make_unsigned<signed long, true> {typedef unsigned long type;};
template <> struct make_unsigned<unsigned long, true> {typedef unsigned long type;};
template <> struct make_unsigned<signed long long, true> {typedef unsigned long long type;};
template <> struct make_unsigned<unsigned long long, true> {typedef unsigned long long type;};


//decay
template <typename T, bool>
struct _decay {
    typedef typename remove_cv<T>::type type;
};

template <typename T>
struct _decay<T, true> {
public:
    typedef typename conditional<
        is_array<T>::value,
        typename remove_extent<T>::type*,
        typename conditional<
            is_function<T>::value,
            typename add_pointer<T>::type,
            typename remove_cv<T>::type
        >::type
    >::type type;
};

template <typename T>
struct decay {
private:
    typedef typename remove_reference<T>::type U;
public:
    typedef typename _decay<U, is_referenceable<U>::value>::type type;
};

//is_constant_evaluated
inline constexpr bool libcpp_is_constant_evaluated() noexcept {
    return __builtin_is_constant_evaluated();
}

struct __any {
    __any(...);
};

//// ------- INVOKE !!! ----------------

//// TODO: invokable is much different than stl
// if Ret is void, the return type is ignored.
// otherwise return type has to be the same.
template <typename Ret, typename F, typename... Args>
struct invokable_r {
    struct nat;
    
    template <typename X, typename... XArgs>
    static auto try_call(int) -> 
        decltype(forward<X>(declval<X>())(forward<XArgs>(declval<XArgs>())...));

    template <typename X, typename... XArgs>
    static nat try_call(...);
    
    using Result = decltype(try_call<F, Args...>(0));

    using type = typename conditional<
        is_same<Result, nat>::value,
        false_type,
        typename conditional<
            is_void_v<Ret>,
            true_type,
            is_convertible<Ret, Result>
            >::type
        >::type;
    static const bool value = type::value;
};

template <typename F, typename... Args>
using invokable = invokable_r<void, F, Args...>;

template <typename F, typename... Args>
struct invoke_result: public enable_if<
    invokable<F, Args...>::value,
    typename invokable<F, Args...>::Result
> {};

template <typename F, typename... Args>
using invoke_result_t = typename invoke_result<F, Args...>::type;

/// About how to invoke a member pointer
template <typename>
constexpr bool is_reference_wrapper_v = false;
template <typename T>
constexpr bool is_reference_wrapper_v<std::reference_wrapper<T>> = true;
template <typename C, typename M, typename T, typename... Args>
constexpr decltype(auto) invoke_memptr(M C::* method, T&& instance, Args... args) {
    if constexpr (is_function<M>::value) {
        if constexpr (std::is_base_of_v<C, typename decay<T>::type>) {
            return (forward<T>(instance).*method)
                (forward<Args>(args)...);
        }
        else if constexpr (is_reference_wrapper_v<typename decay<T>::type>) {
            return (instance.get().*method)(forward<Args>(args)...);
        }
        else {
            return ((*forward<T>(instance)).*method)(forward<Args>(args)...);
        }
    }
}

// TODO: noexcept
template <typename F, typename... Args>
constexpr invoke_result_t<F, Args...>
invoke(F&& f, Args&&... args) 
{
    if constexpr (is_member_pointer<F>::value) {
        return invoke_memptr(f, forward<Args>(args)...);
    } else {
        return forward<F>(f)(forward<Args>(args)...);
    }
}

// If trait, receive three parameters:
// P: if true, returns type T, else return type U,
// T,
// U
template <bool, typename, typename>
struct IfImpl;

template <typename T, typename U>
struct IfImpl<true, T, U> {
    typedef T type;
};

template <typename T, typename U>
struct IfImpl<false, T, U> {
    typedef U type;
};

template <bool p, typename T, typename U>
using If = typename IfImpl<p, T, U>::type;

} //namespace evo


#endif /* _TYPE_TRAITS_H*/

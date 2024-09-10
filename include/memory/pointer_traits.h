/********************************************
 > File Name       : memory/pointer_traits.h
 > Author          : lunar
 > Email           : lunar_ubuntu@qq.com
 > Created Time    : Sat May 27 00:09:42 2023
 > Copyright@ https://github.com/xiaoqixian
********************************************/

#ifndef POINTER_TRAITS_H
#define POINTER_TRAITS_H

#include "type_traits.h"
#include "types.h"

namespace evo {

template <typename T, typename = void>
struct has_element_type: evo::false_type {};

template <typename T>
struct has_element_type<T, 
    typename evo::void_t<typename T::element_type>::type>: evo::true_type {};

template <typename Ptr, bool = has_element_type<Ptr>::value>
struct pointer_traits_element_type;

template <typename Ptr>
struct pointer_traits_element_type<Ptr, true> {
    typedef typename Ptr::element_type type;
};

template <template <typename, typename...> typename S, typename T, typename... Args>
struct pointer_traits_element_type<S<T, Args...>, true> {
    typedef typename S<T, Args...>::element_type type;
};

template <template <typename, typename...> typename S, typename T, typename... Args>
struct pointer_traits_element_type<S<T, Args...>, false> {
    typedef T type;
};

template <typename T, typename = void>
struct has_difference_type: evo::false_type {};

template <typename T>
struct has_difference_type<T, 
    typename evo::void_t<typename T::difference_type>::type>: evo::true_type {};

template <typename Ptr, bool = has_difference_type<Ptr>::value>
struct pointer_traits_difference_type {
    typedef evo::ptrdiff_t type;
};

template <typename Ptr>
struct pointer_traits_difference_type<Ptr, true> {
    typedef typename Ptr::difference_type type;
};

//if a T type pointer can be rebind to a U type pointer
template <typename T, typename U>
struct has_rebind {
private:
    struct _two {char a, b;};
    static_assert(sizeof(_two) == 2);
    template <typename> static _two test(...);
    template <typename X> static char test(typename X::template rebind<U>* = 0);
public:
    static const bool value = sizeof(test<T>(0) == 1);
};

template <typename T, typename U, bool = has_rebind<T, U>::value>
struct pointer_traits_rebind {
    typedef typename T::template rebind<U> type;
};

template <template <typename, typename...> typename S, typename T, typename... Args, typename U>
struct pointer_traits_rebind<S<T, Args...>, U, true> {
    typedef typename S<T, Args...>::template rebind<U> type;
};

template <template <typename, typename...> typename S, typename T, typename... Args, typename U>
struct pointer_traits_rebind<S<T, Args...>, U, false> {
    typedef S<U, Args...> type;
};

template <typename Ptr>
struct pointer_traits {
    typedef Ptr pointer;
    typedef typename pointer_traits_element_type<pointer>::type element_type;
    typedef typename pointer_traits_difference_type<pointer>::type difference_type;
    
    template <typename U>
    using rebind = typename pointer_traits_rebind<pointer, U>::type;

private:
    struct nat {};
public:
    static pointer pointer_to(typename evo::conditional<evo::is_void<element_type>::value, nat, element_type>::type& r) {
        return pointer::pointer_to(r);
    }
};

template <typename T>
struct pointer_traits<T*> {
    typedef T* pointer;
    typedef T element_type;
    typedef evo::ptrdiff_t difference_type;

    template <typename U> using rebind = U*;
private:
    struct nat {};
public:
    static pointer pointer_to(typename evo::conditional<evo::is_void<element_type>::value, nat, element_type>::type& r) noexcept {
        return evo::address_of(r);
    }
};

template <typename From, typename To>
struct rebind_pointer {
    typedef typename pointer_traits<From>::template rebind<To> type;
};

//to_address
template <typename Pointer, typename = void>
struct to_address_helper;

template <typename T>
inline constexpr T* to_address(T* p) noexcept {
    static_assert(!evo::is_function<T>::value, "T is a function type");
    return p;
}

//enable_if is needed here to avoid instantiating checks for fancy pointers on raw pointers
template <typename Pointer, typename = evo::enable_if<!evo::is_pointer<Pointer>::value>> 
inline constexpr typename evo::decay<decltype(to_address_helper<Pointer>::call(declval<const Pointer&>()))>::type to_address(const Pointer& p) noexcept {
    return to_address_helper<Pointer>::call(p);
}

template <typename Pointer, typename>
struct to_address_helper {
    static decltype(evo::to_address(declval<const Pointer&>().operator->())) call(const Pointer& p) noexcept {
        return evo::to_address(p.operator->());
    }
};

template <typename Pointer>
struct to_address_helper<Pointer, decltype((void)pointer_traits<Pointer>::to_address(declval<const Pointer&>()))> {
    static decltype(pointer_traits<Pointer>::to_address(declval<const Pointer&>())) call(const Pointer& p) noexcept {
        return pointer_traits<Pointer>::to_address(p);
    }
};

}

#endif

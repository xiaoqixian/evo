/********************************************
 > File Name       : memory/allocator_traits.h
 > Author          : lunar
 > Email           : lunar_ubuntu@qq.com
 > Created Time    : Fri May 26 00:30:17 2023
 > Copyright@ https://github.com/xiaoqixian
********************************************/

#ifndef ALLOCATOR_TRAITS_H
#define ALLOCATOR_TRAITS_H

#include "evo/type_traits.h"
#include "evo/memory/pointer_traits.h"

/*
The allocator_traits class template provides the standardized way to access various properties of Allocators. The standard containers and other standard library components access allocators through this template, which makes it possible to use any class type as an allocator, as long as the user-provided specialization of std::allocator_traits implements all required functionality.
*/

namespace evo {

// Macro which generates a template type NAME to detect 
// if a type NAME has a member type named PROPERTY.
#define TRAITS_HAS_XXX(NAME, PROPERTY)\
    template <typename, typename = void> struct NAME: evo::false_type {};\
    template <typename T> struct NAME<T, typename evo::void_t<typename T::PROPERTY>::type>: evo::true_type {};

#define HAS_XXX_TYPE(NAME, PROPERTY)\
    template <typename T>\
    concept NAME = requires {\
        typename T::PROPERTY;\
    };

//pointer
// HAS_XXX_TYPE(has_pointer, pointer);
//
// template <typename T, typename Alloc, 
//          typename RawAlloc = evo::remove_reference_t<Alloc>>
// struct pointer {
//     typedef T* type;
// };
//
// template <typename T, typename Alloc, has_pointer RawAlloc>
// struct pointer<T, Alloc, RawAlloc> {
//     typedef typename RawAlloc::pointer type;
// };

TRAITS_HAS_XXX(has_pointer, pointer);
template <typename, typename Alloc,
         typename RawAlloc = typename evo::remove_reference<Alloc>::type,
         bool = has_pointer<RawAlloc>::value>
struct pointer {typedef typename RawAlloc::pointer type;};

template<typename T, typename Alloc, typename RawAlloc>
struct pointer<T, Alloc, RawAlloc, false> {typedef T* type;};

//const pointer
TRAITS_HAS_XXX(has_const_pointer, const_pointer)
template <typename, typename Ptr, typename Alloc,
         bool = has_const_pointer<Alloc>::value>
struct const_pointer {typedef typename Alloc::const_pointer type;};

template<typename T, typename Ptr, typename Alloc>
struct const_pointer<T, Ptr, Alloc, false> {
    typedef typename evo::pointer_traits<Ptr>::template rebind<const T> type;};

//void pointer
TRAITS_HAS_XXX(has_void_pointer, void_pointer)
template <typename Ptr, typename Alloc, bool = has_void_pointer<Alloc>::value>
struct void_pointer {
    typedef typename Alloc::void_pointer type;
};

template <typename Ptr, typename Alloc>
struct void_pointer<Ptr, Alloc, false> {
    typedef typename pointer_traits<Ptr>::template rebind<void> type;
};

//const void pointer
TRAITS_HAS_XXX(has_const_void_pointer, const_void_pointer)
template <typename Ptr, typename Alloc, bool = has_const_void_pointer<Alloc>::value>
struct const_void_pointer {
    typedef typename Alloc::const_void_pointer type;
};

template <typename Ptr, typename Alloc>
struct const_void_pointer<Ptr, Alloc, false> {
    typedef typename pointer_traits<Ptr>::template rebind<const void> type;
};

//size_type
TRAITS_HAS_XXX(has_size_type, size_type)
template <typename Alloc, typename DiffType, bool = has_size_type<Alloc>::value>
struct size_type: evo::make_unsigned<DiffType> {};
template <typename Alloc, typename DiffType>
struct size_type<Alloc, DiffType, true> {
    typedef typename Alloc::size_type type;
};

//alloc_traits_difference_type
TRAITS_HAS_XXX(has_alloc_traits_diff_type, difference_type)
template <typename Alloc, typename Ptr, bool = has_alloc_traits_diff_type<Alloc>::value>
struct alloc_traits_difference_type {
    typedef typename pointer_traits<Ptr>::difference_type type;
};

template <typename Alloc, typename Ptr>
struct alloc_traits_difference_type<Alloc, Ptr, true> {
    typedef Alloc::difference_type type;
};

// TODO propagate_on_container_copy_assignment
// TODO propagate_on_container_move_assignment
// TODO propagate_on_container_swap

// allocator_traits rebind
// if a type T has a template type called `rebind` 
// and `rebind` has an inner type called `other`.
template <typename T, typename U, typename = void>
struct has_rebind_other: evo::false_type {};

template <typename T, typename U>
struct has_rebind_other<T, U, typename evo::void_t<
    typename T::template rebind<U>::other>::type>: evo::true_type {};

template <typename T, typename U, bool = has_rebind_other<T, U>::value>
struct allocator_traits_rebind {
    static_assert(has_rebind_other<T, U>::value, "This allocator has to implement rebind");

    typedef typename T::template rebind<U>::other type;
};

template <template <typename, typename...> typename Alloc, typename T, typename... Args, typename U>
struct allocator_traits_rebind<Alloc<T, Args...>, U, true> {
    typedef typename Alloc<T, Args...>::template rebind<U>::other type;
};

template <template <typename, typename...> typename Alloc, typename T, typename... Args, typename U>
struct allocator_traits_rebind<Alloc<T, Args...>, U, false> {
    typedef Alloc<U, Args...> type;
};

//has_allocate_hint
template <typename Alloc, typename SizeType, typename ConstVoidPtr, typename = void>
struct has_allocate_hint: evo::false_type {};

template <typename Alloc, typename SizeType, typename ConstVoidPtr>
struct has_allocate_hint<Alloc, SizeType, ConstVoidPtr, decltype(
    (void)declval<Alloc>().allocate(declval<SizeType>(), declval<ConstVoidPtr>()))>: evo::true_type {};

//has_construct
template <typename, typename, typename...>
struct has_construct_impl: evo::false_type {};

template <typename Alloc, typename... Args>
struct has_construct_impl<decltype(
    (void)declval<Alloc>().construct(declval<Args>()...)
), Alloc, Args...>: evo::true_type {};

template <typename Alloc, typename... Args>
struct has_construct: has_construct_impl<Alloc, Args...> {};

//has_destroy
//notice that has_destroy is not quite same as has_construct
//has_construct construct with asgs.
//while has_destroy destroy with a pointer
template <typename Alloc, typename Pointer, typename = void>
struct has_destroy: evo::false_type {};

template <typename Alloc, typename Pointer>
struct has_destroy<Alloc, Pointer, decltype(
    (void)declval<Alloc>().destroy(declval<Pointer>())
)>: evo::true_type {};

//has_max_size
template <typename Alloc, typename = void>
struct has_max_size: evo::true_type {};

template <typename Alloc>
struct has_max_size<Alloc, decltype(
    (void)declval<Alloc>().max_size()
)>: evo::true_type {};

//has_select_on_container_copy_construction
template <typename Alloc, typename = void>
struct has_select_on_container_copy_construction: evo:: false_type {};

template <typename Alloc>
struct has_select_on_container_copy_construction<Alloc, decltype(
    (void)declval<Alloc>().select_on_container_copy_construction()
)>: evo::true_type {};


//allocator_traits
template <typename Alloc>
struct allocator_traits {
    typedef Alloc alloc_type;
    typedef typename alloc_type::value_type value_type;
    typedef typename pointer<value_type, alloc_type>::type pointer;
    typedef typename void_pointer<pointer, alloc_type>::type void_pointer;
    typedef typename const_void_pointer<pointer, alloc_type>::type const_void_pointer;
    typedef typename const_pointer<value_type, pointer, alloc_type>::type const_pointer;
    typedef typename alloc_traits_difference_type<alloc_type, pointer>::type difference_type;
    typedef typename size_type<alloc_type, difference_type>::type size_type;


    template <typename T>
    using rebind_alloc = typename allocator_traits_rebind<alloc_type, T>::type;
    template <typename T>
    using rebind_traits = allocator_traits<rebind_alloc<T>>;

    inline constexpr static pointer allocate(alloc_type& a, size_type n) {
        return a.allocate(n);
    }

    template <typename A = Alloc, typename = evo::enable_if<
        has_allocate_hint<A, size_type, const_void_pointer>::value>>
    inline constexpr static pointer allocate(alloc_type& a, size_type n, const_void_pointer hint) {
        return a.allocate(n, hint);
    }

    template <typename A = Alloc, typename = void, typename = evo::enable_if<
        !has_allocate_hint<A, size_type, const_void_pointer>::value>>
    inline constexpr static pointer allocate(alloc_type& a, size_type n, const_void_pointer) {
        return a.allocate(n);
    }

    inline constexpr static void deallocate(alloc_type& a, pointer p, size_type n) noexcept {
        return a.deallocate(p, n);
    }

    // allocator construct at ptr.
    template <typename T, typename... Args, typename = evo::enable_if<
        has_construct<alloc_type, T*, Args...>::value>>
    inline constexpr static void construct(alloc_type& a, T* p, Args&&... args) {
        a.construct(p, evo::forward<Args>(args)...);
    }

    // allocator with no `construct` method uses `new` at ptr.
    template <typename T, typename... Args, typename = void, typename = evo::enable_if<!has_construct<alloc_type, T*, Args...>::value>>
    inline constexpr static void construct(alloc_type&, T* p, Args&&... args) {
        ::new ((void*)p) T(evo::forward<Args>(args)...);
    }

    //destroy
    template <typename T, typename = evo::enable_if<
        has_destroy<alloc_type, T*>::value>>
    inline constexpr static void destroy(alloc_type& a, T* p) {
        a.destroy(p);
    }

    template <typename T, typename = void, typename = evo::enable_if<
        !has_destroy<alloc_type, T*>::value>>
    inline constexpr static void destroy(alloc_type&, T* p) {
        p->~T();
    }

    //max_size
    template <typename A = Alloc, typename = evo::enable_if<
        has_max_size<const A>::value>>
    inline constexpr static size_type max_size(const alloc_type& a) noexcept {
        return a.max_size();
    }

    template <typename A = Alloc, typename = void, typename = evo::enable_if<
        !has_max_size<const A>::value>>
    inline constexpr static size_type max_size(const alloc_type& a) noexcept {
        //TODO return numric limit
        return -1;
    } 

    template <typename A = Alloc, typename = evo::enable_if<
        has_select_on_container_copy_construction<const A>::value>>
    inline constexpr static alloc_type select_on_container_copy_construction(const alloc_type& a) {
        return a.select_on_container_copy_construction();
    }
    template <typename A = Alloc, typename = void, typename = evo::enable_if<
        !has_select_on_container_copy_construction<const A>::value>>
    inline constexpr static alloc_type select_on_container_copy_construction(const alloc_type& a) {
        return a;
    }
};

template <typename Traits, typename T>
struct rebind_alloc_helper {
    typedef typename Traits::template rebind_alloc<T> type;
};

//is_default_allocator
template <typename T>
struct is_default_allocator: evo::false_type {};

template <typename> class allocator;

template <typename T>
struct is_default_allocator<allocator<T>>: evo::true_type {};

//is_cpp17_move_insertable
template <typename Alloc, typename = void>
struct is_cpp17_move_insertable: evo::is_move_constructible<typename Alloc::value_type> {};

template <typename Alloc>
struct is_cpp17_move_insertable<Alloc, evo::enable_if<
    !is_default_allocator<Alloc>::value &&
    has_construct<Alloc, typename Alloc::value_type*, typename Alloc::value_type&&>::value>>: evo::true_type {};

//is_cpp17_copy_insertable
template <typename Alloc, typename = void>
struct is_cpp17_copy_insertable: 
    evo::bool_constant<
        evo::is_copy_constructible<typename Alloc::value_type>::value &&
        is_cpp17_move_insertable<Alloc>::value
    > {};

template <typename Alloc>
struct is_cpp17_copy_insertable<Alloc, evo::enable_if<
    !is_default_allocator<Alloc>::value &&
    has_construct<Alloc, typename Alloc::value_type*, const typename Alloc::value_type&>::value>>: is_cpp17_move_insertable<Alloc> {};

}

#endif

/********************************************
 > File Name       : allocator.h
 > Author          : lunar
 > Email           : lunar_ubuntu@qq.com
 > Created Time    : Thu May 25 16:50:12 2023
 > Copyright@ https://github.com/xiaoqixian
********************************************/

#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include "type_traits.h"
#include "allocator_traits.h"
#include "exception.h"
#include <new>
#include <type_traits>

// A default allocator type used by all evo containers 
// if no user-defined allocator is provided.

namespace evo {

template <typename>
class allocator;

// This class provides a non-trivial default constructor
// to the class that derives from it, if the condition is 
// satisfied.
template <bool Cond, typename Unique>
struct non_trivial_if {};

template <typename Unique>
struct non_trivial_if<true, Unique> {
    inline constexpr non_trivial_if() noexcept {}
};

template <typename T>
class allocator: private non_trivial_if<!evo::is_void_v<T>, allocator<T>> {
    static_assert(!std::is_volatile_v<T>, "evo::allocator does not support volatile types");

public:
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef T           value_type;
    typedef true_type   propagate_on_container_move_assignment;
    typedef true_type   is_always_equal;

    constexpr allocator() noexcept = default;

    template <typename U>
    inline constexpr allocator(allocator<U> const&) noexcept {}

    inline constexpr T* allocate(size_t n) {
        if (n > evo::allocator_traits<allocator>::max_size(*this)) 
            throw std::bad_array_new_length();

        // TODO: simplified
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, size_t n) noexcept {
        ::operator delete(p);
    }
};

template <typename T>
class allocator<const T>: private non_trivial_if<!evo::is_void_v<T>, allocator<const T>> {
    static_assert(!std::is_volatile_v<T>, "evo::allocator does not support volatile types");

public:
    typedef size_t      size_type;
    typedef ptrdiff_t   difference_type;
    typedef const T     value_type;
    typedef true_type   propagate_on_container_move_assignment;
    typedef true_type   is_always_equal;

    constexpr allocator() noexcept = default;

    template <typename U>
    inline constexpr allocator(allocator<U> const&) noexcept {}

    inline constexpr const T* allocate(size_t n) {
        if (n > evo::allocator_traits<allocator>::max_size(*this)) 
            throw std::bad_array_new_length();

        // TODO: simplified
        return static_cast<const T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(const T* p, size_t n) noexcept {
        ::operator delete(const_cast<T*>(p));
    }
};

template <typename T, typename U>
inline constexpr bool operator==(allocator<T> const&, allocator<U> const&) noexcept { return true; }

} // namespace evo

#endif 

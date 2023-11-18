/********************************************
 > File Name       : allocator.h
 > Author          : lunar
 > Email           : lunar_ubuntu@qq.com
 > Created Time    : Thu May 25 16:50:12 2023
 > Copyright@ https://github.com/xiaoqixian
********************************************/

#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include "../type_traits.h"
#include "allocator_traits.h"
#include "../exception.h"

namespace evo {

template <typename> class allocator;

template <>
class allocator<void> {
public:
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;

    template <typename U>
    struct rebind {
        typedef allocator<U> other;
    };
};

template <>
class allocator<const void> {
public:
    typedef const void* pointer;
    typedef const void* const_pointer;
    typedef const void value_type;

    template <typename U>
    struct rebind {
        typedef allocator<U> other;
    };
};

// This class provides a non-trivial default constructor to the class that derives from it
// if the condition is satisfied.
//
// The second template parameter exists to allow giving a unique type to __non_trivial_if,
// which makes it possible to avoid breaking the ABI when making this a base class of an
// existing class. Without that, imagine we have classes D1 and D2, both of which used to
// have no base classes, but which now derive from __non_trivial_if. The layout of a class
// that inherits from both D1 and D2 will change because the two __non_trivial_if base
// classes are not allowed to share the same address.
//
// By making those __non_trivial_if base classes unique, we work around this problem and
// it is safe to start deriving from __non_trivial_if in existing classes.
template <bool Cond, typename Unique>
struct non_trivial_if {};

template <typename Unique>
struct non_trivial_if<true, Unique> {
    inline constexpr non_trivial_if() noexcept {}
};

template <typename T>
class allocator:
    private non_trivial_if<!evo::is_void<T>::value, allocator<T>> {
public:
    typedef evo::size_t size_type;
    typedef evo::ptrdiff_t difference_type;
    typedef T value_type;
    typedef true_type propagete_on_container_move_assignment;
    typedef true_type is_always_equal;

    inline constexpr allocator() noexcept = default;

    template <typename U>
    inline constexpr allocator(const allocator<U>&) noexcept {}

    T* allocate(evo::size_t n) {
        if (n > allocator_traits<allocator>::max_size(*this))
            throw evo::length_error("allocator<T>::allocate(size_t n)"
                    "'n' exceeds maximum supported size");
        if (evo::libcpp_is_constant_evaluated())
            return static_cast<T*>(::operator new(n * sizeof(T)));//operator new just allocate memory.
        //TODO allocate aligned
        /* else return static_cast<T*>(__builtin_operator_new(n * sizeof(T), alignof(T))); */
    }
};
}

#endif 

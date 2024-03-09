// Date:   Tue Feb 27 14:42:19 2024
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _MEMORY_H
#define _MEMORY_H

#include <type_traits>
#include "type_traits.h"
#include "concepts.h"
#include "debug.h"
#include <atomic>

namespace evo {
#if __cplusplus >= 202002L

template <typename T>
class allocator;

namespace pointer_type_impl {

// check if type T::pointer exists.
template <typename T, typename = void>
struct has_pointer_type: evo::false_type {};

template <typename T>
struct has_pointer_type<T, typename evo::void_t<typename T::pointer>::type>: evo::true_type {};

template <typename T>
constexpr bool has_pointer_type_v = has_pointer_type<T>::value;

} // end of namespace pointer_type_impl.

// return D::pointer if exists, 
// otherwise return T*.
template <typename T, typename D, 
         bool = pointer_type_impl::has_pointer_type_v<D>>
struct pointer_type {
    typedef typename D::pointer type;
};

template <typename T, typename D>
struct pointer_type<T, D, false> {
    typedef T* type;
};

template <typename T>
requires evo::is_array_v<T>
struct array_known_bound: evo::false_type {};

template <typename T, size_t N>
requires evo::is_array_v<T>
struct array_known_bound<T[N]>: evo::true_type {};

template <typename T>
constexpr bool array_known_bound_v = array_known_bound<T>::value;

template <typename D, typename T>
concept Deleter = requires(evo::remove_extent_t<T>* ptr, D deleter) {
    deleter(ptr);
};


template <typename T>
struct default_delete {
    static_assert(!std::is_function_v<T>, 
        "default_delete cannot be instantiated for function types");

    constexpr default_delete() noexcept = default;

    template <typename U>
    requires evo::is_convertible_v<U*, T*>
    default_delete(default_delete<U> const&) noexcept {}

    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0, 
                "default_delete cannot delete incomplete type");
        static_assert(!evo::is_void_v<T>, 
                "default_delete cannot delete incomplete type");
        delete ptr;
    }
};

template <typename T>
struct default_delete<T[]> {
    constexpr default_delete() noexcept = default;

    template <typename U>
    requires evo::is_convertible_v<U(*)[], T(*)[]>
    default_delete(default_delete<U> const&) noexcept {}

    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0, 
                "default_delete cannot delete incomplete type");
        static_assert(!evo::is_void_v<T>, 
                "default_delete cannot delete incomplete type");
        
        DEBUG("calling delete[]")
        delete[] ptr;
    }
};

template <typename T, Deleter<T> D = default_delete<T>>
class unique_ptr {
public:
    typedef T element_type;
    typedef D deleter_type;
    typedef typename pointer_type<element_type, deleter_type>::type pointer;

private:
    pointer ptr;
    deleter_type dter;

public:
    constexpr unique_ptr() noexcept 
        requires(
            evo::default_constructible<pointer> &&
            evo::default_constructible<deleter_type>
        )
        : ptr(pointer()), dter(deleter_type())
    {}

    constexpr unique_ptr(std::nullptr_t) noexcept
        requires evo::default_constructible<deleter_type>
        : ptr(nullptr), dter(deleter_type()) {}

    explicit unique_ptr(pointer ptr) noexcept
        requires evo::default_constructible<deleter_type>
        : ptr(ptr), dter(deleter_type()) {}

    unique_ptr(unique_ptr const&) = delete;

    unique_ptr(unique_ptr&& other) noexcept 
        requires evo::move_constructible<deleter_type>
        : ptr(other.ptr), 
            dter(evo::forward<deleter_type>(other.get_deleter())) 
    {}

    // TODO
    // template <typename U, typename E>
    // unique_ptr(unique_ptr<U, E>&&) noexcept;

    // TODO
    // template <typename U>
    // unique_ptr(std::auto_ptr<U>&&) noexcept;

    // check whether *this owns an object.
    explicit operator bool() const noexcept {
        return this->ptr != nullptr;
    }

    unique_ptr& operator=( unique_ptr&& other ) noexcept 
        requires evo::move_assignable<deleter_type>
    {
        if (*this) {
            this->get_deleter()(this->get());
        }
        this->ptr = other.get();
        this->dter = forward<deleter_type>(other.get_deleter());
        
        return *this;
    }
    unique_ptr& operator=( std::nullptr_t ) noexcept {
        if (*this) {
            this->get_deleter(this->get());
        }
        this->ptr = nullptr;
        
        return *this;
    }
    unique_ptr& operator=( unique_ptr const& ) = delete;

    template <typename U, Deleter<U> E>
        requires (evo::assignable<deleter_type&, E&&> &&
                !evo::is_array_v<U>)
    unique_ptr& operator=( unique_ptr<U, E>&& other ) noexcept {
        if (*this)
            this->get_deleter()(this->get());

        this->ptr = other.get();
        this->dter = forward<E>(other.get_deleter());
        
        return *this;
    }

    // returns a pointer to the managed object.
    pointer get() const noexcept {
        return this->ptr;
    }

    // returns the deleter object
    deleter_type& get_deleter() noexcept {
        return this->dter;
    }
    deleter_type const& get_deleter() const noexcept {
        return this->dter;
    }

    // releases the ownership of the managed object, if any.
    pointer release() noexcept {
        pointer temp = this->ptr;
        this->ptr = nullptr;
        return temp;
    }

    // replaces the managed object.
    void reset(pointer ptr = pointer()) noexcept {
        if (*this) {
            this->get_deleter()(this->get());
        }
        this->ptr = ptr;
    }

    // swap the managed objects and associated deleters of *this and another.
    void swap(unique_ptr& other) noexcept {
        std::swap(this->ptr, other.ptr);
        std::swap(this->dter, other.get_deleter());
    }

    typename std::add_lvalue_reference_t<element_type> 
    operator*() const noexcept(noexcept(*std::declval<pointer>())) {
        return *(this->ptr);
    }

    pointer operator->() const noexcept {
        return this->ptr;
    }

    ~unique_ptr() noexcept {
        if (*this) 
            this->get_deleter()(this->get());
    }
};

template <typename T, Deleter<T> D>
class unique_ptr<T[], D> {
public:
    typedef T element_type;
    typedef D deleter_type;
    typedef typename pointer_type<element_type, deleter_type>::type pointer;

private:
    pointer ptr;
    deleter_type dter;

public:
    constexpr unique_ptr() noexcept 
        requires evo::default_constructible<deleter_type>
        : ptr(pointer()), dter(deleter_type())
    {}

    constexpr unique_ptr(std::nullptr_t) noexcept
        requires evo::default_constructible<deleter_type>
        : ptr(nullptr), dter(deleter_type()) {}

    explicit unique_ptr(pointer ptr) noexcept
        requires evo::default_constructible<deleter_type>
        : ptr(ptr), dter(deleter_type()) {}

    unique_ptr(unique_ptr const&) = delete;

    unique_ptr(unique_ptr&& other) noexcept 
        requires evo::move_constructible<deleter_type>
        : ptr(other.ptr), 
            dter(evo::forward<deleter_type>(other.get_deleter())) 
    {}

    // TODO
    // template <typename U, typename E>
    // unique_ptr(unique_ptr<U, E>&&) noexcept;

    // TODO
    // template <typename U>
    // unique_ptr(std::auto_ptr<U>&&) noexcept;

    // check whether *this owns an object.
    explicit operator bool() const noexcept {
        return this->ptr != nullptr;
    }

    unique_ptr& operator=( unique_ptr&& other ) noexcept 
        requires evo::move_assignable<deleter_type>
    {
        if (*this) {
            this->get_deleter()(this->get());
        }
        this->ptr = other.get();
        this->dter = forward<deleter_type>(other.get_deleter());
        
        return *this;
    }
    unique_ptr& operator=( std::nullptr_t ) noexcept {
        if (*this) {
            this->get_deleter(this->get());
        }
        this->ptr = nullptr;
        
        return *this;
    }
    unique_ptr& operator=( unique_ptr const& ) = delete;

    constexpr inline
    element_type& operator[](size_t index) & {
        return (*(this->ptr))[index];
    }
    constexpr inline
    element_type const& operator[](size_t index) const& {
        return (*(this->ptr))[index];
    }

    template <typename U, Deleter<U> E>
        requires (evo::assignable<deleter_type&, E&&> &&
                !evo::is_array_v<U>)
    unique_ptr& operator=( unique_ptr<U, E>&& other ) noexcept {
        if (*this)
            this->get_deleter()(this->get());

        this->ptr = other.get();
        this->dter = forward<E>(other.get_deleter());
        
        return *this;
    }

    // returns a pointer to the managed object.
    pointer get() const noexcept {
        return this->ptr;
    }

    // returns the deleter object
    deleter_type& get_deleter() noexcept {
        return this->dter;
    }
    deleter_type const& get_deleter() const noexcept {
        return this->dter;
    }

    // releases the ownership of the managed object, if any.
    pointer release() noexcept {
        pointer temp = this->ptr;
        this->ptr = nullptr;
        return temp;
    }

    // replaces the managed object.
    void reset(pointer ptr = pointer()) noexcept {
        if (*this) {
            this->get_deleter()(this->get());
        }
        this->ptr = ptr;
    }

    // swap the managed objects and associated deleters of *this and another.
    void swap(unique_ptr& other) noexcept {
        std::swap(this->ptr, other.ptr);
        std::swap(this->dter, other.get_deleter());
    }

    typename std::add_lvalue_reference_t<element_type> 
    operator*() const noexcept(noexcept(*std::declval<pointer>())) {
        return *(this->ptr);
    }

    pointer operator->() const noexcept {
        return this->ptr;
    }

    ~unique_ptr() noexcept {
        if (*this) 
            this->get_deleter()(this->get());
    }

};

template <typename T, typename... Args>
requires (!evo::is_array_v<T> && evo::constructible<T, Args...>)
constexpr unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(evo::forward<Args>(args)...));
}

template <typename T, typename... Args>
requires (!array_known_bound_v<T>)
constexpr unique_ptr<T> make_unique( evo::size_t size ) {
    return unique_ptr<T>(new evo::remove_extent_t<T>[size]);
}

template <typename T, typename... Args>
requires array_known_bound_v<T>
auto make_unique( Args&&... args ) = delete;

template <typename A, typename T>
concept Allocator = requires(A alloc, size_t size) {
    { alloc.allocate(size) } -> evo::same_as<T*>;
};

// shared_ptr
template <typename T>
class weak_ptr;

class shared_count {
    std::atomic<long> count = 1;

public:
    constexpr shared_count() noexcept = default;
    constexpr shared_count(long ref) noexcept: count(ref) {}

    inline void add_shared() noexcept {
        std::atomic_fetch_add(&this->count, (long)1);
    }

    virtual void on_zero_shared() = 0;

    inline bool release_shared() noexcept {
        return std::atomic_fetch_sub(&this->count, (long)1) == 0;
    }

    inline long use_count() const noexcept {
        return std::atomic_load(&this->count);
    }
};

template <typename T, Deleter<T> D, Allocator<T> A>
class shared_ptr_pointer: public shared_count {
private:
    typedef evo::remove_extent_t<T> element_type;
    element_type* ptr;
    D deleter;
    A alloc;
public:
    constexpr shared_ptr_pointer() = default;
    constexpr shared_ptr_pointer( std::nullptr_t ): ptr(nullptr) {}

    shared_ptr_pointer(element_type* ptr)
        requires (evo::default_constructible<D> && 
                evo::default_constructible<A>)
        : ptr(ptr) {}

    shared_ptr_pointer(element_type* ptr, D dter, A alloc) 
        : ptr(ptr), deleter(dter), alloc(alloc) {}

    element_type* operator->() {
        return this->ptr;
    }

    inline virtual void on_zero_shared() override {
        this->deleter(this->ptr);
        this->deleter.~D();
    }
};

template <typename T>
class shared_ptr {
public:
    typedef evo::remove_extent_t<T> element_type;
    typedef weak_ptr<T> weak_type;
    
private:
    element_type* ptr;
    shared_count* ctrl;

public:
    constexpr shared_ptr() noexcept
        : ptr(nullptr), ctrl(nullptr) {}
    constexpr shared_ptr( std::nullptr_t ) noexcept
        : ptr(nullptr), ctrl(nullptr) {}

    template <typename Y>
    requires evo::convertible_to<Y*, T*>
    shared_ptr( Y* p ) noexcept 
        :ptr(p)
    {
        typedef shared_ptr_pointer<Y*, default_delete<Y>, allocator<Y>> CtrlBlock;
        this->ctrl = new CtrlBlock(p, default_delete<Y>(), allocator<Y>());
    }
};

template <typename Y>


#endif // end of __cplusplus >= 202002L

}

#endif // _MEMORY_H

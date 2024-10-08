// Date:   Sat Apr 13 15:41:46 2024
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _SHARED_PTR_HPP
#define _SHARED_PTR_HPP

#include <__compare/compare_three_way.h>
#include <atomic>
#include <compare>
#include <functional>
#include <memory>

#include "memory/allocator_traits.h"
#include "memory/compressed_pair.hpp"
#include "memory/pointer_traits.h"
#include "type_traits.h"
#include "memory/unique_ptr.hpp"

namespace evo {

template <typename T>
class weak_ptr;

class shared_count {
    std::atomic<long> count = 1;

public:
    constexpr shared_count() noexcept = default;
    constexpr shared_count(long ref) noexcept: count(ref) {}

    virtual ~shared_count();

    inline void add_shared() noexcept {
        // std::atomic_fetch_add(&this->count, (long)1);
        this->count.fetch_add(1, std::memory_order_acq_rel);
    }

    virtual void on_zero_shared() = 0;

    bool release_shared() noexcept {
        if (this->count.fetch_sub(1, std::memory_order_acq_rel) == 0) {
            this->on_zero_shared();
            return true;
        } else {
            return false;
        }
    }

    inline long use_count() const noexcept {
        return this->count.load(std::memory_order_acquire);
    }
};

class shared_weak_count: private shared_count {
    std::atomic<long> shared_weak_owners;

public:
    shared_weak_count(long refs = 0) noexcept:
        shared_count(refs),
        shared_weak_owners(refs) {}

    inline void add_shared() noexcept {
        shared_count::add_shared();
    }
    
    inline void add_weak() noexcept {
        this->shared_weak_owners.fetch_add(1, std::memory_order_acq_rel);
    }
    
    void release_shared() noexcept {
        if (shared_count::release_shared()) {
            this->release_weak();
        }
    }

    void release_weak() noexcept;

    inline long use_count() const noexcept {
        return shared_count::use_count();
    }

    shared_weak_count* lock() noexcept;

private:
    virtual void on_zero_shared_weak() noexcept = 0;
};

// T is expected to be a pointer type.
template <typename T, typename D, typename Alloc>
class shared_ptr_pointer: public shared_weak_count {
    evo::compressed_pair<evo::compressed_pair<T, D>, Alloc> data;

public:
    inline shared_ptr_pointer(T p, D d, Alloc a):
        data(evo::compressed_pair<T, D>(p, evo::move(d)), evo::move(a)) 
    {}

    // TODO: get_deleter

private:
    virtual void on_zero_shared() noexcept override {
        data.first().second()(data.first().first());
        // the deleter destroy itself.
        data.first().second().~D();
    }

    // TODO: stuff about allocator_traits.
    virtual void on_zero_shared_weak() noexcept override {
        typedef typename evo::allocator_traits_rebind<Alloc, shared_ptr_pointer>::type Alp;
        typedef evo::allocator_traits<Alp> ATraits;
        typedef evo::pointer_traits<typename ATraits::pointer> PTraits;

        Alp a(this->data.second());
        this->data.second().~Alloc();
        a.deallocate(PTraits::pointer_to(*this), 1);
    }
};

// This tag is used to instantiate an allocator type. The various shared_ptr control blocks
// detect that the allocator has been instantiated for this type and perform alternative
// initialization/destruction based on that.
struct for_overwrite_tag {};

template <typename T, typename Alloc>
struct shared_ptr_emplace: shared_weak_count {
    template <typename... Args>
    explicit shared_ptr_emplace(Alloc a, Args&&... args):
        storage(evo::move(a)) 
    {
        if constexpr (evo::is_same_v<typename Alloc::value_type
                , for_overwrite_tag>) 
        {
            static_assert(sizeof...(Args) == 0, "No argument should be provided to the control block when using for_overwrite_tag");
            ::new ((void*)get_elem()) T;
        } else {
            using TAlloc = typename evo::allocator_traits_rebind<Alloc, T>::type;
            TAlloc temp(*get_alloc());
            evo::allocator_traits<TAlloc>::construct(temp, get_elem(), evo::forward<Args>(args)...);
        }

        ::new ((void*)get_elem()) T(evo::forward<Args>(args)...);
    }

    Alloc* get_alloc() noexcept {
        return this->storage.get_alloc();
    }

    T* get_elem() noexcept {
        return this->storage.get_elem();
    }

private:
    virtual void on_zero_shared() noexcept override {
        if constexpr (evo::is_same_v<typename Alloc::value_type, for_overwrite_tag>) {
            this->get_elem()->~T();
        } else {
            using TAlloc = typename evo::allocator_traits_rebind<Alloc, T>::type;
            TAlloc temp(*get_alloc());
            evo::allocator_traits<TAlloc>::destroy(temp, get_elem());
        }
    }

    virtual void on_zero_shared_weak() noexcept override {
        using ControlBlockAlloc = typename evo::allocator_traits_rebind<
            Alloc, shared_ptr_emplace>::type;
        using ControlBlockPointer = typename evo::allocator_traits<
            ControlBlockAlloc>::pointer;

        ControlBlockAlloc temp(*this->get_alloc());
        this->storage.~Stroge();
        evo::allocator_traits<ControlBlockAlloc>::deallocate(
            temp, 
            evo::pointer_traits<ControlBlockPointer>::pointer_to(*this),
            1
        );
    }

    using CompressedPair = evo::compressed_pair<Alloc, T>;
    
    // A compound storage of type T and a Allocator.
    struct Storage {
        char blob[sizeof(CompressedPair)];

        explicit Storage(Alloc&& a) {
            ::new ((void*) get_alloc()) Alloc(evo::move(a));
        }

        ~Storage() {
            get_alloc()->~Alloc();
        }

        // extract the allocator from memory
        Alloc* get_alloc() noexcept {
            CompressedPair* as_pair = reinterpret_cast<CompressedPair*>(blob);
            typename CompressedPair::Base1* first = CompressedPair::get_first_base(as_pair);
            Alloc* alloc = reinterpret_cast<Alloc*>(first);
            return alloc;
        }

        T* get_elem() noexcept {
            CompressedPair* as_pair = reinterpret_cast<CompressedPair*>(blob);
            typename CompressedPair::Base2* second = CompressedPair::get_second_base(as_pair);
            
            return reinterpret_cast<T*>(second);
        }
    };

    static_assert(alignof(Storage) == alignof(CompressedPair), 
            "Alignof Storage and CompressedPair must be the same");
    static_assert(sizeof(Storage) == sizeof(CompressedPair), 
            "Sizeof Storage and CompressedPair must be the same");

    Storage storage;
};

/*
 * a pointer type Y* is said to be compatible with a pointer type T* 
 * if either Y* is convertible to T* or Y is the array type U[N] and 
 * T is U cv [] (where cv is some set of cv-qualifiers).
 */
template <class Y, class T>
struct bounded_convertible_to_unbounded : false_type {};

template <class U, std::size_t N, class T>
struct bounded_convertible_to_unbounded<U[N], T>
        : evo::is_same<evo::remove_cv_t<T>, U[]> {};

template <typename Y, typename T>
concept compatible_with = evo::is_convertible_v<Y*, T*> ||
    bounded_convertible_to_unbounded<Y, T>::value;

template <typename T>
using shared_ptr_default_allocator = std::allocator<T>;

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
    requires compatible_with<Y*, T*>
    shared_ptr( Y* p ) noexcept 
        :ptr(p)
    {
        typedef shared_ptr_default_allocator<Y> AllocType;
        typedef shared_ptr_pointer<Y, default_delete<T>, AllocType> CtrlBlock;
        this->ctrl = new CtrlBlock(p, default_delete<T>(), AllocType());

        // TODO: enable_weak_this
    }

    shared_ptr(shared_ptr const& other): ptr(other.ptr), ctrl(other.ctrl) {
        if (this->ctrl) 
            this->ctrl->add_shared();
    }

    template <typename Y>
    requires compatible_with<Y*, T*>
    shared_ptr(shared_ptr<Y> const& other): ptr(other.ptr), ctrl(other.ctrl) {
        if (this->ctrl) 
            this->ctrl->add_shared();
    }

    shared_ptr(shared_ptr&& other): ptr(other.ptr), ctrl(other.ctrl) {
        other.ptr = nullptr;
        other.ctrl = nullptr;
    }

    template <typename Y>
    requires compatible_with<Y, T>
    shared_ptr(shared_ptr<Y>&& other): ptr(other.ptr), ctrl(other.ctrl) {
        other.ptr = nullptr;
        other.ctrl = nullptr;
    }

    template <typename Y>
    requires compatible_with<Y, T>
    explicit shared_ptr(weak_ptr<Y> const& other_weak):
        ptr(other_weak.ptr),
        ctrl(other_weak.ctrl ? other_weak.ctrl->lock() : other_weak.ctrl)
    {
        // weak_ptr control block cannot be nullptr.
        if (this->ctrl == nullptr) 
            throw std::bad_weak_ptr();
    }

    // construct from a unique_ptr rvalue reference
    template <typename Y, typename D>
    requires (
        evo::is_lvalue_reference_v<D> &&
        compatible_with<Y, T> &&
        evo::is_constructible_v<typename evo::unique_ptr<Y, D>::pointer, element_type*>
    )
    shared_ptr(unique_ptr<Y>&& other_unique): ptr(other_unique.get()) {
        if (this->ptr == nullptr) 
            this->ctrl = nullptr;
        else {
            typedef typename shared_ptr_default_allocator<Y>::type AllocT;
            typedef shared_ptr_pointer<
                typename unique_ptr<Y, D>::pointer,
                std::reference_wrapper<evo::remove_reference_t<D>>,
                AllocT
            > CtrlBlock;

            this->ctrl = new CtrlBlock(
                other_unique.get(), 
                std::ref(other_unique.get_deleter()), 
                AllocT());
        }
        other_unique.release();
    }

    ~shared_ptr() {
        if (this->ctrl) 
            this->ctrl->release_shared();
    }

    shared_ptr<T>& operator=(shared_ptr const& other) noexcept {
        shared_ptr(other).swap(*this);
        return *this;
    }

    template <typename Y>
    requires compatible_with<Y, T>
    shared_ptr<T>& operator=(shared_ptr<Y> const& other) noexcept {
        shared_ptr(other).swap(*this);
        return *this;
    }

    shared_ptr<T>& operator=(shared_ptr && other) noexcept {
        shared_ptr(evo::move(other)).swap(*this);
        return *this;
    }

    // TODO: assign with unique_ptr.


    void swap(shared_ptr& other) noexcept {
        evo::swap(this->ptr, other.ptr);
        evo::swap(this->ctrl, other.ctrl);
    }

    void reset() noexcept {
        shared_ptr().swap(*this);
    }

    template <typename Y>
    requires compatible_with<Y, T>
    void reset(Y* ptr) noexcept {
        shared_ptr(ptr).swap(*this);
    }

    // TODO: reset with a ptr and a deleter.
    // TODO: reset with a ptr and a deleter and an allocator.

    element_type* get() const noexcept {
        return this->ptr;
    }

    evo::add_lvalue_reference_t<element_type> 
    operator*() const noexcept {
        return *this->ptr;
    }

    element_type* operator->() const noexcept {
        static_assert(!evo::is_array_v<T>, 
            "evo::shared_ptr<T>::operator-> is only valid when T is not an array type");
        return this->ptr;
    }

    long use_count() const noexcept {
        return this->ctrl ? this->ctrl->use_count() : 0;
    }

    bool unique() const noexcept {
        return this->use_count() == 1;
    }

    explicit operator bool() const noexcept {
        return this->get() != nullptr;
    }
};

template <typename T, typename... Args>
requires evo::concepts::constructible<T, Args...>
constexpr shared_ptr<T> make_shared( Args&&... args ) {
    return shared_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename U>
inline bool operator!=(shared_ptr<T> const& x, shared_ptr<U> const& y) noexcept {
    return !(x == y);
}

template <typename T, typename U>
std::strong_ordering operator<=>(shared_ptr<T> const& x, shared_ptr<U> const& y) noexcept {
    return std::compare_three_way()(x.get(), y.get());
}

template <typename T>
std::strong_ordering operator<=>(shared_ptr<T> const& x, nullptr_t) noexcept {
    return std::compare_three_way()(x.get(), static_cast<typename shared_ptr<T>::element_type*>(nullptr));
}



} // namespace evo

#endif // _SHARED_PTR_HPP

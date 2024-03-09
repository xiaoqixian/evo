// Date: Sat Nov 18 11:35:17 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _FUTURE_H
#define _FUTURE_H

#include "type_traits.h"
#include <type_traits>

namespace evo {

//future 

template <typename T>
class future {
public:
    typedef T value_type;
private:
    bool is_valid;
    value_type val;
public:
    future() noexcept
        requires(evo::is_default_constructible_v<value_type>)
        : is_valid(false) 
    {}

    template <typename U>
    future(future<U>&& other) noexcept
        requires(evo::is_constructible_v<value_type, U&&>)
        : val(evo::move(other.get())),
          is_valid(true)
    {
        other.is_valid = false;
    }

    future(future const&) = delete;

    template <typename U>
    future& operator=(future<U>&& other) noexcept
        requires(evo::is_constructible_v<value_type, U&&>)
    {
        this->val = evo::move(other.get());
        this->is_valid = true;
        other.is_valid = false;
        return *this;
    }

    future& operator=(future const&) = delete;

    inline constexpr bool valid() const noexcept {
        return this->is_valid;
    }

    T get() noexcept;

    void wait() noexcept;

};

//promise
template <typename R>
class promise {
public:
    future<R> get_future() noexcept; 
};

// packaged_task

template <typename>
class packaged_task;

template <typename F, typename... Args>
class packaged_task<F(Args...)> {
    typedef typename std::result_of<F(Args...)>::type return_type;
public:
    bool valid() const noexcept;

    future<return_type> get_future() const noexcept;
};

// launch
enum class launch {
    async = 1,
    defered = 2,
    any = async | defered
};

}

#endif // _FUTURE_H

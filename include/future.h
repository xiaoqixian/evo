// Date: Sat Nov 18 11:35:17 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _FUTURE_H
#define _FUTURE_H

#include "type_traits.h"

namespace evo {

//future 

template <typename T>
class future {
public:
    T get() noexcept;

    void wait() noexcept;

};

//promise
template <typename T>
class promise {
public:
    future<T> get_future() noexcept; 
};

// packaged_task

template <typename>
class packaged_task;

template <typename F, typename... Args>
class packaged_task<F(Args...)> {
    typedef typename result_of<F(Args...)>::type return_type;
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

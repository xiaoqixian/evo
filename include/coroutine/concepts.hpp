// Date:   Thu Mar 14 12:04:21 2024
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _CONCEPTS_HPP
#define _CONCEPTS_HPP

#include "concepts.h"
#include <coroutine>

namespace evo {

namespace concepts {

// add concepts for coroutine.
template <typename A>
concept awaiter = requires (A a, std::coroutine_handle<> h) {
    { a.await_ready() } -> evo::concepts::same_as<bool>;

    a.await_suspend(h);

    a.await_resume();
};

} // namespace concepts

} // namespace evo

#endif // _CONCEPTS_HPP

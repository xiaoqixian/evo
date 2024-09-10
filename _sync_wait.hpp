// Date:   Fri Mar 15 15:19:33 2024
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef __SYNC_WAIT_HPP
#define __SYNC_WAIT_HPP

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <optional>
#include <type_traits>
#include "debug.h"
#include "coroutine/concepts.hpp"
#include "coroutine/traits.hpp"

namespace evo {

// sync_wait_task is coroutine uses a conditional variable 
// to block the current thread, and release the conditional 
// variable on the final_suspend.
// so the current thread blocks until the coroutine is done.
template <typename>
class sync_wait_task;

class sync_wait_event {
    std::condition_variable cv;
    std::atomic_bool flag;
    std::mutex mutex;
public:
    sync_wait_event(): flag(false) {}

    void set() noexcept {
        if (!this->flag.exchange(true, std::memory_order_release)) {
            this->cv.notify_all();
        }
    }

    void reset() noexcept {
        // always use read-modify-write operation to participate
        // in the realse sequence.
        this->flag.exchange(false, std::memory_order_release);
    }

    void wait() noexcept {
        std::unique_lock<std::mutex> lk(this->mutex);
        this->cv.wait(lk, [this]() {
            return this->flag.load(std::memory_order_acquire);
        });
    }
};

namespace sync_wait_impl {

struct promise_base;

template <typename Ret>
struct promise_type;

} // namespace sync

template <typename Ret>
class sync_wait_task {

public:
    typedef sync_wait_impl::promise_type<Ret> promise_type;
    typedef std::coroutine_handle<promise_type> handle_t;

private:
    handle_t handle;

public:
    sync_wait_task(handle_t h) noexcept: handle(h) {}

    void start(std::shared_ptr<sync_wait_event> ev) {
        this->handle.promise().set_wait_handle(ev);
        this->handle.resume();
    }

    promise_type & promise() & noexcept {
        return this->handle.promise();
    }
    promise_type const& promise() const& noexcept {
        return this->handle.promise();
    }
};

namespace sync_wait_impl {

struct promise_base {
    typedef std::shared_ptr<sync_wait_event> wait_handle_t;
private:
    std::exception_ptr except_ptr;
public:
    wait_handle_t wait = nullptr;

    void set_wait_handle(wait_handle_t wait) {
        this->wait = wait;
    }

    auto initial_suspend() noexcept {
        return std::suspend_always();
    }

    void unhandled_exception() {
        this->except_ptr = std::current_exception();
    }
};

template <typename Ret = void>
struct promise_type: public promise_base {
    typedef Ret value_type;

    auto get_return_object() 
        noexcept(std::is_nothrow_constructible_v<
            sync_wait_task<Ret>, 
            typename sync_wait_task<Ret>::handle_t>)
    {
        return sync_wait_task<Ret>(
            std::coroutine_handle<promise_type<Ret>>::from_promise(*this)
        );
    }

    auto final_suspend() noexcept {
        struct awaiter {
            typedef std::coroutine_handle<promise_type<Ret>> handle_t;
            constexpr bool await_ready() const noexcept {
                return false;
            }
            void await_suspend(handle_t h) noexcept {
                ASSERT(h.promise().wait, "wait handle of promise type is not set");
                h.promise().wait->set();
            }
            void await_resume() const noexcept {}
        };

        return awaiter();
    }

    void return_value(value_type const& val) noexcept 
        requires evo::concepts::copy_constructible<value_type>
    {
        this->val.emplace(val);
    }

    std::optional<value_type> const& result() const& noexcept {
        return this->val;
    }
    std::optional<value_type> & result() & noexcept {
        return this->val;
    }
    std::optional<value_type> && result() && noexcept {
        return std::move(this->val);
    }
private:
    std::optional<value_type> val;
};

template <>
struct promise_type<void>: public promise_base {
    auto get_return_object() 
        noexcept(std::is_nothrow_constructible_v<
            sync_wait_task<void>,
            typename sync_wait_task<void>::handle_t
        >)
    {
        return sync_wait_task<void>(
            std::coroutine_handle<promise_type<void>>::from_promise(*this)
        );
    }

    auto final_suspend() noexcept {
        struct awaiter {
            typedef std::coroutine_handle<promise_type<void>> handle_t;
            constexpr bool await_ready() const noexcept {
                return false;
            }
            void await_suspend(handle_t h) noexcept {
                ASSERT(h.promise().wait, "wait handle of promise type is not set");
                h.promise().wait->set();
            }
            void await_resume() const noexcept {}
        };

        return awaiter();
    }

    void return_void() noexcept {}
};

} // namespace sync_wait_impl

using evo::concepts::awaiter;
using evo::concepts::awaitable;
using evo::awaitable_traits;

template <
    awaitable A, 
    typename Ret = typename awaitable_traits<A>::return_type>
static sync_wait_task<Ret> make_sync_wait_task(A&& awaitable_obj) {
    if constexpr (std::is_same_v<Ret, void>) {
        co_await std::forward<A>(awaitable_obj);
        co_return;
    } else {
        co_return co_await std::forward<A>(awaitable_obj);
    }
}

template <
    awaitable A, 
    typename Ret = typename awaitable_traits<A>::return_type>
static auto sync_wait(A&& awaitable_obj) {
    auto task = make_sync_wait_task<A>(std::forward<A>(awaitable_obj));
    auto wait_ev = 
        std::shared_ptr<sync_wait_event>(new sync_wait_event());
    task.start(wait_ev);
    wait_ev->wait();

    if constexpr (!std::is_void_v<Ret>) {
        return task.promise().result();
    }
}

} // namespace evo

#endif // __SYNC_WAIT_HPP

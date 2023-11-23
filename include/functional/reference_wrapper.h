// Date: Thu Nov 23 17:55:25 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _REFERENCE_WRAPPER_H
#define _REFERENCE_WRAPPER_H

#include "type_traits.h"
#include "utility/declval.h"

namespace evo {

template <typename T>
class reference_wrapper {
public:
    typedef T type;
private:
    type* ptr;
    static void func(T&) noexcept;
    static void func(T&&) noexcept = delete;

public:
    /// Constructor
    /// requires decay<U>::type is not the same type as reference_wrapper
    template <typename U, typename = enable_if<
        is_same<remove_cv_ref_t<U>, reference_wrapper>::value,
        decltype(func(declval<U>()))
    >>
    reference_wrapper(U&& u) noexcept(func(declval<U>())) {
        type& f = static_cast<U&&>(u);
        this->ptr = address_of(f);
    }

    operator type&() const noexcept {
        return *this->ptr;
    }
    
    type& get() const noexcept {
        return *this->ptr;
    }

    /// invokable reference
    template <typename... Args>
    typename invoke_of<type, Args...>::type
    operator() (Args... args) const {
        return invoke(get(), forward<Args>(declval<Args>())...);
    }
};

}

#endif // _REFERENCE_WRAPPER_H

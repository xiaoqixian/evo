// Date: Tue Nov 21 15:51:38 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _EXPECTED_HPP
#define _EXPECTED_HPP

#include "type_traits.h"
namespace evo {

template <typename T, typename E, bool = is_void<remove_cv_t<T>>::value>
class expected {
public:
    /// Default constructor, if T is not (possibly cv-qualified) void,
    /// constructs an object that contains the expected value.
    /// After construction, has_value() returns true
    template <bool = is_default_constructible<T>::value>
    explicit constexpr expected() noexcept = delete;
    /// if T is default constructible
    template <>
    explicit constexpr expected<true>() noexcept :val(T()), has(true) {}

    /// Copy constructor
    /// This constructor is defined as deleted unless
    ///     - either T is void, or T is copy constructible
    ///     - E is copy constructible
    template <bool = is_copy_constructible<T>::value>
    explicit constexpr expected(expected const& other) noexcept = delete;

    template <>
    explicit constexpr expected<true>(expected const& other) noexcept {

    }

private:
    T val;
    bool has = false;
};

/// As void type can't be set as a field
/// we have to specialize the class
template <typename T, typename E>
class expected<T, E, true> {
public:
    explicit constexpr expected() noexcept :has(true) {}
private:
    bool has = false;
};

} // namespace evo

#endif // _EXPECTED_HPP

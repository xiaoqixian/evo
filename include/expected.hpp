// Date: Tue Nov 21 15:51:38 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _EXPECTED_HPP
#define _EXPECTED_HPP

#include "type_traits.h"
#include "optional.hpp"
#include <initializer_list>
#include <type_traits>
#include "memory.h"

namespace evo {

template <typename>
struct unexpected;

template <typename, typename>
struct expected;

struct unexpect_t {
    explicit unexpect_t() = default;
};
inline constexpr unexpect_t unexpect {};

class bad_expected_access: public exception {
    virtual const char* what() const noexcept {
        return "bad_expected_access";
    }
};

template <typename>
struct is_evo_expected: false_type {};
template <typename T, typename E>
struct is_evo_expected<expected<T, E>>: true_type {};

template <typename>
struct is_evo_unexpected: false_type {};
template <typename T>
struct is_evo_unexpected<unexpected<T>>: true_type {};

template <typename E>
class unexpected {
    typedef E error_type;
private:
    error_type error_value;

public:

    constexpr unexpected(unexpected const&) = default;
    constexpr unexpected(unexpected &&) = default;

    template <typename Err = error_type>
        requires is_constructible_v<error_type, Err const&> 
    constexpr explicit unexpected(Err const& err) 
        noexcept(is_nothrow_constructible_v<error_type, Err>)
        : error_value(forward<Err>(err)) 
    {
        DEBUG("unexpected init with Err")
    }

    template <typename... Args>
        requires is_constructible_v<error_type, Args...> 
    constexpr explicit unexpected(in_place_t, Args&&... args)
        noexcept(is_nothrow_constructible_v<error_type, Args...>)
        : error_value(forward<Args>(args)...)
    {
        DEBUG("unexpected construct with in_place_t args")
    }

    template <typename U, typename... Args>
        requires is_constructible_v<error_type, std::initializer_list<U>&, Args...>
    constexpr explicit unexpected(in_place_t, std::initializer_list<U> il, Args&&... args)
        noexcept(is_nothrow_constructible_v<error_type, std::initializer_list<U>&, Args...>)
        : error_value(il, forward<Args>(args)...)
    {
        DEBUG("unexpected construct with initializer_list args")
    }

    constexpr error_type& error() {
        return this->err_value;
    }
};

// T can not be void.
template <typename T, typename E>
class expected {
    typedef T value_type;
    typedef E error_type;
private:
    bool has_val;
    union {
        value_type val;
        error_type err;
    };

    template <typename U, typename G>
    struct can_convert: And<
        If<is_same_v<remove_cv_t<T>, bool>, 
            true_type,
            Not<Or<
                is_constructible<T, expected<U, G>>,
                is_constructible<T, expected<U, G>&>,
                is_constructible<T, const expected<U, G>>,
                is_constructible<T, const expected<U, G>&>,
                is_convertible<expected<U, G>&, T>,
                is_convertible<expected<U, G>&&, T>,
                is_convertible<expected<U, G> const&, T>,
                is_convertible<expected<U, G> const&&, T>
            >>
        >,
        Not<Or<
            is_constructible<unexpected<E>, expected<U, G>>,
            is_constructible<unexpected<E>, expected<U, G>&>,
            is_constructible<unexpected<E>, const expected<U, G>&>,
            is_constructible<unexpected<E>, const expected<U, G>>
        >>
    > {};
public:

    // default constructor,
    // requires T to be void or is_default_constructible
    constexpr expected()
        noexcept(is_nothrow_default_constructible<value_type>::value)
        requires is_default_constructible_v<value_type>
        : val(), has_val(true) 
    {
        DEBUG("expected constructed with default constructor")
    }

    /*
     * Copy constructor. If other.has_value() is false, the new object contains an unexpected value, which is direct-initialized from other.error(). Otherwise, if T is not (possibly cv-qualified) void, the new object contains an expected value, which is direct-initialized from *other.
After construction, has_value() is equal to other.has_value().

This constructor is defined as deleted unless
either T is (possibly cv-qualified) void, or std::is_copy_constructible_v<T> is true, and
std::is_copy_constructible_v<E> is true.
This constructor is trivial if
either T is (possibly cv-qualified) void, or std::is_trivially_copy_constructible_v<T> is true, and
std::is_trivially_copy_constructible_v<E> is true.
     */
    constexpr expected(expected const&) = delete;

    constexpr expected(expected const&) 
        requires(is_copy_constructible_v<value_type> && 
                is_copy_constructible_v<error_type> &&
                is_trivially_copy_constructible_v<value_type> &&
                is_trivially_copy_constructible_v<error_type>
        )
    = default; 

    constexpr expected(expected const& other) 
        noexcept(
            is_nothrow_copy_constructible_v<value_type> &&
            is_nothrow_copy_constructible_v<error_type>)
        requires(is_copy_constructible_v<value_type> && 
                is_copy_constructible_v<error_type> &&
                !is_trivially_copy_constructible_v<value_type> &&
                !is_trivially_copy_constructible_v<error_type>)
        : has_val(other.has_val)
    {
        DEBUG("construct with expected const&")
        if (other.has_val) {
            // TODO: should've used construct_at here
            new(&(this->val)) value_type(other.value);
        } else {
            // TODO: should've used construct_at here
            new(&(this->err)) error_type(other.error);
        }
    }

    /*
     * Move constructor. If other.has_value() is false, the new object contains an unexpected value, which is direct-initialized from std::move(other.error()). Otherwise, if T is not (possibly cv-qualified) void, the new object contains an expected value, which is direct-initialized from std::move(*other).
After construction, has_value() is equal to other.has_value().

This constructor participates in overload resolution only if
either T is (possibly cv-qualified) void, or std::is_move_constructible_v<T> is true, and
std::is_move_constructible_v<E> is true.
This constructor is trivial if
std::is_trivially_move_constructible_v<T> is true, and
std::is_trivially_move_constructible_v<E> is true.
     */
    constexpr expected(expected &&) = delete;

    constexpr expected(expected &&) 
        requires(is_move_constructible_v<value_type> && 
                is_move_constructible_v<error_type> &&
                is_trivially_move_constructible_v<value_type> &&
                is_trivially_move_constructible_v<error_type>
                )
    = default; 

    constexpr expected(expected && other) 
        noexcept(
            is_nothrow_move_constructible_v<value_type> &&
            is_nothrow_move_constructible_v<error_type>)
        requires(is_move_constructible_v<value_type> && 
                is_move_constructible_v<error_type> &&
                !is_trivially_move_constructible_v<value_type> &&
                !is_trivially_move_constructible_v<error_type>)
        : has_val(other.has_val)
    {
        if (other.has_val) {
            // TODO: should've used construct_at here
            new(&(this->val)) value_type(move(other.value));
        } else {
            // TODO: should've used construct_at here
            new(&(this->err)) error_type(move(other.error));
        }
    }
    
    template <typename U, typename G>
    requires(
        is_constructible_v<T, U const&> &&
        is_constructible_v<E, G const&> &&
        can_convert<U, G>::value
    )
    constexpr explicit(!is_convertible_v<U const&, T> || !is_convertible_v<G const&, E>)
    expected(expected<U, G> const& other): has_val(other.has_val) {
        if (other.has_val) {
            new(&(this->val)) value_type(other.value);
        } else {
            new(&(this->err)) error_type(other.error);
        }
    }

    template <typename U, typename G>
    requires(
        is_constructible_v<T, U &&> &&
        is_constructible_v<E, G &&> &&
        can_convert<U, G>::value
    )
    constexpr explicit(!is_convertible_v<U&&, T> || !is_convertible_v<G&&, E>)
    expected(expected<U, G> && other): has_val(other.has_val) {
        if (other.has_val) {
            new(&(this->val)) value_type(move(other.value));
        } else {
            new(&(this->err)) error_type(move(other.error));
        }
    }
    
    /*
     *Constructs an object that contains an expected value, initialized as if direct-initializing (but not direct-list-initializing) an object of type T with the expression std::forward<U>(v).
After construction, has_value() returns true.

This constructor does not participate in overload resolution unless the following conditions are met:
T is not (possibly cv-qualified) void.
std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> is false.
std::is_same_v<expected, std::remove_cvref_t<U>> is false.
std::is_constructible_v<T, U> is true.
std::remove_cvref_t<U> is not a specialization of std::unexpected.
If T is (possibly cv-qualified) bool, std::remove_cvref_t<U> is not a specialization of std::expected.
     */
    template <typename U = value_type>
    requires(
        !is_same_v<remove_cv_ref_t<U>, in_place_t> &&
        !is_same_v<remove_cv_ref_t<U>, expected> &&
        is_constructible_v<T, U> &&
        !is_evo_unexpected<remove_cv_ref_t<U>>::value &&
        If<
            is_same_v<remove_cv_ref_t<T>, bool>,
            Not<is_evo_expected<remove_cv_ref_t<U>>>,
            true_type
        >::value
    )
    constexpr explicit(!is_convertible_v<U, value_type>)
    expected(U&& u): val(forward<value_type>(u)), has_val(true) {
        DEBUG("construct with rvalue U")
    }

    /*
     *7,8) Let GF be const G& for (7) and G for (8).
Constructs an object that contains an unexpected value, which is direct-initialized from std::forward<GF>(e.error()). After construction, has_value() returns false.

These overloads participate in overload resolution only if std::is_constructible_v<E, GF> is true.
     */
    template <typename G>
    requires(is_constructible_v<E, G const&>)
    constexpr explicit(!is_convertible_v<G const&, E>)
    expected(unexpected<G> const& other_err)
        noexcept(is_nothrow_constructible_v<E, G>)
        : err(other_err.error_value), has_val(false)
    {
        DEBUG("construct with G const&")
    }

    template <typename G>
    requires(is_constructible_v<E, G &&>)
    constexpr explicit(!is_convertible_v<G &&, E>)
    expected(unexpected<G> && other_err)
        noexcept(is_nothrow_constructible_v<E, G>)
        : err(move(other_err.error_value)), has_val(false)
    {
        DEBUG("construct with G &&")
    }

    /*
     * 9) Constructs an object that contains an expected value, which is direct-initialized from the arguments std::forward<Args>(args)....
After construction, has_value() returns true.

This overload participates in overload resolution only if std::is_constructible_v<T, Args...> is true.
     */
    template <typename... Args>
    requires(is_constructible_v<T, Args...>)
    constexpr explicit
    expected(in_place_t, Args&&... args)
        noexcept(is_nothrow_constructible_v<T, Args...>)
        : val(forward<Args>(args)...), has_val(true)
    {
        DEBUG("construct with Args...")
    }

    /*
     * 10) Constructs an object that contains an expected value, which is direct-initialized from the arguments il, std::forward<Args>(args)....
After construction, has_value() returns true.

This overload participates in overload resolution only if std::is_constructible_v<T, std::initializer_list<U>&, Args...> is true.
     */
    template <typename U, typename... Args>
    requires(is_constructible_v<T, std::initializer_list<U>&, Args...>)
    constexpr explicit
    expected(in_place_t, std::initializer_list<U>& il, Args&&... args)
        noexcept(is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)
        : val(il, move(args)...), has_val(true)
    {
        DEBUG("construct with initializer_list")
    }

    /*
     *  Constructs an object such that after construction, has_value() returns true.
     */
    template <typename...>
    constexpr explicit
    expected(in_place_t): has_val(true) {}

    template <typename... Args>
    requires(is_constructible_v<E, Args...>)
    constexpr explicit
    expected(unexpect_t, Args&&... args)
        noexcept(is_nothrow_constructible_v<E, Args...>)
        : err(forward<Args>(args)...), has_val(false)
    {
        DEBUG("construct with unexpect_t")
    }

    template <typename G, typename... Args>
    requires(is_constructible_v<E, std::initializer_list<G>, Args...>)
    constexpr explicit
    expected(unexpect_t, std::initializer_list<G>& il, Args&&... args)
        noexcept(is_nothrow_constructible_v<E, std::initializer_list<G>, Args...>)
        : err(il, forward<Args>(args)...), has_val(false)
    {
        DEBUG("construct with unexpect_t initializer_list")
    }

private:
    template <typename OldType, typename NewType, typename... Args>
    static constexpr void reinit_expected(OldType& old_val, NewType& new_val, Args&&... args) {
        if constexpr (is_nothrow_constructible_v<NewType, Args...>) {
            old_val.~OldType();
            new(&new_val) NewType(forward<Args>(args)...);
        }
        else if constexpr (is_nothrow_move_constructible_v<NewType>) {
            NewType temp(forward<Args>(args)...);
            old_val.~OldType();
            new(&new_val) NewType(forward<Args>(args)...);
        } 
        else {
            OldType temp(move(old_val));
            old_val.~OldType();
            try {
                new(&new_val) NewType(forward<Args>(args)...);
            } catch (...) {
                new(&old_val) OldType(move(temp));
                throw;
            }
        }
    }
    /*
     * Assigns the state of other.
If this->has_value() equals other.has_value(), assigns the value contained in other. Does nothing if T is (possibly cv-qualified) void and other.has_value() is true.
Otherwise, destroys the currently contained value (does nothing if this->has_value() is true and T is (possibly cv-qualified) void), and makes *this contain a copy of the value contained in other.
If other.has_value() is true and T is (possibly cv-qualified) void, does not construct the new value. Otherwise, the new value is copy-constructed (1) or move-constructed (2) from *other or other.error(), as appropriate. If an exception is thrown, the old value is retained; *this does not become valueless.
If no exception was thrown, after assignment, has_value() is equal to other.has_value().

Overload (1) is defined as deleted unless
either T is (possibly cv-qualified) void or std::is_copy_assignable_v<T> is true, and
either T is (possibly cv-qualified) void or std::is_copy_constructible_v<T> is true, and
std::is_copy_assignable_v<E> is true, and
std::is_copy_constructible_v<E> is true, and
at least one of the following is true:
T is (possibly cv-qualified) void
std::is_nothrow_move_constructible_v<T>
std::is_nothrow_move_constructible_v<E>
Overload (2) participates in overload resolution only if
either T is (possibly cv-qualified) void or std::is_move_assignable_v<T> is true, and
either T is (possibly cv-qualified) void or std::is_move_constructible_v<T> is true, and
std::is_move_assignable_v<E> is true, and
std::is_move_constructible_v<E> is true, and
at least one of the following is true:
T is (possibly cv-qualified) void
std::is_nothrow_move_constructible_v<T>
std::is_nothrow_move_constructible_v<E>
     */
public:
    expected& operator=(expected const& other) = delete;

    constexpr expected& operator=(expected const& other)
        noexcept(
            is_nothrow_copy_assignable_v<value_type> &&
            is_nothrow_copy_constructible_v<value_type> &&
            is_nothrow_copy_assignable_v<error_type> &&
            is_nothrow_copy_constructible_v<error_type>
        )
        requires(
            is_copy_assignable_v<value_type> &&
            is_copy_constructible_v<value_type> &&
            is_copy_assignable_v<error_type> &&
            is_copy_constructible_v<error_type> &&
            (is_nothrow_move_constructible_v<value_type> || 
             is_nothrow_move_constructible_v<error_type>)
        )
    {
        if (this->has_val && other.has_val) {
            this->val = other.value;
        } else if (this->has_val) {
            // this static function may throw an exception, 
            // but the original value/error will hold.
            reinit_expected(this->val, this->err, other.error);
        } else if (other.has_val) {
            reinit_expected(this->err, this->val, other.value);
        } else {
            this->err = other.error;
        }
        this->has_val = other.has_val;
        return *this;
    }

    constexpr expected& operator=(expected&& other)
        noexcept(
            is_nothrow_move_assignable_v<value_type> &&
            is_nothrow_move_constructible_v<value_type> &&
            is_nothrow_move_assignable_v<error_type> &&
            is_nothrow_move_constructible_v<error_type>
        )
        requires(
            is_move_assignable_v<value_type> &&
            is_move_constructible_v<value_type> &&
            is_move_assignable_v<error_type> &&
            is_move_constructible_v<error_type> &&
            (is_nothrow_move_constructible_v<value_type> || 
             is_nothrow_move_constructible_v<error_type>) &&
            std::is_destructible_v<value_type> &&
            std::is_destructible_v<error_type>
        )
    {
        if (this->has_val && other.has_val) {
            this->val = move(other.value);
        } else if (this->has_val) {
            reinit_expected(this->val, this->err, move(other.error));
        } else if (other.has_val) {
            reinit_expected(this->err, this->val, move(other.value));
        } else {
            this->err = move(other.error);
        }
        this->has_val = other.has_val;
        return *this;
    }

    // assign directly from a value
    template <typename U>
    constexpr expected& operator=(U&& u)
    requires(
        !is_same_v<expected, remove_cvref_t<U>> &&
        !is_evo_unexpected<remove_cvref_t<U>>::value &&
        is_constructible_v<value_type, U> &&
        is_assignable_v<value_type&, U> &&
        (is_nothrow_constructible_v<value_type, U> ||
         is_nothrow_move_constructible_v<value_type> ||
         is_nothrow_move_constructible_v<U>
        )
    )
    {
        if (this->has_val) {
            this->val = forward<U>(u);
        } else {
            reinit_expected(this->err, this->val, forward<U>(u));
            this->has_val = true;
        }

        return *this;
    }

    template <typename G>
    requires(
        is_constructible_v<error_type, G const&> &&
        is_assignable_v<error_type&, G const&> &&
        (is_nothrow_constructible_v<error_type, G const&> ||
         is_nothrow_move_constructible_v<value_type> ||
         is_nothrow_move_constructible_v<error_type>)
    )
    constexpr expected& operator=(unexpected<G> const& other) {
        if (this->has_val) {
            reinit_expected(this->val, this->err, other.error());
            this->has_val = false;
        } else {
            this->err = other.error();
        }
        return *this;
    }

    template <typename G>
    requires(
        is_constructible_v<error_type, G &&> &&
        is_assignable_v<error_type&, G &&> &&
        (is_nothrow_constructible_v<error_type, G &&> ||
         is_nothrow_move_constructible_v<value_type> ||
         is_nothrow_move_constructible_v<error_type>)
    )
    constexpr expected& operator=(unexpected<G> && other) {
        if (this->has_val) {
            reinit_expected(this->val, this->err, move(other.error()));
            this->has_val = false;
        } else {
            this->err = move(other.error());
        }
        return *this;
    }

    constexpr const value_type* operator->() const noexcept {
        ASSERT(this->has_val, "expected:operator-> requires the expected to contain a value");
        return address_of(this->val);
    }

    constexpr value_type* operator->() noexcept {
        ASSERT(this->has_val, "expected:operator-> requires the expected to contain a value");
        return address_of(this->val);
    }

    constexpr value_type const& operator*() const& noexcept {
        ASSERT(this->has_val, "expected:operator* requires the expected to contain a value");
        return this->val;
    }

    constexpr value_type& operator*() & noexcept {
        ASSERT(this->has_val, "expected:operator* requires the expected to contain a value");
        return this->val;
    }

    constexpr value_type const&& operator*() const&& noexcept {
        ASSERT(this->has_val, "expected:operator* requires the expected to contain a value");
        return move(this->val);
    }

    constexpr value_type&& operator*() && noexcept {
        ASSERT(this->has_val, "expected:operator* requires the expected to contain a value");
        return move(this->val);
    }

    constexpr bool operator()() const noexcept {
        return this->has_val;
    }

    constexpr bool has_value() const noexcept {
        return this->has_val;
    }

    constexpr value_type const& value() const& {
        if (this->has_val)
            return this->val;
        else 
            throw bad_expected_access();
    }

    constexpr value_type& value() & {
        if (this->has_val)
            return this->val;
        else 
            throw bad_expected_access();
    }

    constexpr value_type const&& value() const&& {
        if (this->has_val)
            return this->val;
        else 
            throw bad_expected_access();
    }

    constexpr value_type&& value() && {
        if (this->has_val)
            return this->val;
        else 
            throw bad_expected_access();
    }

    // The behavior is undefined if this->has_value() is true.
    constexpr error_type const& error() const& noexcept {
        return this->err;
    }
    constexpr error_type& error() & noexcept {
        return this->err;
    }
    constexpr error_type const&& error() const&& noexcept {
        return this->err;
    }
    constexpr error_type&& error() && noexcept {
        return this->err;
    }

    template <typename U>
    constexpr value_type const& value_or(U&& default_value) const& {
        static_assert(is_copy_constructible_v<value_type>, 
            "value_type has to be copy constructible");
        static_assert(is_convertible_v<U, value_type>, 
            "argument type has to be convertible to value_type");
        return this->has_val ? this->val : 
            static_cast<value_type>(forward<U>(default_value));
    }

    template <typename U>
    constexpr value_type&& value_or(U&& default_value) && {
        static_assert(is_move_constructible_v<value_type>, 
            "value_type has to be move constructible");
        static_assert(is_convertible_v<U, value_type>, 
            "argument type has to be convertible to value_type");
        return this->has_val ? move(this->val) :
            static_cast<value_type>(forward<U>(default_value));
    }
};

// In the special case of value_type being void type.
template <typename T, typename E>
    requires is_void_v<typename remove_cv<T>::type>
class expected<T, E> {
    
};


} // namespace evo

#endif // _EXPECTED_HPP

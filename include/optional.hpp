// Date: Wed Nov 22 23:37:35 2023
// Mail: lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#ifndef _OPTIONAL_HPP
#define _OPTIONAL_HPP

#include "type_traits.h"
#include "exception.h"
#include "utility.h"
#include <cassert>
#include <functional>
#include <initializer_list>

namespace evo {

class bad_optional_access: public exception {
    virtual const char* what() const noexcept {
        return "bad_optional_access";
    }
};

struct nullopt_t {
    struct secret_tag {
        inline explicit secret_tag() = default;
    };
    inline constexpr explicit nullopt_t(secret_tag, secret_tag) noexcept {}
};


/// We have to seperate a optional destruct base because we have to 
/// differ types that are trivially destructible or not.
/// And we don't want to define all other irrelavent stuff again, 
/// so we pack all relavent stuff into optional_desctruct_base.
template <typename T, bool = is_trivially_destructible<T>::value>
class optional_desctruct_base;

template <typename T>
struct optional_desctruct_base<T, false> {
    typedef T value_type;

    union {
        char null_state;
        value_type val;
    };
    bool is_engaged;

    ~optional_desctruct_base() {
        if (this->is_engaged)
            this->val.~value_type();
    }

    constexpr optional_desctruct_base() noexcept
        : null_state(), is_engaged(false) {}

    /// construct in place
    /// work of type parameters checking is left to derived classes.
    template <typename... Args>
    constexpr explicit optional_desctruct_base(in_place_t, Args&&... args)
        : val(forward<Args>(args)...), is_engaged(true) {}

    inline void reset() noexcept {
        if (this->is_engaged) {
            this->val.~value_type();
            this->is_engaged = false;
        }
    }
};

template <typename T>
struct optional_desctruct_base<T, true> {
    typedef T value_type;

    union {
        char null_state;
        value_type val;
    };
    bool is_engaged;

    ~optional_desctruct_base() {
        if (this->is_engaged)
            this->val.~value_type();
    }

    constexpr optional_desctruct_base() noexcept
        : null_state(), is_engaged(false) {}

    /// construct in place
    /// work of type parameter checking is left for derived classes.
    template <typename... Args>
    constexpr explicit optional_desctruct_base(in_place_t, Args&&... args)
        : val(forward<Args>(args)...), is_engaged(true) {}

    inline void reset() noexcept {
        if (this->is_engaged) {
            // the only difference between trivially/non-trivially 
            // destructible value_type is that the trivially destructible
            // typpes do not need to explicitly invoke destructor.
            this->is_engaged = false;
        }
    }
};

/// we leave code about accessing the value in optional_storage_base
template <typename T, bool = is_reference<T>::value>
struct optional_storage_base: optional_desctruct_base<T> {
    using base = optional_desctruct_base<T>;
    using value_type = T;

    // using the base class's constructor allows the derived classes 
    // accessing the base class's constructor.
    using base::base;

    constexpr bool has_value() const noexcept {
        return this->is_engaged;
    }

    constexpr value_type& get() & noexcept {
        return this->val;
    }
    constexpr value_type const& get() const& noexcept {
        return this->val;
    }
    constexpr value_type&& get() && noexcept {
        return move(this->val);
    }
    constexpr value_type const&& get() const&& noexcept {
        return move(this->val);
    }

    template <typename... Args>
    void construct(Args&&... args) {
        assert(!this->has_value());
        new(&(this->val)) value_type(forward<Args>(args)...);
        this->is_engaged = true;
    }

    // the work of making sure that the Other type 
    // is left to the derived classes.
    template <typename Other>
    void construct_from(Other&& opt) {
        if (opt.has_value()) {
            construct(forward<Other>(opt).get());
        }
    }

    // so as the construct_from
    template <typename Other>
    void assign_from(Other&& opt) {
        if (this->is_engaged == opt.has_value()) {
            if (this->is_engaged) {
                this->val = forward<Other>(opt).get();
            }
        } else {
            if (this->is_engaged) {
                this->reset();
            } else {
                construct(forward<Other>(opt).get());
            }
        }
    }
};

// TODO: optional_storage_base carries reference type.

template <typename T, bool = is_trivially_copy_constructible<T>::value>
struct optional_copy_base: optional_storage_base<T> {
    using optional_storage_base<T>::optional_storage_base;
};

/// If T does not have a trivial copy constructor, 
/// then optional_copy_base<T> cannot have a trivial copy constructor.
template <typename T>
struct optional_copy_base<T, false>: optional_storage_base<T> {
    using optional_storage_base<T>::optional_storage_base;

    optional_copy_base() = default;

    optional_copy_base(optional_copy_base const& opt) {
        this->construct_from(opt);
    }

    optional_copy_base(optional_copy_base&&) = default;

    optional_copy_base& operator=(optional_copy_base const&) = default;
    optional_copy_base& operator=(optional_copy_base &&) = default;
};

template <typename T, bool = is_trivially_copy_constructible<T>::value>
struct optional_move_base: optional_copy_base<T> {
    using optional_copy_base<T>::optional_copy_base;
};

/// If T does not have a trivial move constructor, 
/// then optional_copy_base<T> cannot have a trivial move constructor.
template <typename T>
struct optional_move_base<T, false>: optional_copy_base<T> {
    using optional_copy_base<T>::optional_copy_base;

    optional_move_base() = default;

    optional_move_base(optional_move_base const&) = default;

    optional_move_base(optional_move_base&& opt) {
        this->construct_from(move(opt));
    }

    optional_move_base& operator=(optional_move_base const&) = default;
    optional_move_base& operator=(optional_move_base &&) = default;
};

template <typename T, bool = 
    is_trivially_destructible<T>::value &&
    is_trivially_copy_constructible<T>::value &&
    is_trivially_copy_assignable<T>::value>
struct optional_copy_assign_base: optional_move_base<T> {
    using optional_move_base<T>::optional_move_base;
};

template <typename T>
struct optional_copy_assign_base<T, false>: optional_move_base<T> {
    optional_copy_assign_base() = default;
    optional_copy_assign_base(optional_copy_assign_base const&) = default;
    optional_copy_assign_base(optional_copy_assign_base&&) = default;

    optional_copy_assign_base& operator=(optional_copy_assign_base const& opt) {
        this->assign_from(opt);
        return *this;
    }

    optional_copy_assign_base& operator=(optional_copy_assign_base&&) = default;
};

template <typename T, bool = 
    is_trivially_destructible<T>::value &&
    is_trivially_move_constructible<T>::value &&
    is_trivially_move_assignable<T>::value>
struct optional_move_assign_base: optional_copy_assign_base<T> {
    using optional_copy_assign_base<T>::optional_copy_assign_base;
};

template <typename T>
struct optional_move_assign_base<T, false>: optional_move_base<T> {
    optional_move_assign_base() = default;
    optional_move_assign_base(optional_move_assign_base const&) = default;
    optional_move_assign_base(optional_move_assign_base&&) = default;

    optional_move_assign_base& operator=(optional_move_assign_base const& opt) = default;

    optional_move_assign_base& operator=(optional_move_assign_base&& opt) 
        noexcept(is_nothrow_move_assignable<T>::value && 
                is_nothrow_move_constructible<T>::value)
    {
        this->assign_from(move(opt));
        return *this;
    }
};

template <typename T>
using optional_sfinae_ctor_base_t = sfinae_ctor_base<
    is_copy_constructible<T>::value, 
    is_move_constructible<T>::value>;

template <typename T>
using optional_sfinae_assign_base_t = sfinae_assign_base<
    is_copy_assignable<T>::value, 
    is_move_assignable<T>::value>;

template <typename T>
class optional
    : private optional_move_base<T>
    , private optional_sfinae_ctor_base_t<T>
    , private optional_sfinae_assign_base_t<T>
{
    using base = optional_move_assign_base<T>;
public:
    using value_type = T;

private:
    static_assert(!is_same_v<value_type, in_place_t>, 
            "instantiation of optional with in_place_t is ill-formed");
    static_assert(!is_same_v<value_type, nullopt_t>, 
            "instantiation of optional with nullopt_t is ill-formed");
    static_assert(!is_reference_v<value_type>, 
            "instantiation of optional with a reference type is ill-formed");
    static_assert(std::is_destructible_v<value_type>, 
            "instantiation of optional with a non-destructible type is ill-formed");
    static_assert(!is_array_v<value_type>, 
            "instantiation of optional with an array type is ill-formed");

    struct check_tuple_constructor_fail {

        static constexpr bool enable_explicit_default() { return false; }
        static constexpr bool enable_implicit_default() { return false; }
        template <class ...>
        static constexpr bool enable_explicit() { return false; }
        template <class ...>
        static constexpr bool enable_implicit() { return false; }
        template <class ...>
        static constexpr bool enable_assign() { return false; }
    };

    struct check_optional_args_constructor {
        template <typename U>
        static constexpr bool enable_implicit() {
            return is_constructible_v<T, U&&> &&
                is_convertible_v<U&&, T>;
        }

        template <typename U>
        static constexpr bool enable_explicit() {
            return is_constructible_v<T, U&&> &&
                !is_convertible_v<U&&, T>;
        }
    };

    template <typename U>
    using check_optional_args_ctor = If<
        is_not_same<remove_cv_ref_t<U>, in_place_t>::value &&
        is_not_same<remove_cv_t<U>, optional>::value,
        check_optional_args_constructor,
        check_tuple_constructor_fail
    >;

    // U will be the raw type.
    // QualU is the actual type, like U const& or U &&.
    template <typename QualU>
    struct check_optional_like_constructor {
        template <typename U, typename Opt = optional<U>>
        using check_constructible_from_opt = Or<
          is_constructible<T, Opt&>,
          is_constructible<T, Opt const&>,
          is_constructible<T, Opt&&>,
          is_constructible<T, Opt const&&>,
          is_convertible<Opt&, T>,
          is_convertible<Opt const&, T>,
          is_convertible<Opt&&, T>,
          is_convertible<Opt const&&, T>
        >;

        template <typename U, typename Opt = optional<U>>
        using check_assignable_from_opt = Or<
          is_assignable<T, Opt&>,
          is_assignable<T, Opt const&>,
          is_assignable<T, Opt&&>,
          is_assignable<T, Opt const&&>
        >;
        
        template <typename U, typename QU = QualU>
        static constexpr bool enable_implicit() {
            return is_convertible_v<QU, T> &&
                !check_constructible_from_opt<U>::value;
        }

        template <typename U, typename QU = QualU>
        static constexpr bool enable_explicit() {
            return !is_constructible_v<QU, T> &&
                !check_constructible_from_opt<U>::value;
        }

        template <typename U, typename QU = QualU>
        static constexpr bool enable_assign() {
            return !check_constructible_from_opt<U>::value &&
                !check_assignable_from_opt<U>::value;
        }
    };

    // check optional like args
    template <typename U, typename QualU>
    using check_optional_like_ctor = If<
        And<
            is_not_same<U, T>,
            is_constructible<T, QualU>
        >::value,
        check_optional_like_constructor<QualU>,
        check_tuple_constructor_fail
    >;

    template <typename U, typename QualU>
    using check_optional_liek_assign = If<
        And<
            is_not_same<U, T>,
            is_constructible<T, QualU>,
            is_assignable<T&, QualU>
        >::value,
        check_optional_like_constructor<QualU>,
        check_tuple_constructor_fail
    >;

public:
    optional() = default;
    optional(optional const&) = default;
    optional(optional &&) = default;

    template <typename InPlaceT, typename... Args, typename = enable_if<
        And<
            is_same<InPlaceT, in_place_t>,
            is_constructible<value_type, Args...>
        >::value
    >>
    constexpr explicit optional(InPlaceT, Args&&... args):
        base(in_place, forward<Args>(args)...) {}

    template <typename U, typename... Args, typename = enable_if<
        is_constructible_v<value_type, std::initializer_list<U>&, Args...>
    , void>>
    constexpr explicit optional(in_place_t, std::initializer_list<U> il, Args&&... args):
        base(in_place, il, forward<Args>(args)...) {}

    // if T is able to implicitly construct with U
    template <typename U = value_type, enable_if<
        check_optional_args_ctor<U>::template enable_implicit<U>()
    , int> = 0>
    constexpr optional(U&& v)
        : base(in_place, forward<U>(v)) {}

    // if T is only able to construct with U explicitly
    template <typename U = value_type, enable_if<
        check_optional_args_ctor<U>::template enable_explicit<U>()
    , int> = 0>
    constexpr explicit optional(U&& v)
        : base(in_place, forward<U>(v)) {}

    // if the incoming argument is optional<U>,
    // and T is implicitly constructible with U const&.
    template <typename U, enable_if<
        check_optional_like_ctor<U, U const&>::template enable_implicit<U>()
    , int> = 0>
    optional(optional<U> const& other) {
        this->construct_from(other);
    }

    // if the incoming argument is optional<U>,
    // and T is explicitly constructible with U const&.
    template <typename U, enable_if<
        check_optional_like_ctor<U, U const&>::template enable_explicit()
    , int> = 0>
    explicit optional(optional<U> const& other) {
        this->construct_from(other);
    }

    // if the incoming argument is optional<U>,
    // and T is implicitly constructible with U &&.
    template <typename U, enable_if<
        check_optional_like_ctor<U, U &&>::template enable_implicit<U>()
    , int> = 0>
    optional(optional<U> && other) {
        this->construct_from(move(other));
    }

    // if the incoming argument is optional<U>,
    // and T is explicitly constructible with U &&.
    template <typename U, enable_if<
        check_optional_like_ctor<U, U &&>::template enable_explicit()
    , int> = 0>
    explicit optional(optional<U> && other) {
        this->construct_from(move(other));
    }

    // assgin from nullopt_t
    optional& operator=(nullopt_t) noexcept {
        this->reset();
        return *this;
    }

    optional& operator=(optional const&) = default;
    optional& operator=(optional &&) = default;

    // assign from rvalue reference of type U.
    // U is required to not be optional type, 
    // and be either not the same as value_type,
    // or is not scalar type.
    template <typename U = value_type, typename = enable_if<
        And<
            is_not_same<remove_cv_ref_t<U>, optional>,
            Or<
                is_not_same<remove_cv_ref_t<U>, value_type>,
                Not<is_scalar<value_type>>
            >
        >::value
    >>
    optional& operator=(U&& v) {
        if (this->has_value()) {
            this->get() = forward<U>(v);
        } else {
            this->construct(forward<U>(v));
        }
        return *this;
    }

    // assign from rvalue reference type of optional<U>
    template <typename U, enable_if<
        check_optional_like_ctor<U, U &&>::template enable_assign<U>()
    , int> = 0>
    optional& operator=(optional<U>&& other) {
        this->assign_from(other);
        return *this;
    }

    // construct with multiple arguments,
    // where value_type is constructible with these arguments
    // optional is allowed to have value.
    template <typename... Args, typename = enable_if<
        is_constructible_v<value_type, Args...>
    >>
    T& emplace(Args&&... args) {
        this->reset();
        this->construct(forward<Args>(args)...);
        return this->get();
    }

    template <typename U, typename... Args, typename = enable_if<
        is_constructible_v<value_type, std::initializer_list<U>, Args...>
    >>
    T& emplace(std::initializer_list<U> il, Args&&... args) {
        this->reset();
        this->construct(il, forward<Args>(args)...);
        return this->get();
    }

    void swap(optional& opt) 
        noexcept(is_nothrow_move_constructible_v<value_type> && 
                std::is_nothrow_swappable_v<value_type>) 
    {
        if (this->has_value() == opt.has_value()) {
            if (this->has_value()) {
                swap(this->get(), opt.get());
            }
        } else {
            if (this->has_value()) {
                opt.construct(move(this->get()));
                this->reset();
            } else {
                this->construct(move(opt.get()));
                opt.reset();
            }
        }
    }

    constexpr add_pointer_t<value_type const>
    operator->() const {
        assert(this->has_value());
        return address_of(this->get());
    }

    constexpr add_pointer_t<value_type>
    operator->() {
        assert(this->has_value());
        return address_of(this->get());
    }

    constexpr value_type const& operator*() const& {
        assert(this->has_value());
        return this->get();
    }

    constexpr value_type& operator*() & {
        assert(this->has_value());
        return this->get();
    }

    constexpr value_type const&& operator*() const&& {
        assert(this->has_value());
        return this->get();
    }

    constexpr value_type && operator*() && {
        assert(this->has_value());
        return move(this->get());
    }

    constexpr explicit operator bool() const noexcept {
        return this->has_value();
    }

    using base::has_value;
    using base::get;

    constexpr value_type const& value() const& {
        if (!this->has_value()) {
            throw bad_optional_access();
        }
        return this->get();
    }

    constexpr value_type & value() & {
        if (!this->has_value()) {
            throw bad_optional_access();
        }
        return this->get();
    }

    constexpr value_type && value() && {
        if (!this->has_value()) {
            throw bad_optional_access();
        }
        return this->get();
    }

    constexpr value_type const&& value() const&& {
        if (!this->has_value()) {
            throw bad_optional_access();
        }
        return this->get();
    }

    template <typename U>
    constexpr value_type value_or(U&& u) const& {
        static_assert(is_copy_constructible_v<value_type>, 
            "optional<T>::value_or: T must be copy constructible");
        static_assert(is_convertible_v<U, value_type>, 
            "optional<T>::value_or: U must be convertible to T");
        return this->has_value() ? this->get() :
            static_cast<value_type>(forward<U>(u));
    }

    template <typename U>
    constexpr value_type value_or(U&& u) && {
        static_assert(is_move_constructible_v<value_type>, 
            "optional<T>::value_or: T must be copy constructible");
        static_assert(is_convertible_v<U, value_type>, 
            "optional<T>::value_or: U must be convertible to T");
        return this->has_value() ? move(this->get()) :
            static_cast<value_type>(forward<U>(u));
    }

    using base::reset;
};


// comparisons between optionals
template <typename T, typename U>
enable_if<
    is_convertible_v<decltype(declval<T const&>() == declval<U const&>()), bool>, bool
> operator==(optional<T> const& t, optional<U> const& u) {
    if (static_cast<bool>(t) != static_cast<bool>(u))
        return false;
    if (!static_cast<bool>(t))
        return true;
    return *t == *u;
}

template <typename T, typename U>
enable_if<
    is_convertible_v<decltype(declval<T const&>() != declval<U const&>()), bool>, bool
> operator!=(optional<T> const& t, optional<U> const& u) {
    if (static_cast<bool>(t) != static_cast<bool>(u))
        return true;
    if (!static_cast<bool>(t))
        return false;
    return *t != *u;
}

} // end of evo namespace

#endif // _OPTIONAL_HPP

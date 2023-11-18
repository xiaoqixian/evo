/**********************************************
  > File Name		: optional.h
  > Author		    : lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Sun May  7 12:30:32 2023
  > Location        : Shanghai
  > Copyright@ https://github.com/xiaoqixian
 **********************************************/
#ifndef _OPTIONAL_H
#define _OPTIONAL_H

#include <initializer_list>

#include "type_traits.h"
#include "utility/forward.h"
#include "utility/move.h"
#include "utility.h"
#include "exception.h"

namespace evo {

//null optional type
struct nullopt_t {
    struct secret_tag {explicit secret_tag() = default;};

    constexpr explicit nullopt_t(secret_tag, secret_tag) noexcept {}
};

//bad optional access exception
//threw when access optional which has no value
class bad_optional_access: public evo::exception {
public:
    virtual ~bad_optional_access() noexcept;
    virtual const char* what() const noexcept;
};

template <typename T, bool = evo::is_trivially_destructible<T>::value>
struct optional_destruct_base;

template <typename T> 
struct optional_destruct_base<T, false> {
    typedef T value_type;
    static_assert(
            is_object<value_type>::value,
            "instantiation of optional with a non-object type is undefined behavior");

    union {
        char null_state;
        value_type val;
    };
    // set true if val is set.
    // otherwise set false
    bool engaged;

    ~optional_destruct_base() {
        if (engaged)
            val.~value_type();
    }

    constexpr optional_destruct_base() noexcept : null_state(), engaged(false) {}

    template <typename... Args>
        constexpr explicit optional_destruct_base(Args &&...args)
        : val(evo::forward<Args>(args)...), engaged(true) {}

    void reset() noexcept {
        if (engaged) {
            val.~value_type();
            engaged = false;
        }
    }
};

template <typename T> 
struct optional_destruct_base<T, true> {
    typedef T value_type;
    static_assert(
            evo::is_object<T>::value,
            "instantiation of optional with a non-object type is undefined behavior");
    union {
        char null_state;
        value_type val;
    };
    bool engaged;

    constexpr optional_destruct_base() noexcept : null_state(), engaged(false) {}

    template <typename... Args>
        constexpr explicit optional_destruct_base(Args &&...args)
        : val(evo::forward<Args>(args)...), engaged(true) {}

    void reset() noexcept {
        if (engaged)
            engaged = false;
    }
};

template <typename T, bool = evo::is_reference<T>::value>
struct optional_storage_base : optional_destruct_base<T> {
    typedef T value_type;
    typedef evo::remove_reference<T> raw_type;
    raw_type *value;

    template <typename U>
    static constexpr bool can_bind_reference() {
        typedef typename evo::remove_reference<U>::type RawU;
        typedef RawU* UPtr;
        typedef typename evo::remove_reference<T>::type RawT;
        typedef RawT* TPtr;

        //TODO reference_wrapper  required
        using CheckValueArg = evo::bool_constant<
            (evo::is_lvalue_reference<U>::value && evo::is_convertible<UPtr, TPtr>::value)
            >;
        return (evo::is_lvalue_reference<T>::value && CheckValueArg::value) ||
            (evo::is_lvalue_reference<T>::value && !evo::is_lvalue_reference<U>::value && evo::is_convertible<UPtr, TPtr>::value);
    }

    constexpr optional_storage_base() noexcept: value(nullptr) {}

    template <typename UArg>
    constexpr explicit optional_storage_base(UArg&& uarg)
        : value(evo::address_of(uarg)) {
            static_assert(can_bind_reference<UArg>(), 
            "Attempted to construct a reference element in tuple from a possible temporary");
        }

    void reset() noexcept {value = nullptr;}

    constexpr bool has_value() const noexcept {
        return value != nullptr;
    }

    constexpr value_type& get() const& noexcept {
        return *value;
    }

    constexpr value_type&& get() const&& noexcept {
        return evo::forward<value_type>(*value);
    }

    template <typename UArg>
    void construct(UArg&& val) {
        static_assert(can_bind_reference<UArg>(), "Attempted to construct a reference element in tuple from a possible temporary");
        value = evo::address_of(val);
    }

    template <typename That>
    void construc_from(That&& opt) {
        if (opt.has_value())
            construct(evo::forward<That>(opt).get());
    }

    template <typename That>
    void assign_from(That&& opt) {
        if (has_value() == opt.has_value()) {
            if (has_value())
                *value = evo::forward<That>(opt).get();
        } else {
            if (has_value()) reset();
            else construct(evo::forward<That>(opt).get());
        }
    }
};

template <typename T, bool = evo::is_trivially_copy_constructible<T>::value> 
struct optional_copy_base: optional_storage_base<T> {
    using optional_storage_base<T>::optional_storage_base;
};

template <typename T>
struct optional_copy_base<T, false>: optional_storage_base<T> {
    using optional_storage_base<T>::optional_storage_base;
    
    optional_copy_base() = default;
    optional_copy_base(const optional_copy_base& opt) {
        this->construc_from(opt);
    }
    optional_copy_base(optional_copy_base&&) = default;

    optional_copy_base& operator=(const optional_copy_base&) = default;
    optional_copy_base& operator=(optional_copy_base&&) = default;
};

template <typename T, bool = evo::is_trivially_move_constructible<T>::value>
struct optional_move_base: optional_copy_base<T> {
    using optional_copy_base<T>::optional_copy_base;
};

template <typename T>
struct optional_move_base<T, false>: optional_copy_base<T> {
    using value_type = T;
    using optional_copy_base<T>::optional_copy_base;

    optional_move_base() = default;
    optional_move_base(const optional_move_base&) = default;
    
    optional_move_base(optional_move_base&& opt) noexcept(evo::is_nothrow_move_constructible<value_type>::value) {
        this->construc_from(evo::move(opt));
    }

    optional_move_base& operator=(optional_move_base const&) = default;
    optional_move_base& operator=(optional_move_base&&) = default;
};

template <typename T, bool = 
    evo::is_trivially_destructible<T>::value &&
    evo::is_trivially_copy_constructible<T>::value &&
    evo::is_trivially_copy_assignable<T>::value>
struct optional_copy_assign_base: optional_move_base<T> {
    using optional_move_base<T>::optional_move_base;
};

template <typename T>
struct optional_copy_assign_base<T, false>: optional_move_base<T> {
    using optional_move_base<T>::optional_move_base;

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
    evo::is_trivially_destructible<T>::value &&
    evo::is_trivially_move_constructible<T>::value &&
    evo::is_trivially_move_assignable<T>::value>
struct optional_move_assign_base: optional_copy_assign_base<T> {
    using optional_copy_assign_base<T>::optional_copy_assign_base;
};

template <typename T>
struct optional_move_assign_base<T, false>: optional_copy_assign_base<T> {
    typedef T value_type;
    using optional_copy_assign_base<T>::optional_copy_assign_base;

    optional_move_assign_base() = default;
    optional_move_assign_base(optional_move_assign_base const&) = default;
    optional_move_assign_base(optional_move_assign_base&&) = default;

    optional_move_assign_base& operator=(optional_move_assign_base const&) = default;
    optional_move_assign_base& operator=(optional_move_assign_base&& opt)
        noexcept(evo::is_nothrow_move_assignable<value_type>::value &&
                evo::is_nothrow_move_constructible<value_type>::value) {
            this->construc_from(evo::move(opt));
            return *this;
    }
};

template <typename T>
using optional_sfinae_ctor_base = evo::sfinae_ctor_base<
    evo::is_copy_constructible<T>::value,
    evo::is_move_constructible<T>::value>;

template <typename T>
using optional_sfinae_assign_base = evo::sfinae_assign_base<
    evo::is_copy_constructible<T>::value && evo::is_copy_assignable<T>::value,
    evo::is_move_constructible<T>::value && evo::is_move_assignable<T>::value>;

template <typename T> 
class optional:
    private optional_move_assign_base<T>,
    private optional_sfinae_ctor_base<T>,
    private optional_sfinae_assign_base<T> {

    typedef optional_move_assign_base<T> base;
    typedef T value_type;

private:
    struct CheckOptionalArgsConstructor {
        template <typename U>
        static constexpr bool enable_implicit() {
            return evo::is_constructible<T, U&&>::value &&
                evo::is_convertible<U&&, T>::value;
        }

        template <typename U>
        static constexpr bool enable_explicit() {
            return evo::is_constructible<T, U&&>::value &&
                !evo::is_convertible<U&&, T>::value;
        }
    };

    //TODO check_tuple_constructor_fail required
    template <typename U>
    using CheckOptionalArgsCtor = evo::conditional<
        evo::is_not_same<typename evo::remove_cv_ref<U>::type, evo::in_place_t>::value &&
        evo::is_not_same<typename evo::remove_cv_ref<U>::type, optional>::value,
        CheckOptionalArgsConstructor,
        void>;

    template <typename QualU>
    struct CheckOptionalLikeConstructor {
        template <typename U, typename Opt = optional<U>>
        using CheckConstructibleFromOpt = evo::Or<
            evo::is_constructible<T, Opt&>,
            evo::is_constructible<T, Opt const&>,
            evo::is_constructible<T, Opt&&>,
            evo::is_constructible<T, Opt const&&>,
            evo::is_convertible<Opt&, T>,
            evo::is_convertible<Opt const&, T>,
            evo::is_convertible<Opt&&, T>,
            evo::is_convertible<Opt const&&, T>
        >;

        template <typename U, typename Opt = optional<U>>
        using CheckAssignableFromOpt = evo::Or<
            evo::is_assignable<T&, Opt&>,
            evo::is_assignable<T&, Opt const&>,
            evo::is_assignable<T&, Opt&&>,
            evo::is_assignable<T&, Opt const&&>
        >;

        template <typename U, typename QU = QualU>
        static constexpr bool enable_implicit() {
            return evo::is_convertible<QU, T>::value &&
                !CheckConstructibleFromOpt<U>::value;
        }
        template <typename U, typename QU = QualU>
        static constexpr bool enable_explicit() {
            return !evo::is_convertible<QU, T>::value &&
                !CheckConstructibleFromOpt<U>::value;
        }
        template <typename U, typename QU = QualU>
        static constexpr bool enable_assign() {
            return !CheckConstructibleFromOpt<U>::value &&
                !CheckAssignableFromOpt<U>::value;
        }
    };

    //TODO check_tuple_constructor_fail required
    template <typename U, typename QualU>
    using CheckOptionalLikeCtor = evo::conditional<
        evo::And<
            evo::is_not_same<U, T>,
            evo::is_constructible<T, QualU>
        >::value,
        CheckOptionalLikeConstructor<QualU>,
        void
    >;

    //TODO check_tuple_constructor_fail required
    template <typename U, typename QualU>
    using CheckOptionalLikeAssign = evo::conditional<
        evo::And<
            evo::is_not_same<U, T>,
            evo::is_constructible<T, QualU>,
            evo::is_assignable<T&, QualU>
        >::value,
        CheckOptionalLikeConstructor<QualU>,
        void
    >;
    
public:
    constexpr optional() noexcept {}
    constexpr optional(optional const&) = default;
    constexpr optional(optional&&) = default;
    //TODO nullopt_t type parameter required

    //construct in place
    template <typename InPlaceT, typename... Args, typename = 
        evo::enable_if<
            evo::And<
                evo::is_same<InPlaceT, evo::in_place_t>,
                evo::is_constructible<value_type, Args...>
            >::value
        >
    >
    constexpr explicit optional(InPlaceT, Args&&... args)
        : base(evo::in_place_t {}, evo::forward<Args>(args)...) {}

    //TODO initializer_list parameter required


    //construct with rvalue reference
    template <typename U = value_type, evo::enable_if<
        CheckOptionalArgsCtor<U>::template enable_implicit<U>(),
        int> = 0
    >
    constexpr optional(U&& u): base(evo::in_place_t{}, evo::forward<U>(u)) {}

    //explicitly construct with rvalue reference
    template <typename U, evo::enable_if<
        CheckOptionalArgsCtor<U>::template enable_explicit<U>(),
        int> = 0>
    constexpr explicit optional(U&& u): base(evo::in_place_t{}, evo::forward<U>(u)) {}

    //construct with optional lvalue reference
    template <typename U, evo::enable_if<
        CheckOptionalLikeCtor<U, U const&>::template enable_implicit<U>(),
        int> = 0>
    optional(optional<U> const& v) {
        this->construct_from(v);
    }

    //explicitly construct with optional lvalue reference
    template <typename U, evo::enable_if<
        CheckOptionalLikeCtor<U, U const&>::template enable_explicit<U>(),
        int> = 0>
    explicit optional(optional<U> const& v) {
        this->construct_from(v);
    }

    //construct with optional rvalue reference
    template <typename U, evo::enable_if<
        CheckOptionalLikeCtor<U, U&&>::template enable_implicit<U>(),
        int> = 0>
    optional(optional<U>&& v) {
        this->construc_from(evo::move(v));
    }

    //explicitly construct with optional rvalue reference
    template <typename U, evo::enable_if<
        CheckOptionalLikeCtor<U, U&&>::template enable_explicit<U>(),
        int> = 0>
    explicit optional(optional<U>&& v) {
        this->construc_from(evo::move(v));
    }

    optional& operator=(nullopt_t) noexcept {
        this->reset();
        return *this;
    }

    optional& operator=(optional const&) = default;
    optional& operator=(optional&&) = default;

    template <typename U = value_type, typename = evo::enable_if<
        evo::And<
            evo::is_not_same<typename evo::remove_cv_ref<U>::type, optional>,
            evo::Or<
                evo::is_not_same<typename evo::remove_cv_ref<U>::type, value_type>,
                evo::Not<evo::is_scalar<value_type>>
            >,
            evo::is_constructible<value_type, U>,
            evo::is_assignable<value_type&, U>
        >::value>>
    optional& operator=(U&& v) {
        if (this->has_value())
            this->get() = evo::forward<U>(v);
        else 
            this->construct(evo::forward<U>(v));
        return *this;
    }

    template <typename U, evo::enable_if<
        CheckOptionalLikeAssign<U, U const&>::template  enable_assign<U>(),
        int> = 0>
    optional& operator=(optional<U> const& v) {
        this->assign_from(v);
        return *this;
    }

    template <typename U, evo::enable_if<
        CheckOptionalLikeCtor<U, U&&>::template enable_assign<U>(),
        int> = 0>
    optional& operator=(optional<U>&& v) {
        this->assign_from(evo::move(v));
        return *this;
    }

    template <typename U, typename... Args, typename = evo::enable_if<
        evo::is_constructible<value_type, 
        std::initializer_list<U>&, 
        Args...>::value
    >>
    T& emplace(std::initializer_list<U> il, Args&&... args) {
        this->reset();
        this->construct(il, evo::forward<Args>(args)...);
        return this->get();
    }

    void swap(optional& opt) 
        noexcept(evo::is_nothrow_move_constructible<value_type>::value && 
                evo::is_nothrow_swappable<value_type>::value) {
        if (this->has_value() == opt.has_value()) {
            if (this->has_value())
                evo::swap(this->get(), opt.get());
        } else {
            if (this->has_value()) {
                opt.construct(evo::move(this->get()));
                this->reset();
            } else {
                this->construct(evo::move(opt.get()));
                opt.reset();
            }
        }
    }

    constexpr typename evo::add_pointer<value_type>::type operator->() {
        assert(this->has_value(), "optional operator-> called on disengaged value");
        return evo::address_of(this->get());
    }

    constexpr const value_type& operator*() const& noexcept {
        assert(this->has_value(), "optional operator* called on disengaged value");
        return this->get();
    }

    constexpr value_type& operator*() & noexcept {
        assert(this->has_value(), "optional operator* called on disengaged value");
        return this->get();
    }

    constexpr const value_type&& operator*() const&& noexcept {
        assert(this->has_value(), "optional operator* called on disengaged value");
        return evo::move(this->get());
    }

    constexpr value_type&& operator*() && noexcept {
        assert(this->has_value(), "optional operator* called on disengaged value");
        return evo::move(this->get());
    }

    constexpr explicit operator bool() const noexcept {
        return this->has_value();
    }

    using base::has_value;
    using base::get;

    constexpr value_type const& value() const& {
        if (!this->has_value())
            throw bad_optional_access{};
        return this->get();
    }

    constexpr value_type& value() & {
        if (!this->has_value())
            throw bad_optional_access{};
        return this->get();
    }

    constexpr value_type const&& value() const&& {
        if (!this->has_value())
            throw bad_optional_access{};
        return evo::move(this->get());
    }

    constexpr value_type&& value() && {
        if (!this->has_value())
            throw bad_optional_access{};
        return evo::move(this->get());
    }

    template <typename U>
    constexpr value_type value_or(U&& u) const& {
        static_assert(evo::is_copy_constructible<value_type>::value, 
                "optional<T>::value_or: T must be copy consructible");
        static_assert(evo::is_convertible<U, value_type>::value, 
                "optional<T>::value_or: U must be convertible to T");
        return this->has_value() ? this->get() : 
            static_cast<value_type>(evo::forward<U>(u));
    }

    using base::reset;
};

//user defined template deduction guides
template <typename T>
optional(T) -> optional<T>;

} // namespace evo

#endif /* _OPTIONAL_H*/

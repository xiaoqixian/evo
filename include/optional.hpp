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

/*template <typename T>*/
/*class optional: */
    /*optional_desctruct_base<T>*/
/*{*/
/*private:*/
    /*typedef remove_cv_ref_t<T> raw_type;*/

    /*// whether optional contains a value deterines*/
    /*// on if val == nullptr*/

/*private:*/
    /*struct CheckOptionalArgsConstructor {*/
        /*template <typename U>*/
        /*static constexpr bool enable_implicit() {*/
            /*return evo::is_constructible<T, U&&>::value &&*/
                /*evo::is_convertible<U&&, T>::value;*/
        /*}*/

        /*template <typename U>*/
        /*static constexpr bool enable_explicit() {*/
            /*return evo::is_constructible<T, U&&>::value &&*/
                /*!evo::is_convertible<U&&, T>::value;*/
        /*}*/
    /*};*/

    /*template <typename U>*/
    /*static constexpr bool can_bind_reference() {*/
        /*using RawU = remove_reference_t<U>;*/
        /*using UPtr = RawU*;*/
        /*using RawT = remove_reference_t<T>;*/
        /*using TPtr = RawT*;*/
        /*using CheckLvalueArg = bool_constant<*/
            /*(is_lvalue_reference<U>::value && is_convertible<UPtr, TPtr>::value) ||*/
            /*is_same<U, std::reference_wrapper<T>>::value ||*/
            /*is_same<U, std::reference_wrapper<typename remove_const<RawT>::type>>::value*/
        /*>;*/
        /*return (is_lvalue_reference<T>::value && CheckLvalueArg::value)*/
            /*|| (is_rvalue_reference<T>::value && !is_lvalue_reference<U>::value && is_convertible<UPtr, TPtr>::value);*/
    /*}*/

    /*static_assert(!is_same_v<remove_cv_ref<T>, in_place_t>, */
            /*"instantiation of optional with in_place_t is ill-formed");*/
    /*static_assert(!is_same_v<remove_cv_ref<T>, nullopt_t>, */
            /*"instantiation of optional with nullopt_t is ill-formed");*/
    /*static_assert(!is_reference_v<T>, */
            /*"instantiation of optional with reference type is ill-formed");*/
    /*static_assert(std::is_destructible_v<T>, */
            /*"instantiation of optional with a non-destructible type is ill-formed");*/
    /*static_assert(!std::is_array_v<T>, */
            /*"instantiation of optional with an array type is ill-formed");*/

    /*/// Construct on the address of this->val*/
    /*template <typename... Args>*/
    /*void construct(Args&&... args) {*/
        
    /*}*/

/*public:*/
    /*constexpr optional() noexcept: val(nullptr) {}*/

    /*/// or equivalently construct with nullopt_t*/
    /*constexpr optional(nullopt_t) noexcept {}*/

    /*/// Copy constructor (default)*/
    /*/// this constructor only accepts optional<T> const& type argugment*/
    /*constexpr optional(optional const&) = default;*/
    
    /*/// Move constructor (default)*/
    /*/// this constructor only accepts optional<T> && type argugment*/
    /*constexpr optional(optional &&) = default;*/

    /*/// Copy constructor (implicit)*/
    /*/// constructs with a optional with a different value type U*/
    /*/// but T is constructible with U const& and*/
    /*/// U const& can convert to T*/
    /*template <typename U = T, typename enable_if<*/
        /*is_constructible<T, U const&>::value &&*/
        /*is_convertible<U const&, T>::value*/
        /*, int>::type = 0>*/
    /*constexpr optional(optional<U> const& u): val(address_of(u)) {*/
        /*static_assert(can_bind_reference<U>(), */
            /*"Attempted to bind a reference element in tuple from a possible temporary");*/
    /*}*/
    
    /*/// Copy constructor (implicit)*/
    /*/// constructs with a optional with a different value type U*/
    /*/// but T is constructible with U const& and*/
    /*/// U const& can convert to T*/
    /*template <typename U = T, typename enable_if<*/
        /*is_constructible<T, U const&>::value &&*/
        /*!is_convertible<U const&, T>::value*/
        /*, int>::type = 0>*/
    /*constexpr explicit optional(optional<U> const& u) {*/
        /*if (u.has_value()) {*/
            /*this->val = address_of(T(u.get()));*/
        /*} else {*/
            /*this->val = nullptr;*/
        /*}*/
    /*}*/

    /*/// Move constructor*/
    /*/// enable implicit*/
    /*template <typename U = T, typename evo::enable_if<*/
        /*CheckOptionalArgsConstructor::template enable_implicit<U>()*/
        /*, int>::type = 0>*/
    /*constexpr optional(U&& u): val(address_of(u)) {*/
        /*//TODO static_assert U can bind reference*/
    /*}*/

    /*/// Move constructor*/
    /*/// enable explicit */
    /*template <typename U = T, typename evo::enable_if<*/
        /*CheckOptionalArgsConstructor::template enable_explicit<U>()*/
        /*, int>::type = 0>*/
    /*constexpr explicit optional(U&& u): val(address_of(u)) {}*/

    /*constexpr bool has_value() const noexcept {*/
        /*return this->val != nullptr;*/
    /*}*/

    /*constexpr T& get() & {*/
        /*if (!this->has_value()) {*/
            /*throw bad_optional_access();*/
        /*} */
        /*return *this->val;*/
    /*}*/

    /*constexpr T const& get() const& {*/
        /*if (!this->has_value()) {*/
            /*throw bad_optional_access();*/
        /*}*/
        /*return *this->val;*/
    /*}*/
/*};*/

}

#endif // _OPTIONAL_HPP

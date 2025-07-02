// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_DETAIL_CALL_SIGNATURE_H
#define VOIDSTAR_DETAIL_CALL_SIGNATURE_H

#include <voidstar/detail/misc.h>

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>

namespace voidstar::detail {

// TODO: Differentiate `extern "C"` and `extern "C++"` function types on
// supporting compilers (e.g. Oracle)

/// @brief `extern "C++"` non-variadic function type.
template <typename R, typename... T> using default_abi_fn = R(T...);

/// @brief `extern "C++"` variadic function type.
template <typename R, typename... T> using default_abi_var_fn = R(T..., ...);

/// @brief Function traits for a function specifier @a F.
template <typename F> struct call_signature;

/// @brief Function traits for non-variadic function types.
template <typename R, typename... T>
struct call_signature<default_abi_fn<R, T...>> {
  using return_type = std::remove_cv_t<R>;
  using arg_types = std::tuple<std::decay_t<T>...>;
  static constexpr std::size_t arg_count = sizeof...(T);

  /// @brief Pointer-to-function type described by these properties.
  using fn_ptr_type = default_abi_fn<R, std::decay_t<T>...> *;
};

/// @brief Function traits for variadic function types.
template <typename R, typename... T>
struct call_signature<default_abi_var_fn<R, T...>> {
  static_assert(dependent_false<R>::value,
                "Variadic functions are not yet supported");
};

/// @brief Function traits for pointer-to-function types.
template <typename T> struct call_signature<T *> : call_signature<T> {};

/**
 * @brief Determines whether `std::apply(p_val, t_val)` is well-formed for
 * tuple-like @a T.
 */
template <typename P, typename T> struct can_std_apply;

template <typename P, typename... A>
struct can_std_apply<P, std::tuple<A...>> : std::is_invocable<P, A...> {};

/**
 * @brief Closure payload that can service trampolines with call signature @a C.
 */
// clang-format off
template <typename P, typename C>
concept matches =
  requires {
    typename C::return_type;
    typename C::arg_types;
  }

  // Avoid hard errors if std::apply fails
  and can_std_apply<P, typename C::arg_types>::value

  and requires(P payload, typename C::arg_types const &arg_tuple) {
    {std::apply(payload, arg_tuple)} // TODO: static_cast ins and outs
      -> std::convertible_to<typename C::return_type>;
  };
// clang-format on

} // namespace voidstar::detail

#endif

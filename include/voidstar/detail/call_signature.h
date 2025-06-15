#pragma once

#include <voidstar/detail/misc.h>

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>

namespace voidstar::detail {

template <typename R, typename... T> using default_abi_fn = R(T...);
template <typename R, typename... T> using default_abi_var_fn = R(T..., ...);

template <typename> struct call_signature;

template <typename R, typename... T>
struct call_signature<default_abi_fn<R, T...>> {
  using return_type = R;
  using arg_types = std::tuple<std::decay_t<T>...>;
  static constexpr std::size_t arg_count = sizeof...(T);

  using fn_ptr_type = default_abi_fn<R, std::decay_t<T>...> *;
};

template <typename R, typename... T>
struct call_signature<default_abi_var_fn<R, T...>> {
  static_assert(dependent_false<R>::value,
                "Variadic functions are not yet supported");
};

template <typename T> struct call_signature<T *> : call_signature<T> {};

template <typename P, typename> struct can_std_apply;

template <typename P, typename... A>
struct can_std_apply<P, std::tuple<A...>> : std::is_invocable<P, A...> {};

// clang-format off
template <typename P, typename C>
concept matches =
  requires {
    typename C::return_type;
    typename C::arg_types;
  }

  and can_std_apply<P, typename C::arg_types>::value

  and requires(P payload, typename C::arg_types const &arg_tuple) {
    {std::apply(payload, arg_tuple)}
      -> std::convertible_to<typename C::return_type>;
  };
// clang-format on

} // namespace voidstar::detail

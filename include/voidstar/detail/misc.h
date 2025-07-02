// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_DETAIL_MISC_H
#define VOIDSTAR_DETAIL_MISC_H

#include <functional>
#include <type_traits>
#include <utility>

namespace voidstar::detail {

/**
 * @brief A non-copyable, non-movable type, a-la `std::marker::PhantomPinned`
 * from Rust.
 *
 * Include as a `[[no_unique_address]]` member to make a type pinned.
 */
struct pin {
  pin() = default;
  pin(pin const &) = delete;
  auto operator=(pin const &) -> pin & = delete;
};

/**
 * @brief A constexpr `false` value technically dependent on a parameter.
 *
 * Constructs like `static_assert(false)` or `static_assert(sizeof(T) == 0)` can
 * never pass, and in C++20 compilers are allowed to reject such code before
 * instantiation. Someone _could_ define a specialization for `dependent_false`,
 * though, so `static_assert(dependent_false<T>::value)` cannot be rejected
 * prematurely.
 */
template <typename> struct dependent_false : std::false_type {};

/**
 * @brief A strongly typed constexpr value @a V.
 *
 * Simplifies writing lambdas with constexpr indices:
 *
 * ```c++
 * auto lambda = [](auto index) { std::get<index>(some_tuple); };
 * lambda(5);           // Error: `index` is not a constant expression
 * lambda(constant<5>); // Compiles
 * ```
 */
template <std::integral auto V>
static constexpr std::integral_constant<decltype(V), V> constant;

/**
 * @brief Apply @a callable for constexpr indices 0, 1, ... @a N-1.
 *
 * ```c++
 * auto lambda = [](auto... i) { int array[i + ...]; };
 * with_indices_zero_thru<5>(lambda); // Instantiates int[0 + 1 + 2 + 3 + 4]
 * ```
 */
template <std::size_t N, typename C>
static constexpr auto with_indices_zero_thru(C &&callable) {
  return std::invoke(
      [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::invoke(std::forward<C>(callable), constant<I>...);
      },
      std::make_index_sequence<N>());
}

/// @brief Create a `std::array<T, N>` filled with copies of @a value.
template <std::size_t N, typename T>
static constexpr auto n_copies(T value) -> std::array<T, N> {
  return with_indices_zero_thru<N>(
      [&](auto... i) { return std::array{((void)i, T{value})...}; });
}

} // namespace voidstar::detail

#endif

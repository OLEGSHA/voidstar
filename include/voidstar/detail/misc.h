#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace voidstar::detail {

struct pin {
  pin() = default;
  pin(pin const &) = delete;
  auto operator=(pin const &) -> pin & = delete;
};

template <auto> struct dependent_false : std::false_type {};

template <std::integral auto V>
static constexpr std::integral_constant<decltype(V), V> constant;

template <std::size_t N, typename C>
static constexpr auto with_indices_zero_thru(C &&callable) {
  return std::invoke(
      [&]<std::size_t... I>(std::index_sequence<I...>) {
        return std::invoke(std::forward<C>(callable), constant<I>...);
      },
      std::make_index_sequence<N>());
}

template <std::size_t N, typename T>
static constexpr auto n_copies(T value) -> std::array<T, N> {
  return with_indices_zero_thru<N>(
      [&](auto... i) { return std::array{((void)i, T{value})...}; });
}

} // namespace voidstar::detail

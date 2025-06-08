#pragma once

#include <ffi.h>

#include <concepts>
#include <tuple>

namespace voidstar::detail::ffi {

template <typename T> struct layout {};

template <typename T>
concept has_layout = requires {
  // clang-format off
  typename layout<T>::members;

  { std::tuple_size_v<typename layout<T>::members> }
    -> std::convertible_to<std::size_t>;
  // clang-format on
};

} // namespace voidstar::detail::ffi

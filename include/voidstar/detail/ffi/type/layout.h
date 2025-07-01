// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_DETAIL_FFI_TYPE_LAYOUT_H
#define VOIDSTAR_DETAIL_FFI_TYPE_LAYOUT_H

#include <voidstar/layout.h>

#include <ffi.h>

#include <concepts>
#include <tuple>

namespace voidstar::detail::ffi {

template <typename T>
concept needs_auto_layout = not requires {
  typename layout<T>::members;
};

template <typename T> struct computed_layout;

template <typename T>
requires(not needs_auto_layout<T>) struct computed_layout<T> {
  using members = typename layout<T>::members;
};

template <typename T>
concept has_computed_layout = requires {
  // clang-format off
  typename computed_layout<T>::members;

  { std::tuple_size_v<typename computed_layout<T>::members> }
    -> std::convertible_to<std::size_t>;
  // clang-format on
};

} // namespace voidstar::detail::ffi

#endif

#pragma once

#include <voidstar/detail/ffi/type/base.h>
#include <voidstar/detail/ffi/type/fundamental.h>
#include <voidstar/detail/ffi/type/layout.h>
#include <voidstar/detail/misc.h>

#include <ffi.h>

#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>

namespace voidstar::detail::ffi {

template <std::size_t N> struct member_list : std::array<::ffi_type *, N + 1> {
  constexpr member_list(std::array<::ffi_type *, N> values) noexcept {
    std::ranges::copy(values, this->data());
    (*this)[N] = nullptr;
  }
};

// To enable arbitrarily nested specialization usage, all declarations must
// occur before all definitions.

////////////////////////////////////////////////////////////////////////////////
// Declarations
//

template <typename T>
requires std::is_bounded_array_v<T>
struct type_description<T>;

template <typename T>
requires has_layout<T>
struct type_description<T>;

////////////////////////////////////////////////////////////////////////////////
// Definitions
//

template <typename T>
requires std::is_bounded_array_v<T>
class type_description<T> {
private:
  static constexpr auto size = std::extent_v<T>;

  type_description<std::remove_extent_t<T>> m_element;
  member_list<size> m_member_list = {n_copies<size>(m_element.raw())};
  ::ffi_type m_raw{
      .size = sizeof(T),
      .alignment = alignof(T),
      .type = FFI_TYPE_STRUCT,
      .elements = m_member_list.data(),
  };

  [[no_unique_address]] pin m_pin;

public:
  [[nodiscard]] constexpr auto raw() noexcept -> ::ffi_type * {
    return &this->m_raw;
  }
};

template <typename T, typename member_types> struct struct_type_description;

template <typename T, typename... M>
class struct_type_description<T, std::tuple<M...>> {
private:
  static constexpr auto size = sizeof...(M);

  std::tuple<type_description<M>...> m_members;
  member_list<size> m_member_list{{std::get<M>(m_members).raw()...}};

  ::ffi_type m_raw{
      .size = sizeof(T),
      .alignment = alignof(T),
      .type = FFI_TYPE_STRUCT,
      .members = m_member_list.data(),
  };

  [[no_unique_address]] pin m_pin;

public:
  [[nodiscard]] constexpr auto raw() noexcept -> ::ffi_type * {
    return &this->m_raw;
  }
};

template <typename T>
requires has_layout<T>
struct type_description<T>
    : struct_type_description<T, typename layout<T>::members> {
};

} // namespace voidstar::detail::ffi

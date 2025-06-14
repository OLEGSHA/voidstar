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
requires std::is_pointer_v<T> or std::is_unbounded_array_v<T>
struct type_description<T>;

template <typename T>
requires std::is_enum_v<T>
struct type_description<T>;

template <typename T>
requires std::is_bounded_array_v<T>
struct type_description<T>;

template <typename T>
requires std::is_union_v<T>
struct type_description<T>;

template <typename T>
requires has_layout<T>
struct type_description<T>;

////////////////////////////////////////////////////////////////////////////////
// Definitions
//

template <typename T>
requires std::is_pointer_v<T> or std::is_unbounded_array_v<T>
struct type_description<T> {
  [[nodiscard]] static constexpr auto raw() noexcept -> ::ffi_type * {
    return &::ffi_type_pointer;
  }
};

template <typename T>
requires std::is_enum_v<T>
struct type_description<T> {
  [[nodiscard]] constexpr auto raw() noexcept -> ::ffi_type * {
    using U = std::underlying_type_t<T>;
    return integer_type<std::is_signed_v<U>, sizeof(U), alignof(U)>;
  }
};

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

template <typename T>
requires std::is_union_v<T>
struct type_description<T> {
  // libffi doesn't have robust union support. Determining safe cases is not yet
  // supported by voidstar.
  //
  // The often-cited "struct with greatest member" doesn't actually work with
  // some union types, such as union{int, float} on x86_64.
  //
  // See https://github.com/libffi/libffi/issues/33 for more details.
  static_assert(dependent_false<T>::value, "Union types are not yet supported");
};

template <typename T, typename member_types> struct struct_type_description;

template <typename T, typename... M>
class struct_type_description<T, std::tuple<M...>> {
private:
  static constexpr auto size = sizeof...(M);

  static_assert((... and not std::is_unbounded_array_v<M>),
                "Unbounded arrays cannot be member types. Structs with "
                "flexible array members (C feature) are not supported.");

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

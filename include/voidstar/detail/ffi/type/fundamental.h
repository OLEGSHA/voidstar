#pragma once

#include <voidstar/detail/ffi/type/base.h>
#include <voidstar/detail/misc.h>

#include <ffi.h>

#include <array>
#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace voidstar::detail::ffi {

////////////////////////////////////////////////////////////////////////////////
// Integer type lookup table
//

template <bool signedness, std::size_t size, std::size_t alignment>
inline constexpr ::ffi_type *integer_type =
    // Disable base template
    std::enable_if_t<dependent_false<std::bool_constant<signedness>>{},
                     ::ffi_type *>{nullptr};

// clang-format off
#define VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(CPP_TYPE, FFI_VAR)                 \
  template <>                                                                  \
  inline constexpr ::ffi_type *integer_type<                                   \
      std::is_signed_v<CPP_TYPE>,                                              \
      sizeof(CPP_TYPE),                                                        \
      alignof(CPP_TYPE)                                                        \
  > = &FFI_VAR;
// clang-format on

VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(std::uint8_t, ::ffi_type_uint8)
VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(std::int8_t, ::ffi_type_sint8)
VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(std::uint16_t, ::ffi_type_uint16)
VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(std::int16_t, ::ffi_type_sint16)
VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(std::uint32_t, ::ffi_type_uint32)
VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(std::int32_t, ::ffi_type_sint32)
VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(std::uint64_t, ::ffi_type_uint64)
VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP(std::int64_t, ::ffi_type_sint64)

#undef VOIDSTAR_DEFINE_INTEGER_TYPE_LOOKUP

////////////////////////////////////////////////////////////////////////////////
// Integer-like types
//

#define VOIDSTAR_DEFINE_INTEGER_TYPE(CPP_TYPE)                                 \
  template <> struct type_description<CPP_TYPE> {                              \
    [[nodiscard]] constexpr auto raw() noexcept -> ::ffi_type * {              \
      return integer_type<std::is_signed_v<CPP_TYPE>, sizeof(CPP_TYPE),        \
                          alignof(CPP_TYPE)>;                                  \
    }                                                                          \
  };

VOIDSTAR_DEFINE_INTEGER_TYPE(signed char)
VOIDSTAR_DEFINE_INTEGER_TYPE(unsigned char)
VOIDSTAR_DEFINE_INTEGER_TYPE(signed short)
VOIDSTAR_DEFINE_INTEGER_TYPE(unsigned short)
VOIDSTAR_DEFINE_INTEGER_TYPE(signed int)
VOIDSTAR_DEFINE_INTEGER_TYPE(unsigned int)
VOIDSTAR_DEFINE_INTEGER_TYPE(signed long)
VOIDSTAR_DEFINE_INTEGER_TYPE(unsigned long)
VOIDSTAR_DEFINE_INTEGER_TYPE(signed long long)
VOIDSTAR_DEFINE_INTEGER_TYPE(unsigned long long)

VOIDSTAR_DEFINE_INTEGER_TYPE(char)
VOIDSTAR_DEFINE_INTEGER_TYPE(wchar_t)
VOIDSTAR_DEFINE_INTEGER_TYPE(char8_t)
VOIDSTAR_DEFINE_INTEGER_TYPE(char16_t)
VOIDSTAR_DEFINE_INTEGER_TYPE(char32_t)

VOIDSTAR_DEFINE_INTEGER_TYPE(bool)

#undef VOIDSTAR_DEFINE_INTEGER_TYPE

////////////////////////////////////////////////////////////////////////////////
// Special types
//

#define VOIDSTAR_DEFINE_TYPE(CPP_TYPE, FFI_VAR)                                \
  template <> struct type_description<CPP_TYPE> {                              \
    [[nodiscard]] constexpr auto raw() noexcept -> ::ffi_type * {              \
      return &FFI_VAR;                                                         \
    }                                                                          \
  };

VOIDSTAR_DEFINE_TYPE(void, ffi_type_void)
VOIDSTAR_DEFINE_TYPE(float, ffi_type_float)
VOIDSTAR_DEFINE_TYPE(double, ffi_type_double)
VOIDSTAR_DEFINE_TYPE(long double, ffi_type_longdouble)

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
VOIDSTAR_DEFINE_TYPE(_Complex float, ffi_type_complex_float)
VOIDSTAR_DEFINE_TYPE(_Complex double, ffi_type_complex_double)
VOIDSTAR_DEFINE_TYPE(_Complex long double, ffi_type_complex_longdouble)
#endif

#undef VOIDSTAR_DEFINE_TYPE

} // namespace voidstar::detail::ffi

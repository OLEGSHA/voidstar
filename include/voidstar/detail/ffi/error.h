#ifndef VOIDSTAR_DETAIL_FFI_ERROR_H
#define VOIDSTAR_DETAIL_FFI_ERROR_H

#include <voidstar/error.h>

#include <ffi.h>

#include <exception>
#include <stdexcept>
#include <string>

namespace voidstar::detail::ffi {

[[nodiscard]] static auto status_name(ffi_status status) -> std::string {
  switch (status) {
  case FFI_OK:
    return "FFI_OK";
  case FFI_BAD_TYPEDEF:
    return "FFI_BAD_TYPEDEF";
  case FFI_BAD_ABI:
    return "FFI_BAD_ABI";
  case FFI_BAD_ARGTYPE:
    return "FFI_BAD_ARGTYPE";
  default:
    return std::to_string(status);
  }
};

struct error : voidstar::error {
  using voidstar::error::error;

  error(std::string function, ffi_status status)
      : voidstar::error{function + ": error " + status_name(status)} {}
};

[[nodiscard]] static auto call(auto function,
                               std::string_view func_name) /* -> invoker */ {
  return [=](auto... args) {
    auto const result = function(args...);
    if (result != FFI_OK) {
      throw error{std::string{func_name}, result};
    }
  };
}

} // namespace voidstar::detail::ffi

#endif

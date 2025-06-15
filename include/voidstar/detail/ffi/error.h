#pragma once

#include <ffi.h>

#include <exception>
#include <stdexcept>
#include <string>

namespace voidstar::detail::ffi {

[[nodiscard]] static auto ffi_status_name(ffi_status status) -> std::string {
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

struct ffi_error : std::runtime_error {
  using std::runtime_error::runtime_error;

  ffi_error(std::string ffi_function, ffi_status status)
      : std::runtime_error{ffi_function + ": error " +
                           ffi_status_name(status)} {}
};

[[nodiscard]] static auto
ffi_call(auto ffi_func, std::string_view func_name) /* -> invoker */ {
  return [=](auto... args) {
    auto const result = ffi_func(args...);
    if (result != FFI_OK) {
      throw ffi_error{std::string{func_name}, result};
    }
  };
}

} // namespace voidstar::detail::ffi

// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_DETAIL_FFI_ERROR_H
#define VOIDSTAR_DETAIL_FFI_ERROR_H

#include <voidstar/error.h>

#include <ffi.h>

#include <exception>
#include <stdexcept>
#include <string>

namespace voidstar::detail::ffi {

/// @brief libffi constant name for @a status or `to_string(status)`.
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

/// @brief A libffi call has not completed successfully.
struct error : voidstar::error {
  using voidstar::error::error;

  error(std::string function, ffi_status status)
      : voidstar::error{function + ": error " + status_name(status)} {}
};

/**
 * @brief libffi function caller with automatic error handling.
 *
 * Returns a callable that invokes @a function with arguments provided to it and
 * checks the return code. If it is not `FFI_OK`, throws an ffi::error.
 *
 * @param func_name Name of @a function for error reporting.
 */
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

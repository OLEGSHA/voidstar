#pragma once

#include <ffi.h>

#include <exception>
#include <stdexcept>
#include <string>

namespace voidstar::detail::ffi {

[[nodiscard]] auto ffi_status_name(::ffi_status) -> std::string;

struct ffi_error : std::runtime_error {
  using std::runtime_error::runtime_error;

  ffi_error(std::string ffi_function, ::ffi_status status)
      : std::runtime_error{ffi_function + ": error " +
                           ffi_status_name(status)} {}
};

} // namespace voidstar::detail::ffi

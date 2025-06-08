#include <voidstar/call_signature.h>
#include <voidstar/detail/ffi/cif.h>
#include <voidstar/detail/ffi/closure.h>
#include <voidstar/detail/ffi/error.h>
#include <voidstar/detail/ffi/type.h>

#include <ffi.h>

#include <string>

namespace voidstar::detail::ffi {

auto ffi_status_name(::ffi_status status) -> std::string {
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

namespace {

auto ffi_call(auto ffi_func, std::string_view func_name) /* -> invoker */ {
  return [=](auto... args) {
    auto const result = ffi_func(args...);
    if (result != FFI_OK) {
      throw ffi_error{std::string{func_name}, result};
    }
  };
}

} // namespace

void prep_cif(::ffi_cif &cif, ::ffi_type &rtype, std::span<ffi_type *> atypes) {
  ffi_call(::ffi_prep_cif, "ffi_prep_cif") //
      (&cif, FFI_DEFAULT_ABI, atypes.size(), &rtype, atypes.data());
}

void closure::deleter::operator()(::ffi_closure *writable) const noexcept {
  ::ffi_closure_free(writable);
}

closure::closure()
    : m_executable_ptr{nullptr},
      m_raw{
          static_cast<::ffi_closure *>(
              ::ffi_closure_alloc(sizeof(::ffi_closure), &m_executable_ptr)),
      } {
  if (m_raw == nullptr or m_executable_ptr == nullptr) {
    throw ffi_error{"Could not allocate an FFI closure"};
  }
}

void prep_closure_loc(::ffi_closure *closure, ::ffi_cif *cif,
                      void (*fun)(::ffi_cif *cif, void *ret, void **args,
                                  void *user_data),
                      void *user_data, void *codeloc) {
  ffi_call(::ffi_prep_closure_loc, "ffi_prep_closure_loc") //
      (closure, cif, fun, user_data, codeloc);
}

} // namespace voidstar::detail::ffi

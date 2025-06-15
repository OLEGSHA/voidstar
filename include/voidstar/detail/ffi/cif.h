#pragma once

#include <voidstar/detail/ffi/error.h>
#include <voidstar/detail/ffi/type.h>
#include <voidstar/detail/misc.h>

#include <ffi.h>

#include <functional>
#include <initializer_list>
#include <memory>
#include <span>

namespace voidstar::detail::ffi {

template <typename call_signature, typename arg_types> struct cif_impl;

template <typename call_signature, typename... A>
class cif_impl<call_signature, std::tuple<A...>> {
private:
  type_description<typename call_signature::return_type> m_return_type;
  std::tuple<type_description<A>...> m_arg_types;

  using arg_type_list = std::array<ffi_type *, call_signature::arg_count>;

  arg_type_list m_arg_type_list = std::apply(
      [&](auto &&...arg_types) { return arg_type_list{arg_types.raw()...}; },
      m_arg_types);

  ffi_cif m_raw;

  [[no_unique_address]] pin m_pin;

public:
  cif_impl() {
    ffi_call(ffi_prep_cif, "ffi_prep_cif") //
        (/* cif = */ &m_raw,
         /* abi = */ FFI_DEFAULT_ABI,
         /* nargs = */ m_arg_type_list.size(),
         /* rtype = */ m_return_type.raw(),
         /* atypes = */ m_arg_type_list.data());
  }

  [[nodiscard]] constexpr auto raw() noexcept -> ffi_cif * { return &m_raw; }
};

template <typename call_signature>
using cif = cif_impl<call_signature, typename call_signature::arg_types>;

} // namespace voidstar::detail::ffi

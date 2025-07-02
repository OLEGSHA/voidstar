// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_DETAIL_FFI_CLOSURE_H
#define VOIDSTAR_DETAIL_FFI_CLOSURE_H

#include <voidstar/detail/ffi/cif.h>
#include <voidstar/detail/misc.h>

#include <ffi.h>

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>

namespace voidstar::detail::ffi {

/// @brief A RAII wrapper for a `ffi_closure`.
class closure {
private:
  /// @brief Type-erased function pointer to the trampoline.
  void *m_executable_ptr;

  struct deleter {
    void operator()(ffi_closure *writable) const noexcept {
      ffi_closure_free(writable);
    }
  };

  std::unique_ptr<ffi_closure, deleter> m_raw;

public:
  closure()
      : m_executable_ptr{nullptr},
        m_raw{
            static_cast<ffi_closure *>(
                ffi_closure_alloc(sizeof(ffi_closure), &m_executable_ptr)),
        } {
    if (m_raw == nullptr or m_executable_ptr == nullptr) {
      throw ffi::error{"Could not allocate an FFI closure"};
    }
  }

  /// @brief Get type-erased function pointer to the trampoline.
  [[nodiscard]] auto executable_ptr() const noexcept -> void * {
    return m_executable_ptr;
  };

  /// @brief Pointer to underlying `ffi_closure` struct.
  [[nodiscard]] auto raw() const noexcept -> ffi_closure * {
    return m_raw.get();
  };
};

/**
 * @brief A RAII wrapper for a prepared `ffi_closure` and referenced objects.
 *
 * @tparam call_signature A detail::call_signature describing the call signature
 * of the trampoline.
 * @tparam derived A CRTP parameter; must have an `m_payload` that
 * `detail::matches` @a call_signature.
 */
template <typename call_signature, typename derived> class prepared_closure {
private:
  cif<call_signature> m_cif;
  closure m_closure;

  [[no_unique_address]] pin m_pin; // `this` is baked into the closure

public:
  prepared_closure() {
    ffi::call(ffi_prep_closure_loc, "ffi_prep_closure_loc") //
        (/* closure = */ m_closure.raw(),
         /* cif = */ m_cif.raw(),
         /* fun = */ entrypoint,
         /* user_data = */ this,
         /* codeloc = */ m_closure.executable_ptr());
  }

private:
  using return_type = typename call_signature::return_type;
  static constexpr bool is_void = std::same_as<return_type, void>;
  using arg_types = typename call_signature::arg_types;
  static constexpr std::size_t arg_count = call_signature::arg_count;

  /**
   * @brief Invoke `derived::m_payload` with arguments from @a args and forward
   * its return value, if any.
   *
   * @param args A pointer to an array of #arg_count pointers to individual
   * argument values of types from @a call_signature.
   *
   * @return For non-void call signatures, the return value from the payload
   * coerced into call signature's return type.
   */
  auto call(void **args) -> return_type {
    return with_indices_zero_thru<arg_count>([&](auto... i) {
      return std::invoke(
          static_cast<derived *>(this)->m_payload,
          *static_cast<std::tuple_element_t<i, arg_types> *>(args[i])...);
    });
  }

  /**
   * @brief Called by libffi from within the trampoline.
   */
  static void entrypoint(ffi_cif *cif, void *ret, void **args,
                         void *user_data) {

    if (cif == nullptr or user_data == nullptr) {
      return;
    }

    if (not is_void and ret == nullptr) {
      return;
    }

    if (arg_count > 0 and args == nullptr) {
      return;
    }

    auto *const self = static_cast<prepared_closure *>(user_data);

    if constexpr (is_void) {
      (void)self->call(args);
      return;

    } else if constexpr (std::integral<return_type> and
                         sizeof(return_type) < sizeof(ffi_arg)) {
      // Workaround required by libffi, see documentation for ffi_call
      using widened_return_type =
          std::conditional_t<std::is_signed_v<return_type>, ffi_sarg, ffi_arg>;

      static_assert(alignof(widened_return_type) >= alignof(return_type),
                    "Overaligned integral return types are not supported");

      auto *const ret_typed = static_cast<widened_return_type *>(ret);
      *ret_typed = static_cast<widened_return_type>(self->call(args));
      return;

    } else {
      auto *const ret_typed = static_cast<return_type *>(ret);
      *ret_typed = static_cast<return_type>(self->call(args));
      return;
    }
  }

public:
  /// @brief Pointer-to-function type of this closure.
  using fn_ptr_type = typename call_signature::fn_ptr_type;

  /// @brief Obtain a function pointer to the trampoline.
  [[nodiscard]] auto get() const noexcept -> fn_ptr_type {
    return reinterpret_cast<fn_ptr_type>(m_closure.executable_ptr());
  }
};

} // namespace voidstar::detail::ffi

#endif

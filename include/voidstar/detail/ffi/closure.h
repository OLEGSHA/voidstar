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

class closure {
private:
  struct deleter {
    void operator()(ffi_closure *writable) const noexcept {
      ffi_closure_free(writable);
    }
  };

  void *m_executable_ptr;
  std::unique_ptr<ffi_closure, deleter> m_raw;

public:
  closure()
      : m_executable_ptr{nullptr},
        m_raw{
            static_cast<ffi_closure *>(
                ffi_closure_alloc(sizeof(ffi_closure), &m_executable_ptr)),
        } {
    if (m_raw == nullptr or m_executable_ptr == nullptr) {
      throw ffi_error{"Could not allocate an FFI closure"};
    }
  }

  [[nodiscard]] auto executable_ptr() const noexcept -> void * {
    return m_executable_ptr;
  };

  [[nodiscard]] auto raw() const noexcept -> ffi_closure * {
    return m_raw.get();
  };
};

template <typename call_signature> class basic_prepared_closure {
private:
  cif<call_signature> m_cif;
  closure m_closure;

  [[no_unique_address]] pin m_pin; // `this` is baked into the closure

protected:
  basic_prepared_closure(void (*entrypoint)(ffi_cif *cif, void *ret,
                                            void **args, void *user_data)) {
    ffi_call(ffi_prep_closure_loc, "ffi_prep_closure_loc") //
        (/* closure = */ m_closure.raw(),
         /* cif = */ m_cif.raw(),
         /* fun = */ entrypoint,
         /* user_data = */ this,
         /* codeloc = */ m_closure.executable_ptr());
  }

public:
  using fn_ptr_type = typename call_signature::fn_ptr_type;

  [[nodiscard]] auto get() const noexcept -> fn_ptr_type {
    return reinterpret_cast<fn_ptr_type>(m_closure.executable_ptr());
  }
};

template <typename call_signature, typename P>
class prepared_closure : public basic_prepared_closure<call_signature> {
public:
  P payload;

private:
  using basic_type = basic_prepared_closure<call_signature>;

public:
  template <typename... A>
  prepared_closure(A &&...args)
      : basic_type{&prepared_closure::entrypoint}, //
        payload{std::forward<A>(args)...} {}

private:
  using return_type = typename call_signature::return_type;
  static constexpr bool is_void = std::same_as<return_type, void>;
  using arg_types = typename call_signature::arg_types;
  static constexpr std::size_t arg_count = call_signature::arg_count;

  auto call(void **args) -> return_type {
    return with_indices_zero_thru<arg_count>([&](auto... i) {
      return std::invoke(
          payload,
          *static_cast<std::tuple_element_t<i, arg_types> *>(args[i])...);
    });
  }

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

    auto *const self =
        static_cast<prepared_closure *>(static_cast<basic_type *>(user_data));

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
};

} // namespace voidstar::detail::ffi

#endif

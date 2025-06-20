#pragma once

#include <voidstar/detail/call_signature.h>
#include <voidstar/detail/ffi/closure.h>

#include <memory>
#include <utility>

namespace voidstar {

namespace detail {

template <typename C, matches<C> P> class closure_impl {
public:
  using call_signature = C;
  using payload_type = P;

private:
  detail::ffi::prepared_closure<call_signature, payload_type> m_raw;

public:
  using fn_ptr_type = typename decltype(m_raw)::fn_ptr_type;

public:
  template <typename... A>
  requires std::constructible_from<P, A...> closure_impl(A &&...args)
      : m_raw{std::forward<A>(args)...} {}

  closure_impl(closure_impl const &) = delete;
  auto operator=(closure_impl const &) -> closure_impl & = delete;

  closure_impl(closure_impl &&) = delete;
  auto operator=(closure_impl &&) -> closure_impl & = delete;

  [[nodiscard]] auto get() const noexcept -> fn_ptr_type { return m_raw.get(); }

  operator fn_ptr_type() const noexcept { return get(); }

  [[nodiscard]] auto payload() noexcept -> payload_type & {
    return m_raw.payload;
  }

  [[nodiscard]] auto payload() const noexcept -> payload_type const & {
    return m_raw.payload;
  }
};

} // namespace detail

template <typename F, detail::matches<detail::call_signature<F>> P>
using closure = detail::closure_impl<detail::call_signature<F>, P>;

template <typename F, detail::matches<detail::call_signature<F>> P>
auto make_closure(P payload) -> closure<F, P> {
  return closure<F, P>{std::move(payload)};
}

} // namespace voidstar

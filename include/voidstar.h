#pragma once

#include <voidstar/call_signature.h>
#include <voidstar/detail/ffi/closure.h>

#include <memory>
#include <utility>

namespace voidstar {

namespace detail {

template <typename C, matches<C> P> class closure_impl {
public:
  using call_signature = C;
  using fn_ptr_type = typename call_signature::fn_ptr_type;
  using payload = P;

private:
  detail::ffi::prepared_closure<call_signature, payload> m_raw;

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
};

} // namespace detail

template <typename F, matches<call_signature<F>> P>
using closure = detail::closure_impl<call_signature<F>, P>;

template <typename F, matches<call_signature<F>> P>
auto make_closure(P payload) -> closure<F, P> {
  return closure<F, P>{std::move(payload)};
}

} // namespace voidstar

#ifndef VOIDSTAR_H
#define VOIDSTAR_H

#include <voidstar/detail/call_signature.h>
#include <voidstar/detail/ffi/closure.h>

#include <memory>
#include <utility>

namespace voidstar {

namespace detail {

template <typename C, matches<C> P>
class closure_impl
    : private detail::ffi::prepared_closure<C, closure_impl<C, P>> {
private:
  using base = detail::ffi::prepared_closure<C, closure_impl<C, P>>;
  friend base;

public:
  using call_signature = C;
  using payload_type = P;

protected:
  payload_type m_payload;

public:
  using typename base::fn_ptr_type;

  template <typename... A>
  requires std::constructible_from<P, A...> closure_impl(A &&...args)
      : m_payload{std::forward<A>(args)...} {}

  closure_impl(closure_impl const &) = delete;
  auto operator=(closure_impl const &) -> closure_impl & = delete;

  closure_impl(closure_impl &&) = delete;
  auto operator=(closure_impl &&) -> closure_impl & = delete;

  using base::get;

  operator fn_ptr_type() const noexcept { return get(); }

  [[nodiscard]] auto payload() noexcept -> payload_type & { //
    return m_payload;
  }

  [[nodiscard]] auto payload() const noexcept -> payload_type const & {
    return m_payload;
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

#endif

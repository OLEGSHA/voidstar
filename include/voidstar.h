// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_H
#define VOIDSTAR_H

#include <voidstar/detail/call_signature.h>
#include <voidstar/detail/ffi/closure.h>

#include <memory>
#include <utility>

namespace voidstar {

namespace detail {

/**
 * @brief Implementation of voidstar::closure - a prepared FFI closure and the
 * payload.
 *
 * @tparam C A `voidstar::detail::call_signature` struct describing the call
 * signature of the trampoline.
 *
 * @tparam P User payload.
 */
template <typename C, matches<C> P>
class closure_impl
    : private detail::ffi::prepared_closure<C, closure_impl<C, P>> {
private:
  using base = detail::ffi::prepared_closure<C, closure_impl<C, P>>;
  friend base;

public:
  /**
   * @brief Trampoline call signature.
   *
   * This is an implementation detail of voidstar. Do not use to ensure
   * backwards compatibility.
   */
  using call_signature = C;

  /// @brief Type of the payload object.
  using payload_type = P;

protected:
  /**
   * @brief The object to invoke in the trampoline.
   *
   * This field is referenced by `prepared_closure` via CRTP.
   */
  payload_type m_payload;

public:
  /**
   * @brief Type of the function pointer to the generated C function.
   */
  using typename base::fn_ptr_type;

  /**
   * @brief Prepare a trampoline and construct a payload using @a args.
   *
   * @param args The arguments to forward into the payload constructor call.
   *
   * @throws Any exception thrown by the payload constructor.
   * @throws voidstar::error - if the C function could not be generated. Payload
   * construction is skipped in this case.
   */
  template <typename... A>
  requires std::constructible_from<P, A...> closure_impl(A &&...args)
      : m_payload{std::forward<A>(args)...} {}

  /// @brief Closures are not copyable.
  closure_impl(closure_impl const &) = delete;

  /// @brief Closures are not copyable.
  auto operator=(closure_impl const &) -> closure_impl & = delete;

  /// @brief Closures are not movable.
  closure_impl(closure_impl &&) = delete;

  /// @brief Closures are not movable.
  auto operator=(closure_impl &&) -> closure_impl & = delete;

  /**
   * @brief Obtain a function pointer to the dynamically generated trampoline
   * for this closure.
   */
  using base::get;

  /**
   * @brief Obtain a function pointer to the dynamically generated trampoline
   * for this closure.
   */
  operator fn_ptr_type() const noexcept { return get(); }

  /**
   * @brief Get a mutable reference to the payload object of this closure.
   */
  [[nodiscard]] auto payload() noexcept -> payload_type & { //
    return m_payload;
  }

  /**
   * @brief Get a const reference to the payload object of this closure.
   */
  [[nodiscard]] auto payload() const noexcept -> payload_type const & {
    return m_payload;
  }
};

} // namespace detail

/**
 * @brief A closure -- a wrapper around callable @a P that has a unique C
 * function pointer.
 *
 * Contains an instance of @a P and manages the lifetime of a dynamically
 * generated function, a @a trampoline, that invokes @a P when called.
 *
 * @tparam F The desired call signature of the trampoline; either a function
 * type or a pointer to function type.
 *
 * @tparam P A user-provided callable payload that should be invoked by the
 * trampoline. `std::invoke(payload, F-args...)` must be valid and the result
 * must be convertible to the return type of @a F.
 *
 * @since 1.0.0
 */
template <typename F, detail::matches<detail::call_signature<F>> P>
using closure = detail::closure_impl<detail::call_signature<F>, P>;

/**
 * @brief Constructs a new [closure](#closure) deducing the payload type
 * automatically, useful for lambdas.
 *
 * `voidstar::make_closure<F>(x)` is roughly equivalent to
 * `voidstar::closure<F, decltype(x)>{x}`. It is purely a convenience function.
 *
 * Deduction of @a F is not supported.
 *
 * @tparam F The desired call signature of the trampoline; either a function
 * type or a pointer to function type.
 *
 * @tparam P A user-provided move-constructible callable payload that should be
 * invoked by the trampoline. `std::invoke(payload, F-args...)` must be valid
 * and the result must be convertible to the return type of @a F. Deduced from
 * argument.
 *
 * @param payload The object to invoke through the C function pointer. It is
 * moved into the closure.
 *
 * @return Initialized closure containing a move-constructed payload.
 *
 * @since 1.0.0
 */
template <typename F, detail::matches<detail::call_signature<F>> P>
auto make_closure(P payload) -> closure<F, P> {
  return closure<F, P>{std::move(payload)};
}

} // namespace voidstar

#endif

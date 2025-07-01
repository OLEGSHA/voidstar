// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#include <voidstar.h>

#include <type_traits>

namespace voidstar::test {
namespace {

////////////////////////////////////////////////////////////////////////////////
// Type properties
//

// clang-format off
template <typename T, typename P>
concept correct_closure = //
  std::is_destructible_v<P>
  and std::is_default_constructible_v<T> == std::is_default_constructible_v<P>
  and not std::is_copy_constructible_v<T>
  and not std::is_copy_assignable_v<T>
  and not std::is_move_constructible_v<T>
  and not std::is_move_assignable_v<T>

  and requires {
    typename T::call_signature;
    typename T::fn_ptr_type;
    typename T::payload_type;
  }

  and std::same_as<typename T::payload_type, P>
  and detail::matches<P, typename T::call_signature>

  and std::is_pointer_v<typename T::fn_ptr_type>
  and std::is_function_v<std::remove_pointer_t<typename T::fn_ptr_type>>

  and requires(T closure, typename T::fn_ptr_type& fn_ptr_out) {
    {closure.get()} -> std::convertible_to<typename T::fn_ptr_type>;
    {fn_ptr_out = closure};
  };
// clang-format on

////////////////////////////////////////////////////////////////////////////////
// Substitutions
//

template <typename F, typename P>
concept closure_valid = correct_closure<closure<F, P>, P>;

template <typename F, typename P>
concept closure_invalid = not requires {
  typename closure<F, P>;
};

// Trivial
static_assert(closure_valid<void(), decltype([] {})>);

// Trivial with argument
static_assert(closure_valid<void(int), decltype([](int) {})>);

// Multiple arguments
static_assert(closure_valid<void(int, float *, double),
                            decltype([](int, float *, double) {})>);
static_assert(closure_valid<void(int, int, int), //
                            decltype([](int, int, int) {})>);
static_assert(closure_invalid<void(int, float *), //
                              decltype([](int, float *, double) {})>);
static_assert(closure_invalid<void(int, float *, double), //
                              decltype([](int, float *) {})>);

// Pointer arguments
static_assert(closure_valid<void(int *), decltype([](int *) {})>);
static_assert(closure_valid<void(void *), decltype([](void *) {})>);
static_assert(closure_valid<void(int ***), decltype([](int ***) {})>);

// Constness in pointer arguments
static_assert(closure_invalid<void(int const *), decltype([](int *) {})>);
static_assert(closure_valid<void(int *), decltype([](int const *) {})>);
static_assert(closure_valid<void(int const *), decltype([](int const *) {})>);

// Volatility in pointer arguments
static_assert(closure_invalid<void(int volatile *), decltype([](int *) {})>);
static_assert(closure_valid<void(int *), decltype([](int volatile *) {})>);
static_assert(closure_valid<void(int volatile *), //
                            decltype([](int volatile *) {})>);
static_assert(closure_valid<void(int const volatile *),
                            decltype([](int const volatile *) {})>);

// Compatible arguments
static_assert(
    closure_valid<void(std::uint8_t), decltype([](std::uint64_t) {})>);
static_assert(
    closure_valid<void(std::uint64_t), decltype([](std::uint8_t) {})>);
static_assert( //
    closure_valid<void(int ***), decltype([](void const *) {})>);
static_assert( //
    closure_invalid<void(int), decltype([](int *) {})>);

// Array argument
static_assert(closure_valid<void(int[10]), decltype([](int[10]) {})>);
static_assert(closure_invalid<void(int[10]), decltype([](float[10]) {})>);
static_assert(closure_invalid<void(int[10]), decltype([](int) {})>);

// Array decay
static_assert(closure_valid<void(int *), decltype([](int[10]) {})>);
static_assert(closure_valid<void(int[10]), decltype([](int *) {})>);
static_assert(closure_valid<void(int[10]), decltype([](int[5]) {})>);

// Unbounded array argument
static_assert(closure_valid<void(int[]), decltype([](int[]) {})>);
static_assert(closure_valid<void(int *), decltype([](int[]) {})>);
static_assert(closure_valid<void(int[]), decltype([](int *) {})>);
static_assert(closure_valid<void(int[10]), decltype([](int[]) {})>);
static_assert(closure_valid<void(int[]), decltype([](int[10]) {})>);

// Deductions, etc.
static_assert(closure_valid<void(), decltype([](int = 10) {})>);
static_assert(closure_valid<void(), decltype([](...) {})>);
static_assert(closure_valid<void(int), decltype([](auto) {})>);
static_assert(closure_invalid<void(), decltype([](auto) {})>);
static_assert(
    closure_invalid<void(float), decltype([](std::integral auto) {})>);

// Return types
static_assert(closure_valid<int(), decltype([] { return int{}; })>);
static_assert(closure_invalid<int(), decltype([] {})>);
static_assert(closure_invalid<void(), decltype([] { return int{}; })>);

constexpr int *null_intp = nullptr;
static_assert(closure_valid<int *(), decltype([] { return null_intp; })>);
static_assert(closure_valid<int const *(), decltype([] { return null_intp; })>);
static_assert(
    closure_valid<std::uint8_t(), decltype([] { return std::uint8_t{}; })>);
static_assert(
    closure_valid<std::uint64_t(), decltype([] { return std::uint64_t{}; })>);

// Compatible return type
static_assert(
    closure_valid<std::uint8_t(), decltype([] { return std::uint64_t{}; })>);
static_assert(
    closure_valid<std::uint64_t(), decltype([] { return std::uint8_t{}; })>);
static_assert(closure_invalid<int *(), decltype([] { return int{}; })>);

// Other callables
static_assert(closure_valid<void(), void (*)()>);
static_assert(closure_valid<void(), std::function<void()>>);

struct minimal_callable_vi {
  virtual void operator()(int);

  minimal_callable_vi(minimal_callable_vi const &) = delete;
  auto operator=(minimal_callable_vi const &) -> minimal_callable_vi & = delete;
  minimal_callable_vi(minimal_callable_vi &&) = delete;
  auto operator=(minimal_callable_vi &&) -> minimal_callable_vi & = delete;
};

static_assert(closure_valid<void(int), minimal_callable_vi>);

// Bad callables
static_assert(closure_invalid<void(), int>);
static_assert(closure_invalid<void(), void>);
static_assert(closure_invalid<void(), int (*)()>);
static_assert(closure_invalid<void(), void (*)(int)>);

// Bad function signatures
static_assert(closure_invalid<void, void (*)()>);
static_assert(closure_invalid<int, void (*)()>);
static_assert(closure_invalid<int, int>);

// Other function signatures
static_assert(closure_valid<void (*)(), decltype([] {})>);

extern "C" {
using c_fn_vv = void();
using c_fn_ptr_vv = void (*)();
}

static_assert(closure_valid<c_fn_vv, decltype([] {})>);
static_assert(closure_valid<c_fn_ptr_vv, decltype([] {})>);

} // namespace
} // namespace voidstar::test

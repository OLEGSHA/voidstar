#include <gtest/gtest.h>

#include <voidstar.h>

#include <future>
#include <list>
#include <memory>
#include <thread>
#include <type_traits>

namespace voidstar::test {
namespace {

struct struct_wrapped_int {
  int x;
  auto operator==(struct_wrapped_int const &) const -> bool = default;
};

struct struct_wrapped_float {
  float x;
  auto operator==(struct_wrapped_float const &) const -> bool = default;
};

struct struct_wrapped_char {
  char x;
  auto operator==(struct_wrapped_char const &) const -> bool = default;
};

struct struct_wrapped_ptr {
  int *x;
  auto operator==(struct_wrapped_ptr const &) const -> bool = default;
};

struct struct_simple {
  int x;
  float y;

  auto operator==(struct_simple const &) const -> bool = default;
};

struct struct_empty {
  auto operator==(struct_empty const &) const -> bool = default;
};

struct struct_with_array {
  int x;
  float y[5];

  auto operator==(struct_with_array const &) const -> bool = default;
};

struct struct_composite {
  struct_simple a;
  struct_with_array b;

  auto operator==(struct_composite const &) const -> bool = default;
};

struct struct_with_array_of_structs {
  struct_simple x[3];

  auto operator==(struct_with_array_of_structs const &) const -> bool = default;
};

} // namespace
} // namespace voidstar::test

template <> struct voidstar::layout<voidstar::test::struct_wrapped_int> {
  using members = std::tuple<int>;
};

template <> struct voidstar::layout<voidstar::test::struct_wrapped_float> {
  using members = std::tuple<float>;
};

template <> struct voidstar::layout<voidstar::test::struct_wrapped_char> {
  using members = std::tuple<char>;
};

template <> struct voidstar::layout<voidstar::test::struct_wrapped_ptr> {
  using members = std::tuple<int *>;
};

template <> struct voidstar::layout<voidstar::test::struct_simple> {
  using members = std::tuple<int, float>;
};

template <> struct voidstar::layout<voidstar::test::struct_empty> {
  using members = std::tuple<>;
};

template <> struct voidstar::layout<voidstar::test::struct_with_array> {
  using members = std::tuple<int, float[5]>;
};

template <> struct voidstar::layout<voidstar::test::struct_composite> {
  using members = std::tuple<voidstar::test::struct_simple,
                             voidstar::test::struct_with_array>;
};

template <>
struct voidstar::layout<voidstar::test::struct_with_array_of_structs> {
  using members = std::tuple<voidstar::test::struct_simple[3]>;
};

namespace voidstar::test {
namespace {

template <auto val> struct param {
  using type = decltype(val);
  static constexpr type value = val;
};

// Clang <19 doesn't support float NTTPs
template <std::floating_point T> struct float_param {
  using type = T;
  static constexpr type value = 4.2;
};

template <typename T> struct TypeSupport : testing::Test {
  using param_type = typename T::type;
  static constexpr param_type param_value = T::value;
};

int example_variable;
void example_function() {}

// clang-format off
using TestCases = testing::Types<
    param<static_cast<short>(-10)>,
    param<static_cast<unsigned short>(10)>,
    param<static_cast<int>(-10)>,
    param<static_cast<unsigned int>(10)>,
    param<static_cast<long>(-10)>,
    param<static_cast<unsigned long>(10)>,
    param<static_cast<long long>(-10)>,
    param<static_cast<unsigned long long>(10)>,

    param<static_cast<char>(10)>,
    param<static_cast<signed char>(-10)>,
    param<static_cast<unsigned char>(10)>,
    param<static_cast<wchar_t>(10)>,
    param<static_cast<char8_t>(10)>,
    param<static_cast<char16_t>(10)>,
    param<static_cast<char32_t>(10)>,

    param<true>,
    param<false>,

    float_param<float>,
    float_param<double>,
    float_param<long double>,

    param<&example_variable>,
    param<&example_function>,

    param<struct_wrapped_int{-10}>,
    param<struct_wrapped_float{4.2f}>,
    param<struct_wrapped_char{10}>,
    param<struct_simple{.x = -10, .y = 4.2f}>,
    param<struct_empty{}>,
    param<struct_with_array{.x = -10, .y = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f}}>,
    param<struct_composite{
        .a = {.x = -10, .y = 4.2f},
        .b = {.x = -20, .y = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f}},
    }>,
    param<struct_with_array_of_structs{.x = {
        {.x = -10, .y = 1.1f},
        {.x = -20, .y = 2.2f},
        {.x = -30, .y = 3.3f},
    }}>
>;
// clang-format on

TYPED_TEST_SUITE(TypeSupport, TestCases);

TYPED_TEST(TypeSupport, SanityCheck) {
  EXPECT_EQ(TestFixture::param_value, TestFixture::param_value);
}

TYPED_TEST(TypeSupport, AsArgumentViaRegisters) {
  using type = typename TestFixture::param_type;

  make_closure<void(type)>([](type val) {
    EXPECT_EQ(val, TestFixture::param_value);
  }).get()(TestFixture::param_value);
}

#define VOIDSTAR_8_TIMES(...)                                                  \
  __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__,             \
      __VA_ARGS__, __VA_ARGS__, __VA_ARGS__

TYPED_TEST(TypeSupport, AsArgumentOnStack) {
  // Good luck passing it via registers when the signature is
  // void(/* `int, float` x16 */, param_type, /* `int, float` x16 */)
  //
  // Using a mix of ints and floats to pollute both normal and float registers.

  using type = typename TestFixture::param_type;

  make_closure<void(VOIDSTAR_8_TIMES(int, float), type,
                    VOIDSTAR_8_TIMES(int, float))>(
      [](VOIDSTAR_8_TIMES(int, float), type val, VOIDSTAR_8_TIMES(int, float)) {
        EXPECT_EQ(val, TestFixture::param_value);
      })
      .get()(VOIDSTAR_8_TIMES(0, 0.0f), TestFixture::param_value,
             VOIDSTAR_8_TIMES(0, 0.0f));
}

TYPED_TEST(TypeSupport, AsReturnValue) {
  using type = typename TestFixture::param_type;

  type result =
      make_closure<type()>([]() { return TestFixture::param_value; }).get()();

  EXPECT_EQ(result, TestFixture::param_value);
}

} // namespace
} // namespace voidstar::test

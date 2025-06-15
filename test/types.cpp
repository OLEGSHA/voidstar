#include <gtest/gtest.h>

#include <voidstar.h>

#include <future>
#include <list>
#include <memory>
#include <thread>
#include <type_traits>

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
    param<(short){-10}>,
    param<(unsigned short){10}>,
    param<(int){-10}>,
    param<(unsigned int){10}>,
    param<(long){-10}>,
    param<(unsigned long){10}>,
    param<(long long){-10}>,
    param<(unsigned long long){10}>,

    param<(char){10}>,
    param<(signed char){-10}>,
    param<(unsigned char){10}>,
    param<(wchar_t){10}>,
    param<(char8_t){10}>,
    param<(char16_t){10}>,
    param<(char32_t){10}>,

    param<true>,
    param<false>,

    float_param<float>,
    float_param<double>,
    float_param<long double>,

    param<&example_variable>,
    param<&example_function>
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

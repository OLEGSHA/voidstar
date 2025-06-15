#include <gtest/gtest.h>

#include <voidstar.h>

#include <future>
#include <list>
#include <memory>
#include <thread>
#include <type_traits>

namespace voidstar::test {
namespace {

TEST(Closure, Lifetime) {
  struct payload {
    void operator()() {}
  };
  closure<void(), payload> object;
  (void)object;
}

TEST(Closure, PayloadLifetime) {
  struct snowflake {};

  int value = 0;

  struct payload {
    int &value;
    payload(int &value, snowflake) : value{value} { value++; }
    ~payload() { value++; }
    void operator()() {}
  };

  EXPECT_EQ(value, 0);
  {
    closure<void(), payload> object{value, snowflake{}};
    EXPECT_EQ(value, 1);
  }
  EXPECT_EQ(value, 2);
}

TEST(Closure, SimpleCall) {
  int calls = 0;

  struct payload {
    int &calls;
    payload(int &calls) : calls{calls} {}
    void operator()() { calls++; }
  };

  EXPECT_EQ(calls, 0);
  closure<void(), payload> cls{calls};
  EXPECT_EQ(calls, 0);
  cls.get()();
  EXPECT_EQ(calls, 1);
}

TEST(Closure, MakeClosure) {
  int calls = 0;

  EXPECT_EQ(calls, 0);
  auto cls = make_closure<void()>([&] { calls++; });
  EXPECT_EQ(calls, 0);
  cls.get()();
  EXPECT_EQ(calls, 1);
}

TEST(Closure, ManyClosures) {
  constexpr std::size_t N = 1000;

  std::array<int, N> targets{};

  auto make_payload = [&](int i) {
    return [i, &targets] { targets.at(i) = i; };
  };
  using cls_t = closure<void(), decltype(make_payload(0))>;

  std::list<cls_t> clses;
  for (std::size_t i = 0; i < N; i++) {
    clses.emplace_back(make_payload(i));
  }

  for (std::size_t i = 0; i < N; i++) {
    EXPECT_EQ(targets[i], 0);
  }

  for (auto const &cls : clses) {
    cls.get()();
  }

  for (std::size_t i = 0; i < N; i++) {
    EXPECT_EQ(targets[i], i);
  }
}

TEST(Closure, NoThreadLocality) {
  // Ensure that thread of origin and the thread of use are not important

  // std::jthread instead of std::async or similar to guarantee no thread reuse
  auto run_in_new_thread = [](auto fn) { (void)std::jthread{fn}; };

  int calls = 0;

  auto payload = [&] { calls++; };
  using cls_t = closure<void(), decltype(payload)>;
  std::unique_ptr<cls_t> cls;

  run_in_new_thread([&] { cls = std::make_unique<cls_t>(payload); });

  EXPECT_EQ(calls, 0);
  run_in_new_thread([&] { cls->get()(); });
  EXPECT_EQ(calls, 1);
  run_in_new_thread([&] { cls->get()(); });
  EXPECT_EQ(calls, 2);
}

namespace type_support {

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

} // namespace type_support
} // namespace
} // namespace voidstar::test

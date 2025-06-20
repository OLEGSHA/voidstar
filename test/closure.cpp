#include <gtest/gtest.h>

#include <voidstar.h>

#include <future>
#include <list>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>

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

TEST(Closure, ImplicitCast) {
  int calls = 0;

  EXPECT_EQ(calls, 0);
  auto cls = make_closure<void()>([&] { calls++; });
  EXPECT_EQ(calls, 0);
  void (*ptr)() = cls;
  ptr();
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

TEST(Closure, PayloadGetter) {
  struct payload {
    void operator()() {}
    int value = 0;
  };

  closure<void(), payload> cls;

  EXPECT_EQ(cls.payload().value, 0);
  cls.payload().value = 42;
  EXPECT_EQ(std::as_const(cls).payload().value, 42);
}

} // namespace
} // namespace voidstar::test

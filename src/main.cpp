#include <iostream>
#include <libstupid.h>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <voidstar.h>
#include <voidstar/detail/ffi/closure.h>

template <typename T> std::string to_string(std::decay_t<T> ptr) {
  std::ostringstream oss;
  oss << "[";
  bool first = true;
  for (auto &e : std::span<std::remove_extent_t<T>, std::extent_v<T>>{
           ptr, std::extent_v<T>}) {
    if (not std::exchange(first, false)) {
      oss << ", ";
    }
    if constexpr (std::is_array_v<std::remove_extent_t<T>>) {
      oss << to_string<std::remove_extent_t<T>>(e);
    } else {
      oss << e;
    }
  }
  oss << "]";
  return oss.str();
}

int main() {

  struct cb {
    cb(std::string m) : msg{m} {};
    void operator()() { std::cout << "I am " << msg << std::endl; }
    std::string msg;
  };

  voidstar::closure<libstupid_cb, cb> const m1{"Alice"};
  voidstar::closure<libstupid_cb, cb> const m2{"Bob"};
  auto const m3 = voidstar::make_closure<libstupid_cb>(cb{"char *lie"});

  std::string m4_msg = "Delta";
  auto const m4 = voidstar::make_closure<libstupid_cb>([&]() { cb{m4_msg}(); });

  std::string m5_msg = "EPSILON";
  auto m5 = voidstar::make_closure<libstupid_cb>([&]() { cb{m5_msg}(); });

  libstupid_call_me(m1, m2);
  libstupid_call_me(m2, m3);
  libstupid_call_me(m4, m5);
  libstupid_call_me(m4, voidstar::make_closure<libstupid_cb>([] {}));

  struct compukter {
    compukter(std::string m) : msg{m} {};
    int operator()(float a, int b, double c) {
      std::cout << "Compukter " << msg << " called with " << a << ", " << b
                << ", " << c << std::endl;
      return 100;
    }
    std::string msg;
  };

  voidstar::closure<libstupid_computer *, compukter> comp{"IBM9000"};

  int res = libstupid_compute(comp);
  std::cout << "libstupid_compute() -> " << res << std::endl;

  struct vectortron_500 {
    auto operator()(arr_1 a, arr_2 b, arr_3 c, arr_4 d) {
      std::cout << "vectortron 500: arr_1 = " << to_string<arr_1>(a) << "\n";
      std::cout << "vectortron 500: arr_2 = " << to_string<arr_2>(b) << "\n";
      std::cout << "vectortron 500: arr_3 = " << to_string<arr_3>(c) << "\n";
      std::cout << "vectortron 500: arr_4 = " << to_string<arr_4>(d) << "\n";

      std::cout << std::endl;
      return -45;
    }
  };

  voidstar::closure<vectortron, vectortron_500> const vtron;
  short res2 = vectorize(vtron);
  std::cout << "vectorize() -> " << res2 << std::endl;
}

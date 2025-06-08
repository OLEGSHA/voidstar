#include <libstupid.h>

extern "C" {

using libstupid_cb = void (*)();

void libstupid_call_me(libstupid_cb cb1, libstupid_cb cb2) {
  cb1();
  cb2();
}

using libstupid_computer = int(float, int, double);
int libstupid_compute(libstupid_computer *compukter) {
  return compukter(4.2, 2, 6.9) + 10;
}

short vectorize(vectortron v) {
  arr_1 a{1, 2, 3, -4, 5};
  arr_2 b{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 999.999};
  arr_3 c{{1.1, 1.2}, {2.1, 2.2}};
  arr_4 d{true};
  return v(a, b, c, d);
}
}

#pragma once

extern "C" {

using libstupid_cb = void (*)();

void libstupid_call_me(libstupid_cb cb1, libstupid_cb cb2);

using libstupid_computer = int(float, int, double);

int libstupid_compute(libstupid_computer *compukter);

using arr_1 = short[5];
using arr_2 = float[10];
using arr_3 = double[2][2];
using arr_4 = bool[1];

using vectortron = short (*)(arr_1, arr_2, arr_3, arr_4);

short vectorize(vectortron v);
}

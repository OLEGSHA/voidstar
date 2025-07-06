#include <voidstar.h>

#include <iostream>

int main() {
  try {
    int cls_param = 88;

    auto cls = voidstar::make_closure<int(int)>([&](int param) {
      cls_param = param;
      return -70;
    });

    int cls_result = cls.get()(42);

    if (cls_result != -70) {
      std::cout << "Bad result: expected -70, got " << cls_result << std::endl;
      return 1;
    }

    if (cls_param != 42) {
      std::cout << "Bad param: expected 42, got " << cls_param << std::endl;
      return 1;
    }

    std::cout << "Test passed" << std::endl;
    return 0;
  } catch (...) {
    std::cout << "An exception occurred" << std::endl;
    return 1;
  }
}

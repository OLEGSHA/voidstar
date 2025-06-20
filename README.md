# voidstar

A library to convert any C++ callable into a C function pointer using a dynamically generated [libffi](https://sourceware.org/libffi/) trampoline.

## How to use

```c++
#include <voidstar.h>

// Third-party library
extern "C" {
  typedef int (*example_callback)(float);
  void example_function(example_callback);
}

int main() {
  float param;
  int result = 42;

  auto closure =
    voidstar::make_closure<example_callback>([&](float p) {
      param = p;
      return result;
    });

  // No global or thread_local variables
  example_function(closure.get());

  // example_callback works until `closure` is destroyed
}
```

Check out a more complete [example](example/background_jobs/) or see [full library reference](docs/reference.md).

## Status

The library is implemented and tested on amd64 Linux with g++ 12 and clang++-14. It has not yet been used in a real production environment.

### TODO
- [x] Proper tests
- [x] Main doc
- [ ] Inline docs
- [ ] Conan & proper CMake
- [ ] Struct layout guessing via structured bindings
- [ ] Figure out what to do with type erasure
- [x] Example: background jobs
- [ ] Example: logging interceptor
- [ ] Example: difficult types
- [x] Header-only version?

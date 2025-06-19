# voidstar

A library to convert any C++ callable into a C function pointer using a dynamically generated [libffi](https://sourceware.org/libffi/) trampoline.

## Documentation

See [`docs/`](docs/) directory.

## Status

The library is implemented and tested on amd64 Linux with g++ 12 and clang++-14. It has not yet been used in a real production environment.

### TODO
- [x] Proper tests
- [ ] Main doc
- [ ] Inline docs
- [ ] Conan & proper CMake
- [ ] Struct layout guessing via structured bindings
- [ ] Figure out what to do with type erasure
- [x] Example: background jobs
- [ ] Example: logging interceptor
- [ ] Example: difficult types
- [x] Header-only version?

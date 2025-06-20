# voidstar Library Reference

## Scope and usage

voidstar provides a safe, backwards-compatible public API via identifiers in `voidstar` namespace, excluding `voidstar::detail` namespace.

Users of the library should include either `voidstar.h` to include all functionality, or `voidstar/layout.h` to only include the primary definition of the `voidstar::layout` template. Headers inside `voidstar/detail` are private.

### Inherent limitations

voidstar library relies on [libffi](https://sourceware.org/libffi/) to implement the bulk of its functionality, thus it is limited to the platforms and usage scenarios that are supported by libffi. Detection or workarounds for unsupported usage are provided on a best-effort basis; users are encouraged to check libffi availability and caveats on their target platform.

### Backwards compatibility

voidstar library uses [semantic versioning](https://semver.org/).

To ensure backwards compatibility, users must not rely on the nature or properties of public identifiers beyond those documented. For example, `voidstar::make_closure` is currently a function template, but it could be changed to a variable template of an invocable type in the future.

Similarly, instantiating templates with arguments that do not meet the documented requirements is undefined behavior. Compile-time error detection is done on a best-effort basis, and may become more permissive as new features are added.

## `voidstar::closure`

```c++
template <typename F, typename P>
requires is-function-specifier<F> &&
         is-invocable-as<P, F>
using closure = /* unspecified */;
```

A class template that contains a user-provided invocable _payload_ and provides a C function pointer that invokes the payload when called.

### Template parameters

- `typename F`: A function type such as `void(int, float)` or an cv-unqualified function pointer such as `void(*)(int, float)`. The function must have C linkage. The function must not be variadic. The return type of the function and the parameter types of the function must all be [supported](#type-support).

- `typename P`: The payload type. In simple terms, it must be invocable with call signature compatible with _F_.

  The payload must be destructable and it must meet the _is-invocable-as<P, F>_ requirement: for _F_ with return type _R_ and parameter types _A0_, ... _An_,
  ```c++
  requires(P& payload_mref,
           A0 const& a0, /* … */ An const& an) {
    { std::invoke(payload_mref, a0, /* … */ an) }
      -> std::convertible_to<R>;
  }
  ```
  must evaluate to `true`.

### Safety

> **Warning**
>
> Executing the C function of a closure while or after that closure is destroyed is undefined behavior. A closure object must be kept alive until it is certain that the last call to its C function has returned.

**Closure payloads must not attempt to destroy their host closure,** even through `std::shared_ptr`-like mechanics. There is no generic mechanism to detect that the caller has exited the generated function body, which might be significantly later than the return from payload body due to, for example, OS scheduler.

**Pay attention to temporary closure objects.** Usage such as `foo(voidstar::closure<F, P>().get())` is safe as long as `foo` does not keep the function pointer after returning, i.e. if it is guaranteed to call the function pointer during its execution (if at all). Registering callbacks in this way leads to undefined behavior. For the same reason, `return some_closure.get();` is analogous to returning a pointer to a local variable. In this sense, `voidstar::closure` can be compared to `std::unique_ptr<generated-function>`.

Each `voidstar::closure` instance owns an instance of _P_. This instance is mutable unless specified as `const P`. **Users are responsible for ensuring thread safety within the payload,** since the closure permits concurrent calls.

`voidstar::closure` is immutable after construction, so it is thread-safe. Its C function pointer is reentrant, supports recursion and may be called from any one or multiple threads.

### Notes

Each `voidstar::closure` instance owns a libffi `ffi_closure` object and `ffi_type` descriptions of all referenced types. The main job of `voidstar::closure` is generating type descriptions at compile time and providing a RAII-style, C++-friendly interface to libffi closure objects.

Currently, `voidstar::closure` is not copyable and it is not movable, but these restrictions may be lifted in the future.

### Constructor

```c++
template <typename... A>
requires std::constructible_from<P, A...>
closure(A&&... payload_args);
```

Allocates and prepares a libffi closure with call signature _F_, then creates an instance of _P_ with its `P(std::forward<A>(payload_args)...)` constructor.

In some circumstances, such as running out of memory, lacking necessary permissions from the OS, or a voidstar bug, libffi may fail to allocate or prepare the closure. In this event, an unspecified exception derived from `std::runtime_error` is thrown, and _P_ is not initialized.

If the constructor of _P_ throws, libffi resources are released, and the exception is propagated to the caller.

### Destructor

```c++
~closure()
noexcept(std::is_nothrow_destructible_v<P>);
```

Destroys the payload, then releases the libffi closure.

The last return from the C function body must _happen-before_ the invocation of this destructor; see _Safety_ section in [class description](#voidstarclosure).

### Type aliases

```c++
using fn_ptr_type = /* function pointer based on F */;
```

_F_ as a function pointer. _fn_ptr_type_ points to a function with C linkage.

```c++
using payload_type = P;
```

Payload type.

### C function pointer access

```c++
fn_ptr_type get() const noexcept;

/* implicit */
operator fn_ptr_type()
const noexcept;
```

Return a pointer to the generated C function that invokes the payload when called.

These are trivial getters with minimal, if any, overhead.

### Payload access

```c++
payload_type& payload() noexcept;
payload_type const& payload() const noexcept;
```

Return a reference to the payload instance within the closure. These are trivial getters with no overhead.

## `voidstar::make_closure`

```c++
template <typename F, typename P>
requires is-function-specifier<F> &&
         is-invocable-as<P, F> &&
         std::move_constructible<P>

closure<F, P>
make_closure(P payload);
```

Convenience factory function template that deduces the payload type of closures. Useful for movable lambdas and other move-constructible types.

`voidstar::make_closure<F>(payload)` is equivalent to `voidstar::closure<F, decltype(payload)>(payload)`.

Deduction of _F_ is not currently supported.

### Example

```c++
auto payload_a = [&] { return x; };
voidstar::closure<int(), decltype(payload_a)>
  closure_a{std::move(payload_a)};

auto closure_b = voidstar::make_closure<int()>([&] {
  return y;
});
```

## Type support

To generate trampoline functions at runtime, libffi requires a description of all types that make up the function signature. Calling conventions are complex and sometimes counterintuitive to developers accustomed to higher-level programming languages: for example, in x86_64 ABIs, `struct {int; float}` is passed differently from `struct {int; int}`, even though the sizes and alignments of the structs and their members are identical.

voidstar is able to deconstruct function signatures into argument types and return types at compile time to statically prepare these type descriptions. However, this mechanism and its current implementation have their limits.

Support can be extended to more types than listed below through [`voidstar::layout`](#voidstarlayout) specializations.

### Types with built-in support

Because voidstar targets C interfaces, built-in type support is mostly limited to types found in both C and C++.

- Standard C++20 fundamental types except `std::nullptr_t`.
- C99 `_Complex` fundamental types (when supported by compiler and libffi).
- Pointer-to-object types, including `void*`, including pointers to unsupported types.
- Pointer-to-function types.
- Bounded arrays of any length of any supported type, including multidimensional arrays.
- Enumerator types, including scoped and unscoped, including with and without fixed size.

Note that arrays in function parameters decay to pointers-to-objects. Dedicated bounded array support is required for member types only.

## `voidstar::layout`

```c++
template <typename T>
struct layout;
```

A class template that may be specialized in user code to make a type supported by voidstar.

### Usage

The template may be fully or partially specialized for any cv-unqualified type not already supported, as long as `sizeof(T)` and `alignof(T)` are valid for that type. If support for new types is added in a minor update, voidstar will prioritize user-provided layouts.

The specialization must define type alias `members`, which must be a `std::tuple`. Its contents correspond to `ffi_type::elements`; see below for most common use case. All specified types must be supported.

The specialization must not declare any other members to ensure compatibility with future versions.

> **Warning**
>
> voidstar will not attempt to validate provided layouts. Specifying a layout wrongly may lead to incorrect generated code on all or some platforms.

### Non-empty standard-layout non-union class types

voidstar guarantees correct behavior for non-empty standard-layout non-union class types. For such types, `members` must begin with the single base class if any, followed by the types of all non-static data members in definition order:

```c++
struct base { double d[2]; };

template <> struct voidstar::layout<base> {
  using members = std::tuple<double[2]>;
};

struct derived : base {
  int a;
  float b;
};

template <> struct voidstar::layout<derived> {
  using members = std::tuple<base, int, float>;
};
```

### Other types

For all other types, voidstar will provide the following `ffi_type` to libffi:

```c++
ffi_type{
  .size = sizeof(T),
  .alignment = alignof(T),
  .type = FFI_TYPE_STRUCT,
  .elements = /* ffi_types of layout::members */,
}
```

It it the responsibility of the user to verify that generated trampolines are correct.

### A note on union types

Unfotunately, the version of libffi that voidstar is designed against, 3.5, [does not provide dedicated union type support](https://github.com/libffi/libffi/issues/33). While workarounds exist, none of them can be applied blindly, and they may be limited to specific platforms.

Users should do their own research to find and verify union handling approaches that suit them.

As a starting point for that research, for many union types, it is possible to specify the largest union member as the sole element. `union{struct {char; char}; short}` is likely going to work, but `union {int; float}` will probably not.


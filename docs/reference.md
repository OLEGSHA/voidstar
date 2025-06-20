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
requires is-function-like<F> &&
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
fn_ptr_type
get()
const noexcept;
```

```c++
/* implicit */
operator fn_ptr_type()
const noexcept;
```

Return a pointer to the generated C function that invokes the payload when called.

These are trivial getters with minimal, if any, overhead.

## Type support

TODO

## `voidstar::layout`

TODO

## `voidstar::make_closure`

TODO

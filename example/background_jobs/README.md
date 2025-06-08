# "background_jobs" example for voidstar library

This example demonstrates a realistic yet simple usage scenario.

## Scenario

badlib is an example of a library with a poorly-designed C API. It allows executing some sort of lengthy job that produces a result. The jobs are started in the background, and badlib invokes a user-provided callback on completion.

Unfortunately, the callback function only receives the result value as input. Without a `void* user_data` pointer or some kind of globally-unique job identifier, the callback cannot access its context. Perhaps library authors expected a unique, hand-written callback function for each job?

In this example, a runtime-specified number of jobs must be started in parallel, and each result must be linked back to its job.

## badlib API summary

- `struct badlib_job { ...; void (*on_done)(double); }`: Describes a job that can be started. `on_done` will be invoked in some unknown library-managed thread with the result of the job (the `double`) once it completes.

- `void badlib_start_job(badlib_job)`: Begins execution of the job in the background, returning immediately. Stores the `on_done` pointer so it can be called later.

- `void badlib_join()`: Waits for all background jobs to complete normally, including the calls to the `on_done` callbacks.

## Commentary

See [main.cpp](main.cpp) for a complete, uninterrupted listing.

---

```c++
struct the_callback {
  int job_number;

  void operator()(double result) {
    std::cout << "Job #" << this->job_number
              << " completed with result " << result
              << std::endl;
  }
};
```

The callback logic is implemented in `the_callback::operator()`, while `the_callback::job_number` is the necessary context. voidstar helps make this context available as `this` in `operator()`.

Other than being a non-static instance method, `operator()` matches the function signature, `void(double)`, that badlib works with.

The same could be achieved with C++ lambdas. voidstar works well with lambdas, but being able to name the type of the callback is useful in this example.

---

```c++
// Shortcut for convenience
using closure = voidstar::closure<badlib_job_callback, the_callback>;
```

This `voidstar::closure` type contains an instance of `the_callback` and provides a unique C function pointer of type `badlib_job_callback`. Different instances of `closure` (and, therefore, different `the_callback` objects) get different function addresses. When called, these synthetic functions invoke `operator()` of the correct `the_callback` object. See main documentation for a more thorough explanation.

This way, by instantiating enough `closure` objects at runtime, it is possible to obtain arbitrarily many different C callbacks, each with its own context.

---

```c++
std::list<closure> closures;

for (int i = 0; i < jobs; i++) {
  closures.emplace_back(the_callback{i});

  closure &cls = closures.back();
  badlib_job_callback c_fn_ptr = cls.get();

  badlib_start_job(badlib_job{
      .param = /* ... */,
      .callback = c_fn_ptr,
  });
}
```

Piece by piece:

```c++
closure &cls = /* ... */;
badlib_job_callback c_fn_ptr = cls.get();

badlib_start_job(badlib_job{
    .param = /* ... */,
    .callback = c_fn_ptr,
});
```

Here, a badlib job is started in the background. The `on_done` callback is the dynamically generated trampoline managed by `voidstar::closure`, obtained via `.get()`. Closure objects are implicitly convertible to their target function pointer type, so the following would also work:

```c++
// Alternate style
badlib_start_job(badlib_job{
    .param = /* ... */,
    .callback = closures.back(),
});
```

The lifetime of `voidstar::closure` objects is important.

```c++
std::list<closure> closures;
// in a loop:
closures.emplace_back(the_callback{i});
```

The function pointer provided by `voidstar::closure` is only usable while its source object is alive. Attempting to invoke the dynamically generated function after the closure has been destroyed results in undefined behavior. For this reason, closure objects need to be stored until all jobs are done, that is, until `badlib_join()` returns.

Usage such as

```c++
// Potentially unsafe
foo(voidstar::make_closure<foo_cb>(/* ... */));
```

is safe only if `foo()` does not store the function pointer after `foo()` returns. The callback passed into `badlib_start_job()` is invoked much later.

`voidstar::closure` is not movable because the address of the payload is baked into the synthetic function, so a plain `std::vector<closure>` would not compile. `std::list` is the next best option. `emplace_back()` is used instead of `push_back()` for the same reason.

---

```c++
badlib_join();
closures.clear();
```

Each closure object can be destroyed when the last invocation of its payload returns. In this example, it is enough to wait for `badlib_join()`: it blocks user thread until all scheduled jobs are done. Importantly, it also guarantees that all callback executions are completed as well.

Note that in this scenario, it is impossible to safely destroy closures one by one. There is no mechanism to determine exactly when the calling thread returns out of the dynamically generated function.

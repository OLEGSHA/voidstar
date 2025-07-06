# How does voidstar work?

> **tl;dr**
>
> voidstar is a C++20 wrapper for [libffi](https://sourceware.org/libffi/) closure API. A libffi closure is a function with dynamically generated machine code. When called, it forwards the arguments and some assigned `void*` to a statically compiled function. voidstar uses that pointer to find and invoke the correct user object.

_See also: [voidstar API reference](reference.md)_

## Context

[Callbacks](https://en.wikipedia.org/wiki/Callback_(computer_programming)) are a common and necessary pattern in C APIs. Many higher-level concepts such as event listeners, error handlers, asynchronous operations and others often rely on callbacks.

Callbacks nearly always involve function pointers. In C and C++, function pointers are _thin_ - they are literally just an address in executable memory that a processor can jump to, almost always the start of a function with C linkage. Callback caller can thus call function pointers like functions - pass arguments to them, execute a CALL instruction, and expect to receive control back together with some return value.

To a callback that was called, the following information is available:
- its function arguments,
- global variables it knows about,
- thread-local variables it knows about.

If the callback doesn't need any context to do its job, than there are no complications. Many callbacks, though, require access to various objects.

### The problem

For example, a GUI textbox listener may need to check its textbox object to get full text contents. How can the listener get this pointer without any kind of captures or pre-existing local variables?

The correct answer in almost all cases is `user_data`. It is discussed below. However, it is insightful to start with more naive approaches.

#### Using global variables

If there is only one instance of such listener in the entire program, pointers to the textbox could be stored in a global variable.

```c++
textbox* the_textbox;

void the_listener() {
  get_text(the_textbox);
}

void setup(textbox* component) {
  the_textbox = component;
  add_listener(component, &the_listener);
}
```

Thread safety and lifetime issues aside, the limitation of one callback instance at once is often unacceptable. What if there are two textboxes in the GUI?

With careful use of C++ templates, the number of instances can be larger, as long as it known at compile time. For example, [`snippets/tagged_listener.h`](snippets/tagged_listener.h) automatically creates a new global variable for each use of `add_listener`. However, each instance of the listener is still hardcoded - a hand-written call to `bind()` is required to make the compiler create a new global variable rather than reuse an existing one. What if the number of textboxes is dynamic?

[`snippets/listener_bank.h`](snippets/listener_bank.h) presents a different approach, where many listener slots are allocated at compile time, and then filled in at runtime as necessary. This approach suffers from centralization, but more importantly, the fixed size is a problem. As listener slots have turned into a finite resource, they can now run out at runtime. To satisfy extreme use cases, a lot of slots should be prepared, but each slot slows down the compiler and bloats the binary.

Depending on the use case, the limitations inherent in this approach may not be an issue. It is, for instance, quite common to use global variables in error handling callbacks, as there is usually exactly one. If the maximum number of callbacks is pretty low, `listener_bank` may suffice.

As an aside, a reasonable scenario for `tagged_listener` is wrapping or decorating external functions. Consider aliceapp uses libbob. Some launcher or middleware way wish to inject logging, auditing, bugfixes or extra functionality to libbob. It can do so by loading libbob first, creating wrappers for some of its functions, and then passing these wrappers in place of libbob's functions into aliceapp.

#### Using thread-local variables

Actually, there is an extra sneaky piece of data available to the callback - its thread ID. Though it is not particularly useful with most GUI libraries, other types of callbacks can use `thread_local` variables to get around some problems, but a limit of one callback per thread and the need to know the thread ahead of callback execution are still substantial downsides.

This approach has its place when callbacks are executed immediately, not stored for later:

```c++
static thread_local int file_count = 0;

void visit_file(char const* path) {
  file_count++;
}

void visit_dir(char const* path) { /* Do nothing */ }

int count_files() {
  int old_file_count = std::exchange(file_count, 0);
  walk_tree("example/", &visit_file, &visit_dir);
  return std::exchange(file_count, old_file_count);
}
```

The use of thread-local variables makes this code parallelizable as long as `walk_tree` does everything synchronously in caller's thread. `old_file_count` is only needed if there is a risk of recursion.

#### Using globally-unique identifiers

(WIP)

#### With `user_data`

(WIP)

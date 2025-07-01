// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

// "background_jobs" example for voidstar library

#include "badlib.h"

#include <voidstar.h>

#include <cstdlib>
#include <iostream>
#include <list>
#include <string>

static int get_job_count(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " NUM_JOBS" << std::endl;
    std::exit(1);
  }
  return std::stoi(argv[1]);
}

int main(int argc, char *argv[]) {
  int jobs = get_job_count(argc, argv);

  // The callback for job completion.
  //
  // It could be a lambda with captures instead, but in this example it would be
  // a bit more syntactically awkward.
  struct the_callback {
    int job_number;

    void operator()(double result) {
      std::cout << "Job #" << this->job_number << " completed with result "
                << result << std::endl;
    }
  };

  // Shortcut for convenience
  using closure = voidstar::closure<badlib_job_callback, the_callback>;

  // Callbacks potentially in use.
  //
  // The C function pointer obtained via .get() is only usable while its source
  // voidstar::closure object is still alive. In this example, it is enough to
  // keep them around until badlib_join() returns.
  std::list<closure> closures;

  // Start all jobs in the background
  for (int i = 0; i < jobs; i++) {
    // Construct the i-th closure. The constructor of voidstar::closure
    // allocates and writes the machine code for a new C function.
    closures.emplace_back(the_callback{i});

    // Obtain a pointer to the new dynamically-generated C function
    closure &cls = closures.back();
    badlib_job_callback c_fn_ptr = cls.get();

    // Pass this pointer to the library when starting the job
    badlib_start_job(badlib_job{
        .param = static_cast<double>(rand() % 10),
        .on_done = c_fn_ptr,
    });
  }

  // Wait for all jobs to complete, and for all callbacks to return.
  badlib_join();

  // Now that the closures certainly cannot be invoked, it is safe to free them.
  closures.clear();
}

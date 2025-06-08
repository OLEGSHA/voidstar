#include "badlib.h"

#include <chrono>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

std::vector<std::jthread> jobs_in_progress;
std::mutex jobs_in_progress_mutex;

namespace {

void run(/*no stop token,*/ badlib_job job) {
  // Work really hard
  std::this_thread::sleep_for(std::chrono::milliseconds{1} * (rand() % 5000));

  if (job.on_done != nullptr) {
    job.on_done(job.param * 10.0);
  }
}

} // namespace

extern "C" {

void badlib_start_job(badlib_job job) {
  std::lock_guard const lock{jobs_in_progress_mutex};
  jobs_in_progress.emplace_back(&run, job);
}

void badlib_join(void) {
  std::lock_guard const lock{jobs_in_progress_mutex};
  jobs_in_progress.clear();
}
}

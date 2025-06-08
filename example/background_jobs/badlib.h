#ifndef BADLIB_H
#define BADLIB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*badlib_job_callback)(double result);

typedef struct {
  double param;
  badlib_job_callback on_done;
} badlib_job;

/**
 * @brief Begin execution of @a job in the background.
 *
 * When the job completes, the @c badlib_job_callback will be invoked with the
 * result in an unspecified thread.
 */
void badlib_start_job(badlib_job job);

/**
 * @brief Wait for all started jobs to complete and all callbacks to return.
 */
void badlib_join(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BADLIB_H

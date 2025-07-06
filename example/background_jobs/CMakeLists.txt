add_library(example_background_jobs_badlib STATIC badlib.cpp)

add_executable(example_background_jobs main.cpp)
target_link_libraries(example_background_jobs
                      PRIVATE voidstar example_background_jobs_badlib)

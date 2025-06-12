cmake_minimum_required(VERSION 3.25.1)

project(voidstar LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(voidstar STATIC src/ffi.cpp)
target_include_directories(voidstar PUBLIC include)
target_link_libraries(voidstar PUBLIC ffi)

add_executable(main src/main.cpp src/libstupid.cpp)
target_link_libraries(main PUBLIC voidstar)

add_subdirectory(example)
add_subdirectory(test)

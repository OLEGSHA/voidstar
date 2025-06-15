cmake_minimum_required(VERSION 3.25.1)

project(voidstar LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(voidstar INTERFACE)
target_include_directories(voidstar INTERFACE include)
target_link_libraries(voidstar INTERFACE ffi)

add_subdirectory(example)
add_subdirectory(test)

find_package(GTest REQUIRED)
enable_testing()

add_executable(tests closure.static.cpp closure.cpp types.cpp)

target_link_libraries(tests PRIVATE voidstar GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)

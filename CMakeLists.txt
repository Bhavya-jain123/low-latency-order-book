cmake_minimum_required(VERSION 3.20)
project(low_latency_order_engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LOE_BUILD_TESTS "Build unit and integration tests" ON)
option(LOE_BUILD_BENCHMARKS "Build benchmark executable" ON)

add_library(order_engine
    src/limit_order_book.cpp
    src/matching_engine.cpp
    src/workload.cpp
    src/benchmark_runner.cpp)
target_include_directories(order_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
find_package(Threads REQUIRED)
target_link_libraries(order_engine PUBLIC Threads::Threads)
target_compile_options(order_engine PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    # C4324 is expected: SPSC index alignment intentionally pads the type to isolate cache lines.
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /wd4324>)

add_executable(order_engine_cli src/cli.cpp)
target_link_libraries(order_engine_cli PRIVATE order_engine)

if(LOE_BUILD_BENCHMARKS)
  add_executable(order_engine_bench benchmarks/benchmark.cpp)
  target_link_libraries(order_engine_bench PRIVATE order_engine)
endif()

if(LOE_BUILD_TESTS)
  include(CTest)
  include(FetchContent)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  add_executable(order_engine_tests
      tests/order_book_tests.cpp
      tests/ring_buffer_tests.cpp
      tests/workload_tests.cpp)
  target_link_libraries(order_engine_tests PRIVATE order_engine GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(order_engine_tests)
endif()

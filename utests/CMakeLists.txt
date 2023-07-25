cmake_minimum_required(VERSION 3.14)
project(akk0rdsdk_utests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  akk0rdsdk_utests
  ../framework/configmanager.cpp
  src/test.cpp
)

target_link_libraries(
  akk0rdsdk_utests
  GTest::gtest_main
)

include_directories(
  ../framework
  src
)

include(GoogleTest)
gtest_discover_tests(akk0rdsdk_utests)

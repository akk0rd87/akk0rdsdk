cmake_minimum_required(VERSION 3.14)
project(akk0rdsdk_utests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_PROGRESS   TRUE
  GIT_TAG        04ee1b4f2aefdffb0135d7cf2a2c519fe50dabe4
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

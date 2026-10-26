cmake_minimum_required(VERSION 3.16)

# Десктопная сборка CI-приложения SDK (Windows, macOS): тот же src/main.cpp, что и у ci/proj.android
project(citest VERSION 1.0.0)

set(TARGET_NAME citest)
add_executable(${TARGET_NAME} ../proj.android/src/main.cpp)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /J")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funsigned-char")
endif()

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

set(SOURCES
    $ENV{AKKORD_SDK_HOME}/framework/billing.cpp
    $ENV{AKKORD_SDK_HOME}/framework/adrandomizer.cpp
    $ENV{AKKORD_SDK_HOME}/framework/basewrapper.cpp
    $ENV{AKKORD_SDK_HOME}/framework/configmanager.cpp
    $ENV{AKKORD_SDK_HOME}/framework/videodriver.cpp
    $ENV{AKKORD_SDK_HOME}/framework/atlasmanager.cpp
    $ENV{AKKORD_SDK_HOME}/framework/ads/admob/windows/admobfactory.cpp
    $ENV{AKKORD_SDK_HOME}/framework/ads/yandex/windows/yandexadsfactory.cpp
    $ENV{AKKORD_SDK_HOME}/framework/gdpr/windows/gdpr_consentpolicymanager_windows.cpp
)

if(APPLE) # macOS: обертка платформы — общая POSIX, видео — GPU-рендерер SDL3 (Metal)
    list(APPEND SOURCES
        $ENV{AKKORD_SDK_HOME}/framework/core/platformwrapper/linux/linux_wrapper.cpp
        $ENV{AKKORD_SDK_HOME}/framework/core/videoadapter/videoadapter_macos.cpp
    )
else()
    list(APPEND SOURCES
        $ENV{AKKORD_SDK_HOME}/framework/core/platformwrapper/windows/windows_wrapper.cpp
        $ENV{AKKORD_SDK_HOME}/framework/core/videoadapter/videoadapter_windows.cpp
    )
endif()

add_subdirectory("$ENV{AKKORD_SDK_HOME}/libraries/SDL/SDL" "${CMAKE_BINARY_DIR}/SDL")

include_directories(
    $ENV{AKKORD_SDK_HOME}/framework/
    $ENV{AKKORD_SDK_HOME}/libraries/SDL/SDL/include
)

target_sources(${TARGET_NAME} PRIVATE ${SOURCES})
target_link_libraries(${TARGET_NAME} PRIVATE SDL3::SDL3)

set_target_properties(SDL3-shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")

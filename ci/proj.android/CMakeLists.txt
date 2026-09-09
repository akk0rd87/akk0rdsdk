cmake_minimum_required(VERSION 3.10.0)

project(citest VERSION 1.0.0)

set(TARGET_NAME main)
add_library(${TARGET_NAME} SHARED ./src/main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funsigned-char -g") # -g: debug symbols for crashlytics
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fPIC")

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${TARGET_NAME} PRIVATE GL_GLEXT_PROTOTYPES)
target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wno-error=uninitialized)

set(SOURCES
    $ENV{AKKORD_SDK_HOME}/framework/billing.cpp
    $ENV{AKKORD_SDK_HOME}/framework/adrandomizer.cpp
    $ENV{AKKORD_SDK_HOME}/framework/basewrapper.cpp
    $ENV{AKKORD_SDK_HOME}/framework/configmanager.cpp
    $ENV{AKKORD_SDK_HOME}/framework/videodriver.cpp
    $ENV{AKKORD_SDK_HOME}/framework/core/platformwrapper/android/android_wrapper.cpp
    $ENV{AKKORD_SDK_HOME}/framework/core/platformwrapper/android/android_utils_wrapper.cpp
    $ENV{AKKORD_SDK_HOME}/framework/core/videoadapter/videoadapter_android.cpp
    $ENV{AKKORD_SDK_HOME}/framework/ads/admob/android/admobfactory.cpp
    $ENV{AKKORD_SDK_HOME}/framework/ads/yandex/android/yandexadsfactory.cpp
    $ENV{AKKORD_SDK_HOME}/framework/ads/vk/android/vkadsfactory.cpp
    $ENV{AKKORD_SDK_HOME}/framework/gdpr/android/gdpr_consentpolicymanager_android.cpp
)

add_subdirectory("$ENV{AKKORD_SDK_HOME}/libraries/SDL/SDL"             "${CMAKE_BINARY_DIR}/SDL")
add_subdirectory("$ENV{AKKORD_SDK_HOME}/libraries/SDL_image/SDL_image" "${CMAKE_BINARY_DIR}/SDL_image")

include_directories(
    ./src
    $ENV{AKKORD_SDK_HOME}/framework/
    $ENV{AKKORD_SDK_HOME}/libraries/SDL/SDL/include
    $ENV{AKKORD_SDK_HOME}/libraries/SDL_image/SDL_image/include
)

target_sources(${TARGET_NAME} PRIVATE ${SOURCES})
target_link_libraries(${TARGET_NAME}
    PRIVATE SDL2::SDL2
    PRIVATE SDL2::SDL2main
    PRIVATE SDL2_image::SDL2_image
    PRIVATE GLESv2
    PRIVATE android
)

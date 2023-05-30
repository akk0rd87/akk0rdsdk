export APP_STL=c++_shared
export APP_PLATFORM=android-16
export NDK_PROJECT_PATH=${AKKORD_SDK_HOME}/libraries/SDL_image/SDL_image
export APP_BUILD_SCRIPT=${NDK_PROJECT_PATH}/Android.mk
export NDK_LIBS_OUT=${AKKORD_SDK_HOME}/libraries/SDL_image/android/lib
export NDK_OUT=${AKKORD_SDK_HOME}/libraries/SDL_image/android/obj
export NDK_DEBUG=0
export SUPPORT_WEBP=FALSE
rm -rf ${NDK_LIBS_OUT}
${ANDROID_NDK_HOME}/ndk-build -e

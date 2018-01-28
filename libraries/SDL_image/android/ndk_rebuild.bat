rem ndk to 10c
call cls
set APP_PLATFORM=android-14
set NDK_PROJECT_PATH=%AKKORD_SDK_HOME%\libraries\SDL_image\SDL_image
set APP_BUILD_SCRIPT=%NDK_PROJECT_PATH%\Android.mk

call %ANDROID_NDK_HOME%/ndk-build -e clean
call %ANDROID_NDK_HOME%/ndk-build -e

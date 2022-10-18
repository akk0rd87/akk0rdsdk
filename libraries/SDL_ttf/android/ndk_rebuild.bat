call cls
set APP_ALLOW_MISSING_DEPS=true
set LOCAL_ALLOW_UNDEFINED_SYMBOLS=true
set LOCAL_C_INCLUDES       =%AKKORD_SDK_HOME%/libraries/SDL/SDL/include;
set LOCAL_EXPORT_C_INCLUDES=%AKKORD_SDK_HOME%/libraries/SDL/SDL/include
set APP_STL := c++_shared
set APP_PLATFORM=android-16
set NDK_PROJECT_PATH=%AKKORD_SDK_HOME%\libraries\SDL_ttf\SDL_ttf
set APP_BUILD_SCRIPT=%NDK_PROJECT_PATH%\Android.mk
set NDK_LIBS_OUT=%AKKORD_SDK_HOME%\libraries\SDL_ttf\android\lib
set NDK_OUT=%AKKORD_SDK_HOME%\libraries\SDL_ttf\android\obj

set SUPPORT_WEBP=FALSE

rd /s/q lib
call %ANDROID_NDK_HOME%/ndk-build -e clean
call %ANDROID_NDK_HOME%/ndk-build -e

del /s /q %AKKORD_SDK_HOME%\libraries\SDL_ttf\android\lib\libSDL2.so
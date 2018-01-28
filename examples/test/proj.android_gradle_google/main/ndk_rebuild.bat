rem ndk to 10c
call cls
set APP_PLATFORM=android-14
rem set NDK_PROJECT_PATH=%AKKORD_SDK_HOME%\libraries\SDL\SDL
set NDK_PROJECT_PATH=libs
set APP_BUILD_SCRIPT=C:\CommonVersioning\Akk0rdSDK\examples\test\proj.android_gradle_google\main\Android.mk
rem set LOCAL_C_INCLUDES=C:\CommonVersioning\Akk0rdSDK\libraries\SDL\SDL\include

call %ANDROID_NDK_HOME%/ndk-build -e clean
call %ANDROID_NDK_HOME%/ndk-build -e
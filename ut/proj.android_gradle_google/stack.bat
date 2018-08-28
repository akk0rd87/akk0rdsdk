call adb logcat -c
call adb logcat | "%ANDROID_NDK_HOME%/ndk-stack.cmd" -sym app/build/intermediates/transforms/stripDebugSymbol/debug/0/lib/x86/libmain.so
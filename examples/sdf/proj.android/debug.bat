call adb install -r app\build\outputs\apk\app-all-debug.apk
call adb logcat -c
call adb shell am start -n org.akk0rdsdk.sdffonttest/org.akk0rdsdk.sdffonttest.MainActivity
call adb logcat | findstr /i sdl
rem call adb logcat

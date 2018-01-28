call adb install -r app\build\outputs\apk\debug\app-debug.apk
call adb logcat -c
call adb shell am start -n org.akk0rdsdksdltest.app/org.akk0rdsdksdltest.app.MainActivity
call adb logcat | findstr /i sdl
rem call adb logcat

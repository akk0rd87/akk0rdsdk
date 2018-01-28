rem call adb install -r app\build\outputs\apk\app-all-debug.apk
call adb install -r app\build\outputs\apk\app-arm7-debug.apk
call adb logcat -c
call adb shell am start -n org.akk0rdsdksdltest.app/org.akk0rdsdksdltest.app.MainActivity
call adb logcat | findstr /i sdl

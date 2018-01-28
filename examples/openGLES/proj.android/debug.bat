call adb install -r app\build\outputs\apk\app-all-debug.apk
call adb logcat -c
call adb shell am start -n org.akk0rdsdkopenglestest.app/org.akk0rdsdkopenglestest.app.MainActivity
call adb logcat | findstr /i sdl

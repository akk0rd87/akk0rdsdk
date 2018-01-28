call adb install -r app\build\outputs\apk\debug\app-debug.apk
call adb logcat -c
call adb shell am start -n org.akk0rdsdkexample1.app/org.akk0rdsdkexample1.app.MainActivity
call adb logcat | findstr /i sdl

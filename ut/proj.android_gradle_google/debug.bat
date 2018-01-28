call adb install -r app\build\outputs\apk\debug\app-debug.apk
call adb logcat -c
call adb shell am start -n org.akk0rdsdkunittests.app/org.akk0rdsdkunittests.app.MainActivity
call adb logcat | findstr /i sdl

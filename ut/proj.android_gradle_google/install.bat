call adb logcat -c
FOR /F "skip=1" %%x IN ('adb devices') DO call adb -s %%x install -r app\build\outputs\apk\release\app-universal-release.apk
FOR /F "skip=1" %%x IN ('adb devices') DO call adb -s %%x shell am start -n org.akk0rdsdkunittests.app/org.akk0rdsdkunittests.app.MainActivity
FOR /F "skip=1" %%x IN ('adb devices') DO call adb -s %%x logcat | findstr /i sdl

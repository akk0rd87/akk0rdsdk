FOR /F "skip=1" %%x IN ('adb devices') DO call adb -s %%x logcat -c
FOR /F "skip=1" %%x IN ('adb devices') DO start %~dp0/stacklogcat.bat %%x
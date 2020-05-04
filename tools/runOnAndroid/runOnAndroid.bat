rem %1 - packageName
rem %2 - activityName
rem %3 - 0 or 1 if we need start logcat

FOR /F "skip=1" %%x IN ('adb devices') DO call adb -s %%x logcat -c
FOR /F "skip=1" %%x IN ('adb devices') DO call adb -s %%x shell am start -n %1/%1.%2

if 1==%3 FOR /F "skip=1" %%x IN ('adb devices') DO start %0/../runlogcat.bat %%x
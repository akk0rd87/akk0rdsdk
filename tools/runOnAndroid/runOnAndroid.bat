rem %1 - packageName
rem %2 - activityName
rem %3 - 0 or 1 if we need start logcat

FOR /F "skip=1 delims=~" %%x IN ('adb devices -l') DO (
    for /F "tokens=5* delims=:" %%a in ("%%x") do (
        call adb -t %%a logcat -c
    )
)

FOR /F "skip=1 delims=~" %%x IN ('adb devices -l') DO (
    for /F "tokens=5* delims=:" %%a in ("%%x") do (
        call adb -t %%a shell am start -n %1/%1.%2
    )
)


if 1 == %3 (
    FOR /F "skip=1 delims=~" %%x IN ('adb devices -l') DO (
        for /F "tokens=5 delims=:" %%a in ("%%x") do (
            start %0/../runlogcat.bat %%a %%x
        )
    )
)
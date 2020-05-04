rem %1 - packageName
rem %2 - activityName
rem %3 - 0 or 1 if we need start logcat

FOR /F "skip=1 delims=~" %%x IN ('adb devices -l') DO (
    echo %%x
    for /F "tokens=5* delims=:" %%a in ("%%x") do (
        echo %%a
        DO call adb -t %%a logcat -c
    )
)

FOR /F "skip=1 delims=~" %%x IN ('adb devices -l') DO (
    echo %%x
    for /F "tokens=5* delims=:" %%a in ("%%x") do (
        echo %%a
        DO call adb -t %%a shell am start -n %1/%1.%2
    )
)

FOR /F "skip=1 delims=~" %%x IN ('adb devices -l') DO (
    for /F "tokens=2,3,4,5 delims=:" %%a in ("%%x") do (
        echo %%a
        echo %%b
        echo %%c
        echo %%d

        set str = "%%a:%%b:%%c:%%d"
        echo %str%
        start %0/../runlogcat.bat %%d
    )
)
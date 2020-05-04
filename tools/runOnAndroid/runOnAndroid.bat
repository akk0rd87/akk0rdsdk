rem %1 - packageName
rem %2 - activityName
rem %3 - 0 or 1 if we need start logcat

@Echo Off

For /F "Skip=1 delims=~" %%G In ('adb.exe devices -l') Do (Set "DI=%%G"
    SetLocal EnableDelayedExpansion
    For /F %%I In ("!DI:*transport_id:=!") Do EndLocal & Echo %%I & call adb -t %%I logcat -c
)

For /F "Skip=1 delims=~" %%G In ('adb.exe devices -l') Do (Set "DI=%%G"
    SetLocal EnableDelayedExpansion
    For /F %%I In ("!DI:*transport_id:=!") Do EndLocal & Echo %%I & call adb -t %%I shell am start -n %1/%1.%2
)

if 1 == %3 (
    For /F "Skip=1 delims=~" %%G In ('adb.exe devices -l') Do (Set "DI=%%G"
        SetLocal EnableDelayedExpansion
        For /F %%I In ("!DI:*transport_id:=!") Do EndLocal & Echo %%I & start %0/../runlogcat.bat %%I %%G
    )
)
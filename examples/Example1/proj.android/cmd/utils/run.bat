rem @echo off
FOR /F "skip=1" %%x IN ('adb devices') DO call adb -s %%x logcat -c
for /f "tokens=* delims=" %%# in ('call %AKKORD_SDK_HOME%/tools/xpath/xpath.bat %~dp0../../AndroidManifest.xml "//manifest/@package"') do set "reagent=%%#"
FOR /F "skip=1" %%x IN ('adb devices') DO call adb -s %%x shell am start -n %reagent%/%reagent%.MainActivity

FOR /F "skip=1" %%x IN ('adb devices') DO start %~dp0/runlogcat.bat %%x
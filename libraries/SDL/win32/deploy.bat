del /q Release\*
set source_path=..\SDL\VisualC\Win32\Release
copy %source_path%\SDL3.dll Release\
copy %source_path%\SDL3.lib Release\
copy %source_path%\SDL3main.lib Release\
del /q Release\*
set source_path=..\SDL\VisualC\Win32\Release
copy %source_path%\SDL2.dll Release\
copy %source_path%\SDL2.lib Release\
copy %source_path%\SDL2.pdb Release\
copy %source_path%\SDL2main.lib Release\
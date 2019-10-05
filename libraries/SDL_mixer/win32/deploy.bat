del /q Release\*
set source_path=..\SDL_mixer\VisualC\Win32\Release
copy %source_path%\SDL2_image.dll Release\
copy %source_path%\SDL2_image.lib Release\
copy %source_path%\*.dll Release\
copy %source_path%\*.lib Release\
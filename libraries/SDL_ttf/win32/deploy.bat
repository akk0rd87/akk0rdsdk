del /q Release\*
set source_path=..\SDL_image\VisualC\Win32\Release
copy %source_path%\SDL2_image.dll Release\
copy %source_path%\SDL2_image.lib Release\
copy %source_path%\lib*.dll Release\
copy %source_path%\zlib*.dll Release\
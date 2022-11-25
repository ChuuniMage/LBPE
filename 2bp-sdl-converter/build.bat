call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
set CommonCompilerFlags=/Iinclude -Zi 
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib shell32.lib SDL2.lib sdl2main.lib /subsystem:console

cl %CommonCompilerFlags% .\hello.cpp /link %CommonLinkerFlags%
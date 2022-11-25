call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cl -Zi .\2bp.cpp
@REM @echo off
@REM mkdir build
@REM pushd build
@REM subst w: C:\Users\krisd\Documents\Programming\cpp
@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
@REM set path=w:\handmade\misc;%path%

@REM REM -WX treats warnings as compile errors
@REM set CommonCompilerFlags=-MT -nologo -Gm- -Oi -GR- -EHa- -FC -Z7 
@REM set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

@REM REM 32-bit build
@REM REM cl %CommonCompilerFlags% w:\handmade\code\win32_handmade.cpp /link  -subsystem:windows,5.1 %CommonLinkerFlags%

@REM cl %CommonCompilerFlags% w:\handmade\code\4bp.cpp /link %CommonLinkerFlags%

@REM popd
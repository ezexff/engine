call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

CLS

REM main
call devenv ..\build\win32_engine.exe

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

CLS

REM main
call devenv ..\build\win32_engine.exe

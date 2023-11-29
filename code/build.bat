@echo off
REM Init
REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
REM CLS
REM (ezexff): Run vcvarsall if need
where /q cl
IF %ERRORLEVEL% == 1 (call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64)

REM Compiler options
SET CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4456 -FC -Z7 -D_CRT_SECURE_NO_WARNINGS
SET CommonCompilerFlags=-DENGINE_INTERNAL=1 -DENGINE_SLOW=1 -DENGINE_WIN32=1 %CommonCompilerFlags%

REM Linker options
SET CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib shell32.lib "..\code\libs\glfw\lib-vc2022\glfw3_mt.lib" /NODEFAULTLIB:LIBCMT.lib /subsystem:windows /ENTRY:mainCRTStartup
REM Link compiled imgui .obj files
SET CommonLinkerFlags= %CommonLinkerFlags% imgui.obj imgui_demo.obj imgui_draw.obj imgui_impl_glfw.obj imgui_impl_opengl3.obj imgui_tables.obj imgui_widgets.obj
REM CommonLinkerFlags= %CommonLinkerFlags% "..\code\libs\glew\glew32.lib"

REM Includes
SET ImGui=/I..\code\libs\imgui
SET GLFW=/I..\code\libs\glfw\include
SET Glad=/I..\code\libs\glad\include /I..\code\libs\glad\src
SET StbImage=/I..\code\libs\stb
SET Includes=%ImGui% %GLFW% %Glad% %StbImage%

REM Sources
SET Sources=..\code\win32_engine.cpp

REM Recomplie imgui .obj files
REM SET Sources=..\code\win32_engine.cpp ..\code\libs\imgui\imgui*.cpp

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

del *.pdb > NUL 2> NUL

REM 64-bit build
REM Optimization switches -Od -Odi -O2
echo WAITING FOR PDB > lock.tmp
del lock.tmp
cl %CommonCompilerFlags% %Includes% %Sources% -Fmwin32_engine.map /link %CommonLinkerFlags%
popd
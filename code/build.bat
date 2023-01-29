@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
CLS

REM Compiler parameters
SET CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4456 -FC -Z7 -D_CRT_SECURE_NO_WARNINGS 
SET CommonCompilerFlags=-DENGINE_INTERNAL=1 -DENGINE_SLOW=1 -DENGINE_WIN32=1 %CommonCompilerFlags%
SET CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib "..\code\libs\glfw\lib-vc2022\glfw3dll.lib" /subsystem:windows /ENTRY:mainCRTStartup
REM "..\code\libs\glew\glew32.lib"

REM Libs
SET CppFiles="..\code\libs\imgui\imgui.cpp" "..\code\libs\imgui\imgui_impl_glfw.cpp" "..\code\libs\imgui\imgui_impl_opengl3.cpp" "..\code\libs\imgui\imgui_draw.cpp" "..\code\libs\imgui\imgui_tables.cpp" "..\code\libs\imgui\imgui_widgets.cpp" "..\code\libs\imgui\imgui_demo.cpp"
SET ImGui=/I"..\code\libs\imgui"
SET GLFW=/I"..\code\libs\glfw\include"
REM SET GLEW=/I"..\code\libs\glew\include"
SET Glad=/I"..\code\libs\glad\include" /I"..\code\libs\glad\src"
SET StbImage=/I"..\code\libs\stb"

SET Libs= %CppFiles% %ImGui% %GLFW% %Glad% %StbImage%

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

del *.pdb > NUL 2> NUL

REM 64-bit build
REM Optimization switches /O2
echo WAITING FOR PDB > lock.tmp
del lock.tmp
cl %CommonCompilerFlags% ..\code\win32_engine.cpp %Libs% -Fmwin32_engine.map /link %CommonLinkerFlags%
popd
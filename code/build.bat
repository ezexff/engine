@echo off

SET VcvarsallPath="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"

SET BuildType=Debug
SET BuildFolderPath=..\build
SET LibsFolderPath=..\libs
SET MainTranslationUnit=..\code\win32_engine.cpp
SET MapFileName=win32_engine.map

REM Run vcvarsall if need
where /q cl
IF %ERRORLEVEL% == 1 (call %VcvarsallPath% x64)

SET DefaultCompilerOpts=/fp:fast /GR- /Oi /nologo /EHa- /D_CRT_SECURE_NO_WARNINGS
SET DefaultCompilerOpts=%DefaultCompilerOpts% /DENGINE_INTERNAL=1 /DENGINE_SLOW=1 /DENGINE_WIN32=1
REM /GS- /Gs9999999
SET WarningsCompilerOpts=/WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /wd4456
SET DebugCompilerOpts=/Od /diagnostics:column /WL /FC /Z7 %WarningsCompilerOpts%
SET ReleaseCompilerOpts=/w /O2
REM /MTd /Zo (не нужен, если есть /Z7), /GL (оптимизация всей программы)

SET ImGui=/I%LibsFolderPath%\imgui
SET GLFW=/I%LibsFolderPath%\glfw\include
SET Glad=/I%LibsFolderPath%\glad\include /I%LibsFolderPath%\glad\src
SET Includes=%ImGui% %GLFW% %Glad%

SET LinkerOpts=/incremental:no /opt:ref /NODEFAULTLIB:LIBCMT.lib /SUBSYSTEM:windows /ENTRY:mainCRTStartup
REM /NODEFAULTLIB /STACK:0x100000,0x100000 /VERBOSE
SET LinkerLibs=user32.lib gdi32.lib winmm.lib opengl32.lib shell32.lib %LibsFolderPath%\glfw\lib-vc2022\glfw3_mt.lib
SET LinkerLibs=%LinkerLibs% imgui.obj imgui_demo.obj imgui_draw.obj imgui_impl_glfw.obj imgui_impl_opengl3.obj imgui_tables.obj imgui_widgets.obj

IF NOT EXIST %BuildFolderPath% mkdir %BuildFolderPath%
pushd %BuildFolderPath%

del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp

IF /i %BuildType%==Debug (
del lock.tmp
cl %DefaultCompilerOpts% %DebugCompilerOpts% %Includes% %MainTranslationUnit% /Fm%MapFileName% /link %LinkerOpts% %LinkerLibs%
)

IF /i %BuildType%==Release (
del lock.tmp
cl %DefaultCompilerOpts% %ReleaseCompilerOpts% %Includes% %MainTranslationUnit% /link %LinkerOpts% %LinkerLibs%
)

popd
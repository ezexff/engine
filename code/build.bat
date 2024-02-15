@echo off

SET vcvarsallPath="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
where /q cl
IF %ERRORLEVEL% == 1 (call %vcvarsallPath% x64)

REM BuildType can be ImGui or Debug or Release
REM Before Debug build use once ImGui that will create necessary imgui*.obj files
SET BuildType=Debug

SET BuildPath=..\build
SET LibsPath=..\libs
SET Win32TrUnit=..\code\win32_engine.cpp
SET EngineTrUnit=..\code\engine.cpp
SET RendererTrUnit=..\code\win32_engine_opengl.cpp
SET ImGuiTrUnits=%LibsPath%\imgui\imgui_impl_opengl3.cpp %LibsPath%\imgui\imgui_impl_win32.cpp %LibsPath%\imgui\imgui*.cpp
SET Win32Map=win32_engine.map
SET EngineMap=engine.map

REM Compiler options
SET DefaultCompilerOpts=/fp:fast /GR- /Oi /nologo /EHa- /D_CRT_SECURE_NO_WARNINGS /GS- /Gs9999999
SET DebugCompilerOpts=/WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /wd4456
SET DebugCompilerOpts=/MTd /Od /diagnostics:column /WL /FC /Z7 %DebugCompilerOpts% %DefaultCompilerOpts%
SET DebugCompilerOpts=/DENGINE_INTERNAL=1 %DebugCompilerOpts%
SET ReleaseCompilerOpts=/MT /w /O2 /DENGINE_INTERNAL=0 %DefaultCompilerOpts%
REM Possible options: /GL (оптимизация всей программы)

REM Includes
SET ImGui=/I%LibsPath%\imgui
SET DebugIncludes=%ImGui%

REM Linker options
SET DefaultLinkerOpts=/incremental:no /opt:ref /STACK:0x100000,0x100000
REM Possible options: /LTCG (оптимизация всей программы) /VERBOSE (печать сообщений о ходе выполнения)

IF NOT EXIST %BuildPath% mkdir %BuildPath%
pushd %BuildPath%

IF /i %BuildType%==ImGui (
cl /c %DebugCompilerOpts% %DebugIncludes% %ImGuiTrUnits%
)

IF /i %BuildType%==Debug (
del *.pdb >NUL 2>NUL
echo WAITING FOR PDB > lock.tmp
cl %DebugCompilerOpts% %DebugIncludes% %RendererTrUnit% /LD /link %DefaultLinkerOpts% /PDB:win32_engine_opengl_%random%.pdb /EXPORT:Win32LoadRenderer /EXPORT:Win32BeginFrame /EXPORT:Win32EndFrame User32.lib Gdi32.lib opengl32.lib imgui*.obj
cl %DebugCompilerOpts% %DebugIncludes% %EngineTrUnit% /Fm%EngineMap% /LD /link %DefaultLinkerOpts% /PDB:engine_%random%.pdb /EXPORT:GetSoundSamples /EXPORT:UpdateAndRender imgui*.obj
del lock.tmp
cl %DebugCompilerOpts% %DebugIncludes% %Win32TrUnit% /Fm%Win32Map% /link %DefaultLinkerOpts% /ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS User32.lib Gdi32.lib kernel32.lib Ole32.lib imgui*.obj
)

IF /i %BuildType%==Release (
REM Whats better for .dll compile? /NODEFAULTLIB or /NOENTRY (for now engine.dll needs /NOENTRY for SVML auto linking by MSVC)
cl %ReleaseCompilerOpts% %RendererTrUnit% /LD /link %DefaultLinkerOpts% /NOENTRY /EXPORT:Win32LoadRenderer /EXPORT:Win32BeginFrame /EXPORT:Win32EndFrame User32.lib Gdi32.lib opengl32.lib
cl %ReleaseCompilerOpts% %EngineTrUnit% /LD /link %DefaultLinkerOpts% /NOENTRY /EXPORT:GetSoundSamples /EXPORT:UpdateAndRender
cl %ReleaseCompilerOpts% %Win32TrUnit% /link %DefaultLinkerOpts% /ENTRY:WinMainCRTStartup /SUBSYSTEM:WINDOWS /NODEFAULTLIB User32.lib Gdi32.lib kernel32.lib Ole32.lib
del *.pdb >NUL 2>&1
del *.map >NUL 2>&1
del *.ini >NUL 2>&1
del *.obj >NUL 2>&1
del *.exp >NUL 2>&1
del *.lib >NUL 2>&1
)

popd
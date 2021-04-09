@echo off

set "PF=%ProgramFiles(x86)%"
if "%PF%"=="" set "PF=%ProgramFiles%"
call "%PF%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set VC="%VCIDEInstallDir%..\devenv.exe"
set PreferredToolArchitecture=x64
set DEPS=%~dp0deps

if exist \local_settings.cmd call \local_settings.cmd

set INCLUDE=%INCLUDE%;%DEPS%\vld\src
set INCLUDE=%INCLUDE%;%DEPS%\spdlog\include
set INCLUDE=%INCLUDE%;%DEPS%\expat\expat\lib
set INCLUDE=%INCLUDE%;%DEPS%\rapidxml
set INCLUDE=%INCLUDE%;%DEPS%\blend2d\src
set INCLUDE=%INCLUDE%;%DEPS%\CavalierContours\include
set INCLUDE=%INCLUDE%;%BOOST%

set LIBDBG=%DEPS%\blend2d\out\build\x64-Debug
set LIBDBG=%LIBDBG%;%DEPS%\expat\expat\out\build\x64-Debug
set LIBDBG=%LIBDBG%;%DEPS%\dxflib\debug
set LIBDBG=%LIBDBG%;%BOOST%\stage\lib
set LIBDBG=%LIBDBG%;%LIB%

set LIBREL=%DEPS%\blend2d\out\build\x64-Release
set LIBREL=%LIBREL%;%DEPS%\expat\expat\out\build\x64-Release
set LIBREL=%LIBREL%;%DEPS%\dxflib\release
set LIBREL=%LIBREL%;%BOOST%\stage\lib
set LIBREL=%LIBREL%;%LIB%

set PYTHON=%LOCALAPPDATA%\Programs\Python\Python38\python.exe
set CLANG_FORMAT="%ProgramW6432%\llvm\bin\clang-format.exe"
start "" %VC% "%~dp0pcb.sln"

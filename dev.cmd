@echo off
set "PF=%ProgramFiles(x86)%"
if "%PF%"=="" set "PF=%ProgramFiles%"
call "%PF%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set VC="%VCIDEInstallDir%..\devenv.exe"
set PreferredToolArchitecture=x64
set DEPS=%~dp0deps
if exist \local_settings.cmd call \local_settings.cmd
set LIB=%LIB%;%DEPS%\lib
set PYTHON=%LOCALAPPDATA%\Programs\Python\Python38\python.exe
set CLANG_FORMAT="%ProgramW6432%\llvm\bin\clang-format.exe"
start "" %VC% "%~dp0pcb.sln"

@echo off
set "PF=%ProgramFiles(x86)%"
if "%PF%"=="" set "PF=%ProgramFiles%"
call "%PF%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set VC="%VCIDEInstallDir%..\devenv.exe"
set PreferredToolArchitecture=x64
set DEPS=d:\oss
set BOOST=boost_1_75_0
set LIB=%LIB%;d:\oss\lib
start "" %VC% "%~dp0pcb.sln"

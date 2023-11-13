# build.ps1
# Scaffolds and compiles wxWidgets to our requirements

#echo test

#$CMakeDefs = -DwxBUILD_COMPATIBILITY='2.8' -DwxBUILD_SHARED=ON -DwxUSE_EXCEPTIONS=OFF"


# Set to continue to enable debug output
#$DebugPreference = "SilentlyContinue"
$DebugPreference = "Continue"

###################
##   Variables   ##
###################

$Root = $PSScriptRoot
$Win64Root = "$Root\x64"
$Win32Root = "$Root\x86"


# Create Win64 and Win32 builds
Write-Output "Generating Projects"

# We surpress the deprecated warning, yes we know, but we don't care right now!
cmake .. -B x64 -Wno-deprecated -G "Visual Studio 17 2022" -A x64 -DwxBUILD_COMPATIBILITY="2.8" -DwxBUILD_SHARED=ON -DwxUSE_EXCEPTIONS=OFF
cmake .. -B x86 -Wno-deprecated -G "Visual Studio 17 2022" -A Win32 -DwxBUILD_COMPATIBILITY="2.8" -DwxBUILD_SHARED=ON -DwxUSE_EXCEPTIONS=OFF

# Compile Win64 Debug & Release
Set-Location $Win64Root
Write-Output "Compiling Win64 Debug"
cmake --build . --config Debug
Write-Output "Compiling Win64 Release"
cmake --build . --config Release
Write-Output "Win64 build complete!"

# Compile Win32 Debug & Release
Set-Location $Win32Root
Write-Output "Compiling Win32 Debug"
cmake --build . --config Debug
Write-Output "Compiling Win32 Release"
cmake --build . --config Release
Write-Output "Win32 build complete!"

# Set back to root
Set-Location $Root
Write-Output "Builds completed, please verify output for errors!"

# build.ps1
# Scaffolds and compiles wxWidgets to our requirements

# Set to continue to enable debug output
#$DebugPreference = "SilentlyContinue"
$DebugPreference = "Continue"

$CMakeGenerator = 'Visual Studio 17 2022'
$CMakeOptions = @(
    "wxBUILD_COMPATIBILITY='2.8'",
    "wxBUILD_SHARED=ON",
    "wxUSE_EXCEPTIONS=OFF",
    "wxUSE_ON_FATAL_EXCEPTION=OFF",
    "wxUSE_STACKWALKER=OFF",
    "wxUSE_EXTENDED_RTTI=OFF",
    "wxUSE_CRASHREPORT=OFF"

)
$JoinedOptions = $CMakeOptions -join ' -D'

Write-Debug "Compiling with the following options:"
foreach ($Option in $CMakeOptions) {
    Write-Debug "$Option"
}

###################
##   Variables   ##
###################

$Root = $PSScriptRoot
$Win64Root = "$Root\x64"
$Win32Root = "$Root\x86"


# Create Win64 and Win32 builds
Write-Output "Generating Projects"

# We surpress the deprecated warning, yes we know, but we don't care right now!

Write-Output "Generating Win64"
Invoke-Expression "cmake .. -B x64 -Wno-deprecated -G '$CMakeGenerator' -A x64 -D$JoinedOptions"
Write-Output "Generating Win32"
Invoke-Expression "cmake .. -B x86 -Wno-deprecated -G '$CMakeGenerator' -A Win32 -D$JoinedOptions"

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

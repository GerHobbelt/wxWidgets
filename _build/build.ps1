# build.ps1
# Scaffolds and compiles wxWidgets to our requirements
#
# To build without Win32, run this: ./build.ps1 -BuildWin32:$false
#

param
(
    [switch]$ConfigureOnly = $false,
    [switch]$BuildWin32 = $true,
    [switch]$BuildWin64 = $true
)

# Set to continue to enable debug output
#$DebugPreference = "SilentlyContinue"
$DebugPreference = "Continue"

$Root = $PSScriptRoot
$CMakeGenerator = 'Visual Studio 17 2022'

$OptionsFile = "$Root\build_options.txt"
$OptionsFileContent = Get-Content -Path $OptionsFile
$JoinedOptions = $OptionsFileContent -join ' '

Write-Debug "Building Win64 $BuildWin64"
Write-Debug "Building Win32 $BuildWin32"

Write-Debug "Compiling with the following options:"
foreach ($Option in $OptionsFileContent)
{
    Write-Debug "$Option"
}

###################
##   Variables   ##
###################

$Win64Root = "$Root\x64"
$Win32Root = "$Root\x86"

# Create Win64 and Win32 builds
Write-Output "Generating Projects"

# If you add/remove options, be sure to comment this out so the last compile gets removed
# else the new options may not propagate for the compile due to CMakeCache
#Remove-Item -path $Win32Root -Recurse -Force
#Remove-Item -path $Win64Root -Recurse -Force

# We surpress the deprecated warning, yes we know, but we don't care right now!
if ($BuildWin64)
{
    Write-Output "Generating Win64"
    Invoke-Expression "cmake .. -B x64 -Wno-deprecated -G '$CMakeGenerator' -A x64 $JoinedOptions"
}

if ($BuildWin32)
{
    Write-Output "Generating Win32"
    Invoke-Expression "cmake .. -B x86 -Wno-deprecated -G '$CMakeGenerator' -A Win32 $JoinedOptions"
}

if ($ConfigureOnly)
{
    Write-Output "-ConfigureOnly, exiting the script."
    exit
}

# Compile Win64 Debug & Release
if ($BuildWin64)
{
    Set-Location $Win64Root
    Write-Output "Compiling Win64 Debug"
    cmake --build . --config Debug
    Write-Output "Compiling Win64 Release"
    cmake --build . --config Release
    Write-Output "Win64 build complete!"
}

# Compile Win32 Debug & Release
if ($BuildWin32)
{
    Set-Location $Win32Root
    Write-Output "Compiling Win32 Debug"
    cmake --build . --config Debug
    Write-Output "Compiling Win32 Release"
    cmake --build . --config Release
    Write-Output "Win32 build complete!"
}

# Set back to root
Set-Location $Root
Write-Output "Builds completed, please verify output for errors!"

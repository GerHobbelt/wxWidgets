# Set to continue to enable debug output
$DebugPreference = "SilentlyContinue"
#$DebugPreference = "Continue"

###################
##   Variables   ##
###################

$Root = $PSScriptRoot
$Win64Root = "$Root\x64"
$Win32Root = "$Root\x86"

$Win64Lib = "$Win64Root\Lib\vc_x64_dll"
$Win32Lib = "$Win32Root\Lib\vc_dll"

$OutputPath = "$Root\Out"
$Win64DllOut = "$OutputPath\Binaries\Win64"
$Win32DllOut = "$OutputPath\Binaries\Win32"

#Updates OutputPath.
# Example: SetOutDir -SubDir Dev
# Write-Output $OutputPath ("$Root\Out\Dev")
function SetOutDir
{
    param
    (
        [string] $SubDir
    )
    Write-Debug "Operation for SetOutDir -SubDir $SubDir"


    #Set-Variable -Name "OutputDir" -Value "$Root\Out\$SubDir"
    $script:OutputPath = "$Root\Out\$SubDir"
    $script:Win64DllOut = "$OutputPath\Binaries\Win64"
    $script:Win32DllOut = "$OutputPath\Binaries\Win32"
}

function CopyDirectory {
    param
    (
        [string]$InDir,
        [string]$OutDir
    )

    Write-Debug "Operation for CopyDirectory -InDir $InDir -OutDir $OutDir"

    # Create the destination folder if it doesn't exist
    if (-not (Test-Path $OutDir)) {
        New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
    }

    # Copy the entire directory and its contents recursively
    Copy-Item -Path $InDir -Destination $OutDir -Recurse -Force | Out-Null
}

function CopyGlob
{
    param
    (
        [string]$InDir,
        [string]$OutDir,
        [string]$Filter = "wxmsw*.dll"
    )

    Write-Debug "Operation for CopyGlob -InDir $InDir -OutDir $OutDir -Ext $Filter"

    # Create the output folder if it doesn't exist
    if (-not (Test-Path $OutDir)) {
        New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
    }

    # Get all DLL files in the source folder
    $Files = Get-ChildItem -Path $InDir -Filter "$Filter"

    # Loop over each DLL file
    foreach ($File in $Files)
    {
        Write-Debug "Copying $($File.Name)"
        Copy-Item -Path $File.FullName -Destination $OutDir -Force
    }

}

#####################
##   Script Main   ##
#####################

if (-not (Test-Path -Path $Win64Lib))
{
    Write-Error "$Win64Lib could not be found! Make sure it exists and you compiled both debug and release!"
    Return
}

if (-not (Test-Path -Path $Win32Lib))
{
    Write-Error "$Win32Lib could not be found! Make sure it exists and you compiled both debug and release!"
    Return
}

if (Test-Path -Path $OutputPath)
{
    Write-Output "Old output detected, removing"
    Remove-Item "$OutputPath\*" -Recurse -Force | Out-Null
}

# Package up only the stuff we need to ship to the end user
Write-Output "Preparing Release"
SetOutDir -SubDir "Rel"
CopyGlob -InDir $Win64Lib -OutDir $Win64DllOut -Filter "wx*311u_*"
CopyGlob -InDir $Win32Lib -OutDir $Win32DllOut -Filter "wx*311u_*"

# Copy wxrc
CopyGlob -InDir $Win64Lib -OutDir $Win64DllOut -Filter "wxrc*"
CopyGlob -InDir $Win32Lib -OutDir $Win32DllOut -Filter "wxrc*"

# Package up the dev requirements
Write-Output "Preparing Developer"
SetOutDir -SubDir "Dev"

$InIncDir = "$Root\..\include"
$ExtFolder = "$OutputPath\Development\External\wxWidgets"

# Copy Includes
CopyDirectory -InDir $InIncDir -OutDir $ExtFolder
CopyDirectory -InDir "$Win64Lib\mswud" -OutDir "$ExtFolder\lib\vc_dll\win64"
CopyDirectory -InDir "$Win64Lib\mswu" -OutDir "$ExtFolder\lib\vc_dll\win64"
CopyDirectory -InDir "$Win32Lib\mswud" -OutDir "$ExtFolder\lib\vc_dll\win32"
CopyDirectory -InDir "$Win32Lib\mswu" -OutDir "$ExtFolder\lib\vc_dll\win32"

CopyGlob -InDir $Win64Lib -OutDir $Win64DllOut -Filter "wx*311ud*"
CopyGlob -InDir $Win32Lib -OutDir $Win32DllOut -Filter "wx*311ud*"

CopyGlob -InDir $Win64Lib -OutDir "$ExtFolder\lib\vc_dll\win64" -Filter "*.lib"
CopyGlob -InDir $Win32Lib -OutDir "$ExtFolder\lib\vc_dll\win32" -Filter "*.lib"

# Copy wxrc
CopyGlob -InDir $Win64Lib -OutDir $Win64DllOut -Filter "wxrc*"
CopyGlob -InDir $Win32Lib -OutDir $Win32DllOut -Filter "wxrc*"


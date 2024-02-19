param
(
    [Parameter(Mandatory=$true)]
    [string]$BuildDir,

    [Parameter(Mandatory=$true)]
    [string]$Arch,

    [Parameter(Mandatory=$true)]
    [string]$OutputDir
)

$Root = $PSScriptRoot

# Include common packaging utils
. "$Root/package_utils.ps1"

$NewArch = ConvertArchType -CurrentArch $Arch
$NewArchLower = $NewArch.ToLower()

$InLibDir = "$BuildDir/lib/vc_dll/"
$InIncDir = "$Root/../include"

$ExtFolder = "$OutputDir/Development/External/wxWidgets"
$OutBinaryDir = "$OutputDir/Binaries/$NewArch"
$OutLibDir = "$ExtFolder/lib/vc_dll/$NewArchLower"
# x64 is a bit weird in that it has the arch in the lib folder
if ($Arch -eq "x64")
{
    $InLibDir = "$BuildDir/lib/vc_x64_dll/"
}

Write-Output "Packaging for $NewArch"
Write-Output "BuildDir: $BuildDir"
Write-Output "OutputDir: $OutputDir"

# Includes
CopyDirectory -InDir $InIncDir -OutDir $ExtFolder

# Lib Dir
CopyDirectory -InDir "$InLibDir/mswud" -OutDir $OutLibDir
CopyDirectory -InDir "$InLibDir/mswu" -OutDir $OutLibDir
CopyGlob -InDir $InLibDir -OutDir $OutLibDir -Filter "*.lib"

# Binaries Dir
CopyGlob -InDir $InLibDir -OutDir $OutBinaryDir -Filter "wx*311u*.dll"
CopyGlob -InDir $InLibDir -OutDir $OutBinaryDir -Filter "wx*311u*.pdb"
CopyGlob -InDir $InLibDir -OutDir $OutBinaryDir -Filter "wxrc*.exe"
CopyGlob -InDir $InLibDir -OutDir $OutBinaryDir -Filter "wxrc*.pdb"

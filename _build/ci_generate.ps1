param
(
    [Parameter(Mandatory=$true)]
    [string]$OutputDir,

    [Parameter(Mandatory=$true)]
    [string]$BuildType
)

$Root = $PSScriptRoot
$OptionsFile = "$Root\build_options.txt"
$OptionsFileContent = Get-Content -Path $OptionsFile
$JoinedOptions = $OptionsFileContent -join ' '

Write-Output "Compiling with the following options:"
foreach ($Option in $OptionsFileContent)
{
    Write-Output "$Option"
}

Invoke-Expression "cmake .. -G 'Ninja' -B $OutputDir -Wno-deprecated -DCMAKE_BUILD_TYPE=$BuildType $JoinedOptions"

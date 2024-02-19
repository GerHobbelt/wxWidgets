function CopyDirectory
{
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

# Converts various arch types to more friendly ones
# Example: x64 -> Win64.
# We have this because cmake uses x64 for targeting win64, but UE3 wants
# all x64 binaries to be in Win64 instead.
function ConvertArchType
{
    param
    (
        [Parameter(Mandatory=$true)]
        [string] $CurrentArch
    )

    switch ($CurrentArch) {
        "x64" { return "Win64" }
        Default { return $CurrentArch }
    }
}

## How to

### Local Builds

To run a local build, simply run build.ps1 then package.ps1. This should build both Win32/64 and Debug/Release.

All of these scripts have options to run in either Win32 or Win64 modes. You can set this by passing `-BuildWin32:$false` or `-BuildWin64:$false` to the script.

### Adding new options

To add new options, simply add them into build_options.txt in the same way you define CMake options through the command line.
This specific setup is used so that both our CI and local builds are built using the same settings without
having to go through and manually add settings to the GitHub CI file

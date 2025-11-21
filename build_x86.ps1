# Build P2P-DLL for x86

$ErrorActionPreference = "Stop"

Write-Host "Setting up VS Dev Shell for x86..."
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch x86

$projectDir = "d:\RO\patcher\WARP-p2p-client\P2P-DLL"
$buildDir = "$projectDir\build_x86"

if (!(Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir -Force
}

Set-Location $buildDir

Write-Host "Configuring CMake..."
& "D:\Program Files\CMake\bin\cmake.exe" `
  -S ".." `
  -B "." `
  -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DCMAKE_BUILD_TYPE=Release `
  -DVCPKG_TARGET_TRIPLET=x86-windows `
  -DCMAKE_PREFIX_PATH="D:/vcpkg/installed/x86-windows"

Write-Host "Building..."
& "D:\Program Files\CMake\bin\cmake.exe" --build "." --config Release

Write-Host "Build Complete."

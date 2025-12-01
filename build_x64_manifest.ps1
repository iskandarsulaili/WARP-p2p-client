# Build P2P-DLL for x64 (Manifest Mode)

$ErrorActionPreference = "Stop"

Write-Host "Setting up VS Dev Shell for x64..."
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64

$projectDir = "d:\RO\patcher\WARP-p2p-client\P2P-DLL"
$buildDir = "$projectDir\build_x64_manifest"

if (Test-Path $buildDir) {
    Remove-Item -Path $buildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $buildDir -Force

Set-Location $buildDir

Write-Host "Configuring CMake (x64)..."
& "D:\Program Files\CMake\bin\cmake.exe" `
  -S ".." `
  -B "." `
  -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DCMAKE_BUILD_TYPE=Release `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DVCPKG_MANIFEST_MODE=ON

Write-Host "Building (x64)..."
& "D:\Program Files\CMake\bin\cmake.exe" --build "." --config Release

Write-Host "Build Complete (x64)."

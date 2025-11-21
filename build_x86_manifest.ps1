# Build P2P-DLL for x86 (Manifest Mode)

$ErrorActionPreference = "Stop"

Write-Host "Setting up VS Dev Shell for x86..."
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch x86

$projectDir = "d:\RO\patcher\WARP-p2p-client\P2P-DLL"
$buildDir = "$projectDir\build_x86_manifest"

if (Test-Path $buildDir) {
    Remove-Item -Path $buildDir -Recurse -Force
    # Write-Host "Build dir exists."
}
New-Item -ItemType Directory -Path $buildDir -Force

Set-Location $buildDir

Write-Host "Configuring CMake (x86)..."
& "D:\Program Files\CMake\bin\cmake.exe" `
  -S ".." `
  -B "." `
  -A Win32 `
  -DCMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DCMAKE_BUILD_TYPE=Release `
  -DVCPKG_TARGET_TRIPLET=x86-windows `
  -DVCPKG_INSTALL_OPTIONS=--allow-unsupported `
  -DVCPKG_MANIFEST_MODE=ON

Write-Host "Building (x86)..."
& "D:\Program Files\CMake\bin\cmake.exe" --build "." --config Release

Write-Host "Build Complete (x86)."

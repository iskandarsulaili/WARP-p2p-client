# P2P-DLL Build Fix Script
# Resolves zlib linking issues and performs clean rebuild
# Run from WARP-p2p-client/P2P-DLL directory

param(
    [string]$Triplet = "x86-windows",
    [string]$Config = "Release"
)

Write-Host "=== P2P-DLL Build Fix Script ===" -ForegroundColor Cyan
Write-Host "Triplet: $Triplet" -ForegroundColor White
Write-Host "Config: $Config" -ForegroundColor White
Write-Host ""

# Step 1: Set vcpkg triplet
Write-Host "[1/10] Setting vcpkg triplet..." -ForegroundColor Yellow
$env:VCPKG_DEFAULT_TRIPLET = $Triplet

# Step 2: Check vcpkg availability
Write-Host "[2/10] Checking vcpkg..." -ForegroundColor Yellow
if (-not (Get-Command vcpkg -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: vcpkg not found in PATH" -ForegroundColor Red
    Write-Host "Install vcpkg: https://github.com/microsoft/vcpkg" -ForegroundColor Red
    exit 1
}
$vcpkgVersion = vcpkg version
Write-Host "  vcpkg found: $vcpkgVersion" -ForegroundColor Green

# Step 3: Remove old zlib installations
Write-Host "[3/10] Removing old zlib installations..." -ForegroundColor Yellow
vcpkg remove zlib:$Triplet --recurse 2>$null | Out-Null
vcpkg remove zlib:x64-windows --recurse 2>$null | Out-Null
Write-Host "  Old installations removed" -ForegroundColor Green

# Step 4: Install zlib for correct triplet
Write-Host "[4/10] Installing zlib:$Triplet..." -ForegroundColor Yellow
vcpkg install zlib:$Triplet
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: vcpkg install failed!" -ForegroundColor Red
    exit 1
}

# Step 5: Verify zlib installation
Write-Host "[5/10] Verifying zlib installation..." -ForegroundColor Yellow
$zlibCheck = vcpkg list | Select-String "zlib:$Triplet"
if ($zlibCheck) {
    Write-Host "  ✓ $zlibCheck" -ForegroundColor Green
} else {
    Write-Host "  ✗ Zlib not installed for $Triplet!" -ForegroundColor Red
    exit 1
}

# Step 6: Verify import library exists
Write-Host "[6/10] Checking zlib import library..." -ForegroundColor Yellow
$zlibLib = "vcpkg_installed\$Triplet\lib\zlib.lib"
if (Test-Path $zlibLib) {
    $libSize = (Get-Item $zlibLib).Length / 1KB
    Write-Host "  ✓ Found: $zlibLib ($([math]::Round($libSize, 2)) KB)" -ForegroundColor Green
} else {
    Write-Host "  ✗ Missing: $zlibLib" -ForegroundColor Red
    Write-Host "  Check vcpkg installation manually" -ForegroundColor Red
    exit 1
}

# Step 7: Clean build directory
Write-Host "[7/10] Cleaning build directory..." -ForegroundColor Yellow
if (Test-Path build) {
    Remove-Item -Recurse -Force build
    Write-Host "  Removed old build/" -ForegroundColor Green
}
New-Item -ItemType Directory -Path build | Out-Null
Write-Host "  Created fresh build/" -ForegroundColor Green

# Step 8: Configure with CMake
Write-Host "[8/10] Configuring with CMake..." -ForegroundColor Yellow
cmake -B build -S . `
    -DVCPKG_TARGET_TRIPLET=$Triplet `
    -DCMAKE_BUILD_TYPE=$Config `
    -DBUILD_TESTS=ON

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ✗ CMake configuration failed!" -ForegroundColor Red
    Write-Host "  Check CMakeLists.txt and vcpkg.json" -ForegroundColor Red
    exit 1
}
Write-Host "  ✓ Configuration successful" -ForegroundColor Green

# Step 9: Build
Write-Host "[9/10] Building p2p_network.dll..." -ForegroundColor Yellow
cmake --build build --config $Config -j

if ($LASTEXITCODE -ne 0) {
    Write-Host "  ✗ Build failed! Check errors above." -ForegroundColor Red
    Write-Host "  Common issues:" -ForegroundColor Yellow
    Write-Host "    - Wrong vcpkg triplet (x86 vs x64)" -ForegroundColor White
    Write-Host "    - Missing dependencies" -ForegroundColor White
    Write-Host "    - Compiler version incompatibility" -ForegroundColor White
    exit 1
}
Write-Host "  ✓ Build successful" -ForegroundColor Green

# Step 10: Verify DLL created
Write-Host "[10/10] Verifying DLL..." -ForegroundColor Yellow
$dllPath = "build\bin\$Config\p2p_network.dll"
if (Test-Path $dllPath) {
    $dllSize = (Get-Item $dllPath).Length / 1KB
    Write-Host "  ✓ DLL created: $dllPath" -ForegroundColor Green
    Write-Host "    Size: $([math]::Round($dllSize, 2)) KB" -ForegroundColor Green
    
    # Check for zlib dependency
    $deps = dumpbin /DEPENDENTS $dllPath 2>$null | Select-String "zlib"
    if ($deps) {
        Write-Host "    ✓ Zlib dependency resolved: $deps" -ForegroundColor Green
    }
} else {
    Write-Host "  ✗ DLL not created at: $dllPath" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "=== Build Fix Complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Run tests:" -ForegroundColor White
Write-Host "     cmake --build build --target RUN_TESTS" -ForegroundColor Gray
Write-Host ""
Write-Host "  2. Integrate ECDHE key exchange:" -ForegroundColor White
Write-Host "     See FIX_IMPLEMENTATIONS.md Section 1" -ForegroundColor Gray
Write-Host ""
Write-Host "  3. Disable compression (security):" -ForegroundColor White
Write-Host "     Edit config/p2p_config.json, set compression.enabled=false" -ForegroundColor Gray
Write-Host ""
Write-Host "  4. Review analysis:" -ForegroundColor White
Write-Host "     BUILD_ERROR_ANALYSIS.md" -ForegroundColor Gray
Write-Host ""
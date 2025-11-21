# Deploy x86 DLLs to Client

$buildDir = "d:\RO\patcher\WARP-p2p-client\P2P-DLL\build_x86_manifest\bin\Release"
$clientDir = "d:\RO\client"

Write-Host "Deploying x86 DLLs from $buildDir to $clientDir..."

$dlls = @(
    "p2p_network.dll",
    "spdlog.dll",
    "libssl-3.dll", 
    "libcrypto-3.dll",
    "brotlidec.dll",
    "brotlicommon.dll",
    "fmt.dll",
    "datachannel.dll",
    "msquic.dll",
    "zlib1.dll",
    "libsodium.dll",
    "lz4.dll"
)

# Note: OpenSSL 3 names might vary (libssl-3-x64 vs libssl-3). x86 usually is libssl-3.dll or libssl-3-x86.dll?
# I'll check the output later. For now I'll copy *.dll to be safe, or specific ones.
# Actually, copying *.dll is safer for dependencies.

Get-ChildItem "$buildDir\*.dll" | ForEach-Object {
    Copy-Item $_.FullName -Destination $clientDir -Force
    Write-Host "Copied $($_.Name)"
}

# Copy config if needed
$configSrc = "d:\RO\patcher\WARP-p2p-client\P2P-DLL\config\p2p_config.json"
if (Test-Path $configSrc) {
    Copy-Item $configSrc -Destination $clientDir -Force
    Write-Host "Copied p2p_config.json"
}

Write-Host "Deployment Complete."

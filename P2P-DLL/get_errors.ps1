$output = cmake --build "build" --config Release --target p2p_network 2>&1
$errors = $output | Where-Object { $_ -match "error" }
$errors | Out-File -FilePath "errors.txt" -Width 4096
Get-Content "errors.txt"

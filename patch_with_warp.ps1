<#
.SYNOPSIS
    Patches Ragnarok Online client with P2P network DLL support using WARP.exe

.DESCRIPTION
    This script automates the WARP patching process to inject p2p_network.dll
    into the RO client's Import Table using the CustomDLL patch.
    
    Supports both GUI mode (WARP.exe) and console mode (WARP_console.exe).

.PARAMETER InputExe
    Path to the input RO client executable to patch

.PARAMETER OutputExe
    Path where the patched executable will be saved

.PARAMETER SessionFile
    YAML session file to use for patching (default: P2P_Session.yml)

.PARAMETER ConsoleMode
    Use console mode (WARP_console.exe) instead of GUI mode

.EXAMPLE
    .\patch_with_warp.ps1 -InputExe "client.exe" -OutputExe "client_patched.exe"
    
.EXAMPLE
    .\patch_with_warp.ps1 -InputExe "client.exe" -OutputExe "client_patched.exe" -ConsoleMode
    
.EXAMPLE
    .\patch_with_warp.ps1 -InputExe "client.exe" -OutputExe "client_patched.exe" -SessionFile "P2P_Patch_Session.yml" -ConsoleMode

.NOTES
    Author: AI-MMORPG Project
    Requires: WARP.exe or WARP_console.exe in win32/ directory
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true, HelpMessage="Path to input RO client executable")]
    [ValidateNotNullOrEmpty()]
    [string]$InputExe,
    
    [Parameter(Mandatory=$true, HelpMessage="Path for output patched executable")]
    [ValidateNotNullOrEmpty()]
    [string]$OutputExe,
    
    [Parameter(Mandatory=$false, HelpMessage="Session YAML file to use")]
    [string]$SessionFile = "P2P_Session.yml",
    
    [Parameter(Mandatory=$false, HelpMessage="Use console mode instead of GUI")]
    [switch]$ConsoleMode
)

# Script configuration
$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$WarpDir = Join-Path $ScriptDir "win32"
$WarpGui = Join-Path $WarpDir "WARP.exe"
$WarpConsole = Join-Path $WarpDir "WARP_console.exe"

# Color output functions
function Write-ColorOutput {
    param([string]$Message, [string]$Color = "White")
    Write-Host $Message -ForegroundColor $Color
}

function Write-Success {
    param([string]$Message)
    Write-ColorOutput "✓ $Message" "Green"
}

function Write-Error-Custom {
    param([string]$Message)
    Write-ColorOutput "✗ $Message" "Red"
}

function Write-Warning-Custom {
    param([string]$Message)
    Write-ColorOutput "⚠ $Message" "Yellow"
}

function Write-Info {
    param([string]$Message)
    Write-ColorOutput "ℹ $Message" "Cyan"
}

# Banner
function Show-Banner {
    Write-Host ""
    Write-ColorOutput "╔══════════════════════════════════════════════════════╗" "Cyan"
    Write-ColorOutput "║                                                      ║" "Cyan"
    Write-ColorOutput "║         WARP P2P-DLL Patcher                         ║" "Cyan"
    Write-ColorOutput "║         RO Client Import Table Modifier              ║" "Cyan"
    Write-ColorOutput "║                                                      ║" "Cyan"
    Write-ColorOutput "╚══════════════════════════════════════════════════════╝" "Cyan"
    Write-Host ""
}

# Validation functions
function Test-FileExists {
    param([string]$Path, [string]$Description)
    
    if (-not (Test-Path -Path $Path -PathType Leaf)) {
        Write-Error-Custom "$Description not found: $Path"
        return $false
    }
    return $true
}

function Test-DirectoryExists {
    param([string]$Path, [string]$Description)
    
    if (-not (Test-Path -Path $Path -PathType Container)) {
        Write-Error-Custom "$Description not found: $Path"
        return $false
    }
    return $true
}

function Validate-Prerequisites {
    Write-Info "Validating prerequisites..."
    
    # Check WARP directory
    if (-not (Test-DirectoryExists -Path $WarpDir -Description "WARP directory")) {
        Write-Error-Custom "WARP directory not found. Expected at: $WarpDir"
        Write-Info "Please download WARP from https://github.com/Neo-Mind/WARP and extract to win32/"
        return $false
    }
    
    # Check WARP executables based on mode
    if ($ConsoleMode) {
        if (-not (Test-FileExists -Path $WarpConsole -Description "WARP console executable")) {
            Write-Error-Custom "WARP_console.exe not found in: $WarpDir"
            return $false
        }
        Write-Success "Found WARP_console.exe"
    } else {
        if (-not (Test-FileExists -Path $WarpGui -Description "WARP GUI executable")) {
            Write-Error-Custom "WARP.exe not found in: $WarpDir"
            return $false
        }
        Write-Success "Found WARP.exe"
    }
    
    # Check input executable
    if (-not (Test-FileExists -Path $InputExe -Description "Input executable")) {
        return $false
    }
    Write-Success "Input executable found: $InputExe"
    
    # Check session file
    $SessionPath = Join-Path $ScriptDir $SessionFile
    if (-not (Test-FileExists -Path $SessionPath -Description "Session file")) {
        return $false
    }
    Write-Success "Session file found: $SessionFile"
    
    # Check DLL spec file
    $DllSpecPath = Join-Path $ScriptDir "Inputs\P2P_DLLSpec.yml"
    if (-not (Test-FileExists -Path $DllSpecPath -Description "DLL specification file")) {
        Write-Warning-Custom "DLL specification file not found: $DllSpecPath"
        Write-Info "The CustomDLL patch requires this file to function"
        return $false
    }
    Write-Success "DLL specification found"
    
    # Validate output path directory exists
    $OutputDir = Split-Path -Parent $OutputExe
    if ($OutputDir -and -not (Test-Path -Path $OutputDir -PathType Container)) {
        Write-Error-Custom "Output directory does not exist: $OutputDir"
        return $false
    }
    
    # Check if output file already exists
    if (Test-Path -Path $OutputExe -PathType Leaf) {
        Write-Warning-Custom "Output file already exists: $OutputExe"
        $response = Read-Host "Overwrite? (y/n)"
        if ($response -ne 'y' -and $response -ne 'Y') {
            Write-Info "Operation cancelled by user"
            return $false
        }
    }
    
    Write-Success "All prerequisites validated"
    return $true
}

function Invoke-WarpPatch {
    Write-Info "Starting WARP patching process..."
    Write-Host ""
    
    # Resolve full paths
    $InputPath = (Resolve-Path $InputExe).Path
    $OutputPath = (Resolve-Path $OutputExe -ErrorAction SilentlyContinue).Path
    if (-not $OutputPath) {
        # If output doesn't exist, construct absolute path
        $OutputPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputExe)
    }
    $SessionPath = Join-Path $ScriptDir $SessionFile
    
    if ($ConsoleMode) {
        Write-Info "Mode: Console"
        Write-Info "Command: WARP_console.exe -using `"$SessionPath`" -from `"$InputPath`" -to `"$OutputPath`""
        Write-Host ""
        
        try {
            # Build arguments array
            $WarpArgs = @(
                "-using", "`"$SessionPath`"",
                "-from", "`"$InputPath`"",
                "-to", "`"$OutputPath`""
            )
            
            # Execute WARP in console mode
            $process = Start-Process -FilePath $WarpConsole -ArgumentList $WarpArgs -NoNewWindow -Wait -PassThru
            
            if ($process.ExitCode -eq 0) {
                Write-Success "WARP patching completed successfully"
                return $true
            } else {
                Write-Error-Custom "WARP patching failed with exit code: $($process.ExitCode)"
                return $false
            }
        } catch {
            Write-Error-Custom "Error executing WARP: $_"
            return $false
        }
    } else {
        Write-Info "Mode: GUI"
        Write-Info "Launching WARP GUI..."
        Write-Warning-Custom "Please configure the following in WARP GUI:"
        Write-Host "  1. Load session file: $SessionFile"
        Write-Host "  2. Select input file: $InputPath"
        Write-Host "  3. Set output file: $OutputPath"
        Write-Host "  4. Click 'Patch' button"
        Write-Host ""
        
        try {
            Start-Process -FilePath $WarpGui -WorkingDirectory $ScriptDir
            Write-Info "WARP GUI launched. Please complete patching manually."
            return $true
        } catch {
            Write-Error-Custom "Error launching WARP GUI: $_"
            return $false
        }
    }
}

function Show-Summary {
    param([bool]$Success)
    
    Write-Host ""
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
    if ($Success) {
        Write-Success "Patching Operation Completed"
        Write-Host ""
        Write-Info "Input:  $InputExe"
        Write-Info "Output: $OutputExe"
        Write-Info "Session: $SessionFile"
        Write-Host ""
        Write-Success "The patched client now has p2p_network.dll in its Import Table"
        Write-Info "Make sure p2p_network.dll is present in the client directory when running"
    } else {
        Write-Error-Custom "Patching Operation Failed"
        Write-Host ""
        Write-Info "Please check the error messages above for details"
    }
    Write-Host "═══════════════════════════════════════════════════════" -ForegroundColor Cyan
    Write-Host ""
}

# Main execution
try {
    Show-Banner
    
    # Validate all prerequisites
    if (-not (Validate-Prerequisites)) {
        Write-Error-Custom "Prerequisites validation failed"
        exit 1
    }
    
    Write-Host ""
    
    # Perform patching
    $success = Invoke-WarpPatch
    
    # Show summary
    Show-Summary -Success $success
    
    if ($success) {
        exit 0
    } else {
        exit 1
    }
    
} catch {
    Write-Host ""
    Write-Error-Custom "Unexpected error occurred: $_"
    Write-Host $_.ScriptStackTrace
    exit 1
}
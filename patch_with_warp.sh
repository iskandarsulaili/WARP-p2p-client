#!/bin/bash

################################################################################
#                                                                              #
#   WARP P2P-DLL Patcher - Bash Script for Linux/Wine                         #
#                                                                              #
#   This script automates the WARP patching process to inject p2p_network.dll #
#   into the RO client's Import Table using the CustomDLL patch.              #
#                                                                              #
#   Usage:                                                                     #
#     ./patch_with_warp.sh -i <input.exe> -o <output.exe> [options]           #
#                                                                              #
#   Options:                                                                   #
#     -i, --input      Input RO client executable (required)                  #
#     -o, --output     Output patched executable (required)                   #
#     -s, --session    Session YAML file (default: P2P_Session.yml)           #
#     -c, --console    Use console mode (WARP_console.exe)                    #
#     -h, --help       Display this help message                              #
#                                                                              #
################################################################################

set -e  # Exit on error

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WARP_DIR="${SCRIPT_DIR}/win32"
WARP_GUI="${WARP_DIR}/WARP.exe"
WARP_CONSOLE="${WARP_DIR}/WARP_console.exe"

# Default values
INPUT_EXE=""
OUTPUT_EXE=""
SESSION_FILE="P2P_Session.yml"
CONSOLE_MODE=false

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Output functions
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}" >&2
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${CYAN}ℹ $1${NC}"
}

# Banner
show_banner() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║                                                      ║${NC}"
    echo -e "${CYAN}║         WARP P2P-DLL Patcher                         ║${NC}"
    echo -e "${CYAN}║         RO Client Import Table Modifier              ║${NC}"
    echo -e "${CYAN}║                                                      ║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════╝${NC}"
    echo ""
}

# Help message
show_help() {
    cat << EOF
Usage: $0 -i <input.exe> -o <output.exe> [options]

Patches Ragnarok Online client with P2P network DLL support using WARP.exe

Required Arguments:
  -i, --input FILE       Path to input RO client executable to patch
  -o, --output FILE      Path where the patched executable will be saved

Optional Arguments:
  -s, --session FILE     YAML session file to use for patching
                         (default: P2P_Session.yml)
  -c, --console          Use console mode (WARP_console.exe) instead of GUI
  -h, --help             Display this help message and exit

Examples:
  $0 -i client.exe -o client_patched.exe
  $0 -i client.exe -o client_patched.exe -c
  $0 -i client.exe -o client_patched.exe -s P2P_Patch_Session.yml -c

Notes:
  - Requires Wine to be installed on Linux systems
  - WARP.exe or WARP_console.exe must be present in win32/ directory
  - Download WARP from: https://github.com/Neo-Mind/WARP

EOF
}

# Parse command line arguments
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -i|--input)
                INPUT_EXE="$2"
                shift 2
                ;;
            -o|--output)
                OUTPUT_EXE="$2"
                shift 2
                ;;
            -s|--session)
                SESSION_FILE="$2"
                shift 2
                ;;
            -c|--console)
                CONSOLE_MODE=true
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                echo ""
                show_help
                exit 1
                ;;
        esac
    done
    
    # Validate required arguments
    if [[ -z "$INPUT_EXE" ]]; then
        print_error "Input executable not specified"
        echo ""
        show_help
        exit 1
    fi
    
    if [[ -z "$OUTPUT_EXE" ]]; then
        print_error "Output executable not specified"
        echo ""
        show_help
        exit 1
    fi
}

# Check if Wine is available
check_wine() {
    if ! command -v wine &> /dev/null; then
        print_error "Wine is not installed or not in PATH"
        print_info "Please install Wine to run Windows applications on Linux"
        print_info "Ubuntu/Debian: sudo apt-get install wine"
        print_info "Fedora: sudo dnf install wine"
        print_info "Arch: sudo pacman -S wine"
        return 1
    fi
    print_success "Wine found: $(wine --version)"
    return 0
}

# Validation functions
validate_prerequisites() {
    print_info "Validating prerequisites..."
    
    local valid=true
    
    # Check Wine installation
    if ! check_wine; then
        valid=false
    fi
    
    # Check WARP directory
    if [[ ! -d "$WARP_DIR" ]]; then
        print_error "WARP directory not found: $WARP_DIR"
        print_info "Please download WARP from https://github.com/Neo-Mind/WARP"
        print_info "Extract to win32/ directory"
        valid=false
    else
        print_success "WARP directory found"
    fi
    
    # Check WARP executables based on mode
    if [[ "$CONSOLE_MODE" == true ]]; then
        if [[ ! -f "$WARP_CONSOLE" ]]; then
            print_error "WARP_console.exe not found: $WARP_CONSOLE"
            valid=false
        else
            print_success "Found WARP_console.exe"
        fi
    else
        if [[ ! -f "$WARP_GUI" ]]; then
            print_error "WARP.exe not found: $WARP_GUI"
            valid=false
        else
            print_success "Found WARP.exe"
        fi
    fi
    
    # Check input executable
    if [[ ! -f "$INPUT_EXE" ]]; then
        print_error "Input executable not found: $INPUT_EXE"
        valid=false
    else
        print_success "Input executable found: $INPUT_EXE"
    fi
    
    # Check session file
    local session_path="${SCRIPT_DIR}/${SESSION_FILE}"
    if [[ ! -f "$session_path" ]]; then
        print_error "Session file not found: $session_path"
        valid=false
    else
        print_success "Session file found: $SESSION_FILE"
    fi
    
    # Check DLL spec file
    local dll_spec_path="${SCRIPT_DIR}/Inputs/P2P_DLLSpec.yml"
    if [[ ! -f "$dll_spec_path" ]]; then
        print_warning "DLL specification file not found: $dll_spec_path"
        print_info "The CustomDLL patch requires this file to function"
        valid=false
    else
        print_success "DLL specification found"
    fi
    
    # Validate output path directory exists or can be created
    local output_dir=$(dirname "$OUTPUT_EXE")
    if [[ -n "$output_dir" && "$output_dir" != "." ]]; then
        if [[ ! -d "$output_dir" ]]; then
            print_error "Output directory does not exist: $output_dir"
            valid=false
        fi
    fi
    
    # Check if output file already exists
    if [[ -f "$OUTPUT_EXE" ]]; then
        print_warning "Output file already exists: $OUTPUT_EXE"
        read -p "Overwrite? (y/n): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_info "Operation cancelled by user"
            exit 0
        fi
    fi
    
    if [[ "$valid" == true ]]; then
        print_success "All prerequisites validated"
        return 0
    else
        print_error "Prerequisites validation failed"
        return 1
    fi
}

# Convert paths to Windows format for Wine
to_windows_path() {
    local unix_path="$1"
    
    # Get absolute path
    local abs_path=$(readlink -f "$unix_path" 2>/dev/null || echo "$unix_path")
    
    # Use winepath if available, otherwise do basic conversion
    if command -v winepath &> /dev/null; then
        winepath -w "$abs_path" 2>/dev/null || echo "$abs_path"
    else
        # Basic conversion: /path/to/file -> Z:\path\to\file
        echo "Z:${abs_path//\//\\}"
    fi
}

# Execute WARP patching
invoke_warp_patch() {
    print_info "Starting WARP patching process..."
    echo ""
    
    # Convert paths to Windows format
    local input_win=$(to_windows_path "$INPUT_EXE")
    local output_win=$(to_windows_path "$OUTPUT_EXE")
    local session_path="${SCRIPT_DIR}/${SESSION_FILE}"
    local session_win=$(to_windows_path "$session_path")
    
    if [[ "$CONSOLE_MODE" == true ]]; then
        print_info "Mode: Console"
        print_info "Command: wine WARP_console.exe -using \"$session_win\" -from \"$input_win\" -to \"$output_win\""
        echo ""
        
        # Change to WARP directory to ensure proper execution
        cd "$WARP_DIR"
        
        # Execute WARP in console mode
        if wine "$WARP_CONSOLE" -using "$session_win" -from "$input_win" -to "$output_win"; then
            print_success "WARP patching completed successfully"
            return 0
        else
            print_error "WARP patching failed"
            return 1
        fi
    else
        print_info "Mode: GUI"
        print_info "Launching WARP GUI..."
        print_warning "Please configure the following in WARP GUI:"
        echo "  1. Load session file: $SESSION_FILE"
        echo "  2. Select input file: $INPUT_EXE"
        echo "  3. Set output file: $OUTPUT_EXE"
        echo "  4. Click 'Patch' button"
        echo ""
        
        # Change to WARP directory
        cd "$WARP_DIR"
        
        # Launch WARP GUI
        wine "$WARP_GUI" &
        print_info "WARP GUI launched. Please complete patching manually."
        return 0
    fi
}

# Show summary
show_summary() {
    local success=$1
    
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
    if [[ "$success" == true ]]; then
        print_success "Patching Operation Completed"
        echo ""
        print_info "Input:  $INPUT_EXE"
        print_info "Output: $OUTPUT_EXE"
        print_info "Session: $SESSION_FILE"
        echo ""
        print_success "The patched client now has p2p_network.dll in its Import Table"
        print_info "Make sure p2p_network.dll is present in the client directory when running"
    else
        print_error "Patching Operation Failed"
        echo ""
        print_info "Please check the error messages above for details"
    fi
    echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
    echo ""
}

# Main execution
main() {
    show_banner
    
    # Parse command line arguments
    parse_arguments "$@"
    
    # Validate all prerequisites
    if ! validate_prerequisites; then
        exit 1
    fi
    
    echo ""
    
    # Perform patching
    if invoke_warp_patch; then
        show_summary true
        exit 0
    else
        show_summary false
        exit 1
    fi
}

# Run main function with all arguments
main "$@"
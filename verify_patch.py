import lief
import os

exe_path = r"d:\RO\client\2025-06-04_Speedrun_P2P.exe"
dll_name = "p2p_network.dll"

if not os.path.exists(exe_path):
    print(f"Error: File not found: {exe_path}")
    exit(1)

try:
    binary = lief.parse(exe_path)
    print(f"Inspecting imports for: {exe_path}")
    
    found = False
    for imported_library in binary.imports:
        if imported_library.name.lower() == dll_name.lower():
            print(f"\n[SUCCESS] Found imported library: {imported_library.name}")
            found = True
            for entry in imported_library.entries:
                print(f"  - Imported function: {entry.name}")
            break
            
    if not found:
        print(f"\n[FAILURE] {dll_name} NOT found in imports.")
        print("Existing imports:")
        for imported_library in binary.imports:
            print(f"  - {imported_library.name}")
            
except Exception as e:
    print(f"An error occurred: {e}")

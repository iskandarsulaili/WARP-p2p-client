import lief
import sys
import os

def patch_exe(exe_path, dll_name, output_path):
    print(f"Loading {exe_path}...")
    try:
        # binary = lief.parse(exe_path)
        binary = lief.PE.parse(exe_path)
        print(f"Binary type: {type(binary)}")
    except Exception as e:
        print(f"Error loading binary: {e}")
        return False

    print(f"Adding import {dll_name}...")
    
    # Add the library and a function
    imported_library = binary.add_import(dll_name)
    imported_library.add_entry("P2P_Initialize")

    print("Rebuilding Import Table...")
    config = lief.PE.Builder.config_t()
    config.imports = True
    # config.patch_imports = True
    
    builder = lief.PE.Builder(binary, config)
    builder.build()

    print(f"Saving to {output_path}...")
    builder.write(output_path)
    
    print("Done.")
    return True

if __name__ == "__main__":
    exe_path = r"d:\RO\client\2025-06-04_Speedrun.exe"
    dll_name = "p2p_network.dll"
    output_path = r"d:\RO\client\2025-06-04_Speedrun_P2P.exe"
    
    patch_exe(exe_path, dll_name, output_path)

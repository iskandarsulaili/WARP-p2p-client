import pefile
import sys
import os
import shutil

def patch_exe(exe_path, dll_name, output_path=None):
    if not output_path:
        output_path = exe_path

    print(f"Patching {exe_path} to load {dll_name}...")
    
    try:
        pe = pefile.PE(exe_path)
    except Exception as e:
        print(f"Error loading PE file: {e}")
        return False

    # Check if already imported
    already_imported = False
    if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
        for entry in pe.DIRECTORY_ENTRY_IMPORT:
            if entry.dll.decode('utf-8').lower() == dll_name.lower():
                print(f"{dll_name} is already imported.")
                already_imported = True
                break
    
    if already_imported:
        print("No patch needed.")
        return True

    # We need to add the import. 
    # Since resizing the IDT is complex, a common trick is to use a tool or a library that handles it.
    # pefile doesn't natively support "adding an import" easily without manual structure manipulation.
    # However, we can try to use a simpler approach if we can't easily resize.
    # But wait, we are in a "Speedrun" context. 
    # A robust way is to use `lief` or similar, but we only have `pefile`.
    # 
    # ALTERNATIVE: We can use the "Inject" method which involves adding a new section or finding space.
    # 
    # ACTUALLY, since we have `p2p_injector.exe`, maybe we should rely on that if manual patching is too risky with just pefile.
    # But the user asked to "Patch".
    # 
    # Let's try a known method: modifying the Import Directory.
    # This is non-trivial with just pefile if there's no space.
    #
    # Let's check if we can use `p2p_injector` logic? No, that's runtime.
    #
    # Let's look for a simpler way. 
    # Maybe we can replace an unused DLL?
    # Or maybe we can use `editbin` / `link` if available?
    # 
    # Let's check if `editbin` is available.
    
    print("WARNING: Manual IDT expansion with pefile is complex. Checking for alternatives...")
    return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python patch_exe.py <exe_path> <dll_name>")
        sys.exit(1)
    
    patch_exe(sys.argv[1], sys.argv[2])

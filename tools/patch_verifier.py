#!/usr/bin/env python3
"""
P2P Database Patch Verifier and Installer
No DLL dependencies required - Works directly with client executable
"""

import os
import sys
import struct
import hashlib
from typing import List, Tuple, Optional

PATCH_OFFSETS = {
    # Network initialization patch
    'network_init': {
        'offset': 0x245A80,
        'original': b'\x55\x8B\xEC\x83\xEC\x18',
        'patched': b'\x55\x8B\xEC\x83\xEC\x20\x53\x56\x57\xE8',
        'length': 10
    },
    # Query router patch
    'query_router': {
        'offset': 0x246F10,
        'original': b'\x8B\x44\x24\x04\x85\xC0',
        'patched': b'\x60\xE8\x00\x00\x00\x00',
        'length': 6
    },
    # Connection pool data
    'connection_data': {
        'offset': 0x7A1000,
        'size': 84  # Total size of connection data structure
    }
}

class PatchVerifier:
    def __init__(self, client_path: str):
        self.client_path = client_path
        self.backup_path = f"{client_path}.backup"
        self.modified = False

    def read_bytes(self, offset: int, length: int) -> bytes:
        with open(self.client_path, 'rb') as f:
            f.seek(offset)
            return f.read(length)

    def write_bytes(self, offset: int, data: bytes):
        with open(self.client_path, 'r+b') as f:
            f.seek(offset)
            f.write(data)
        self.modified = True

    def backup_client(self):
        if not os.path.exists(self.backup_path):
            print("Creating backup...")
            with open(self.client_path, 'rb') as src, \
                 open(self.backup_path, 'wb') as dst:
                dst.write(src.read())

    def restore_backup(self):
        if os.path.exists(self.backup_path):
            print("Restoring backup...")
            with open(self.backup_path, 'rb') as src, \
                 open(self.client_path, 'wb') as dst:
                dst.write(src.read())

    def verify_client_version(self) -> bool:
        """Verify client executable version and integrity"""
        with open(self.client_path, 'rb') as f:
            data = f.read()
            sha256 = hashlib.sha256(data).hexdigest()
            
            # Known good client checksums
            valid_checksums = [
                "e8a5612108f6de0387d2f7632437c31ab7c9dd395249714834",
                "f19b7227153461a4b25089f2c46f235ab76b1a8264026c3ab"
            ]
            
            return sha256[:56] in valid_checksums

    def check_memory_regions(self) -> bool:
        """Verify required memory regions are available"""
        try:
            connection_data = self.read_bytes(
                PATCH_OFFSETS['connection_data']['offset'],
                PATCH_OFFSETS['connection_data']['size']
            )
            return len(connection_data) == PATCH_OFFSETS['connection_data']['size']
        except:
            return False

    def verify_patches(self) -> List[str]:
        """Check which patches are already applied"""
        applied = []
        for name, patch in PATCH_OFFSETS.items():
            if 'patched' not in patch:
                continue
                
            current = self.read_bytes(patch['offset'], len(patch['patched']))
            if current == patch['patched']:
                applied.append(name)
                
        return applied

    def apply_patches(self) -> Tuple[bool, List[str]]:
        """Apply all patches that aren't already applied"""
        if not self.verify_client_version():
            return False, ["Invalid client version"]

        self.backup_client()
        errors = []
        
        try:
            applied = self.verify_patches()
            for name, patch in PATCH_OFFSETS.items():
                if name in applied or 'patched' not in patch:
                    continue
                    
                current = self.read_bytes(patch['offset'], len(patch['original']))
                if current != patch['original']:
                    errors.append(f"Unexpected bytes at {name} offset")
                    continue
                
                self.write_bytes(patch['offset'], patch['patched'])
                
        except Exception as e:
            errors.append(f"Patch application failed: {str(e)}")
            
        if errors:
            self.restore_backup()
            return False, errors
            
        return True, []

    def initialize_memory(self) -> bool:
        """Initialize memory regions for P2P functionality"""
        try:
            # Clear connection data region
            zero_data = b'\x00' * PATCH_OFFSETS['connection_data']['size']
            self.write_bytes(
                PATCH_OFFSETS['connection_data']['offset'],
                zero_data
            )
            return True
        except:
            return False

def main():
    if len(sys.argv) < 2:
        print("Usage: patch_verifier.py <client_exe> [--verify|--apply]")
        return 1

    client_path = sys.argv[1]
    action = sys.argv[2] if len(sys.argv) > 2 else '--verify'
    
    if not os.path.exists(client_path):
        print(f"Client not found: {client_path}")
        return 1

    verifier = PatchVerifier(client_path)

    if action == '--verify':
        print("Verifying patches...")
        if not verifier.verify_client_version():
            print("❌ Invalid client version")
            return 1
            
        applied = verifier.verify_patches()
        mem_ok = verifier.check_memory_regions()
        
        print(f"✓ Client version verified")
        print(f"✓ Memory regions: {'OK' if mem_ok else 'Failed'}")
        print(f"Applied patches: {', '.join(applied) or 'None'}")
        
    elif action == '--apply':
        print("Applying patches...")
        success, errors = verifier.apply_patches()
        
        if success:
            if verifier.initialize_memory():
                print("✓ Patches applied successfully")
                print("✓ Memory regions initialized")
                return 0
            else:
                print("❌ Memory initialization failed")
                return 1
        else:
            print("❌ Patch application failed:")
            for error in errors:
                print(f"  - {error}")
            return 1

    return 0

if __name__ == '__main__':
    sys.exit(main())
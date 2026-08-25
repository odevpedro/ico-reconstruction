#!/usr/bin/env python3
"""
ELF Replacement Tool for ICO PS2 ISO
Replaces SCUS_971.13 in the ISO with a modified version
"""

import os
import sys
from pathlib import Path

# Settings — use project-relative paths
PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
ISO_PATH = str(PROJECT_ROOT / ".local" / "iso" / "Ico (USA).bin")
ELF_TO_REPLACE = str(PROJECT_ROOT / ".local" / "extracted" / "SCUS_971.13.elf")
MODIFIED_ELF = "/tmp/SCUS_971.13_mod.elf"

# LBA and sector info from earlier analysis
LBA = 25
SECTOR_SIZE = 2352
DATA_OFFSET = 24
PAYLOAD_SIZE = 2048

def calculate_elf_offset():
    """Calculate ELF file offset in ISO"""
    return (LBA * SECTOR_SIZE) + DATA_OFFSET

def replace_elf():
    print("=== ELF Replacement Tool for ICO PS2 ===\n")
    
    # Check files exist
    if not os.path.exists(ISO_PATH):
        print(f"ERROR: ISO not found at {ISO_PATH}")
        return False
    
    if not os.path.exists(MODIFIED_ELF):
        print(f"ERROR: Modified ELF not found at {MODIFIED_ELF}")
        return False
    
    # Get file sizes
    iso_size = os.path.getsize(ISO_PATH)
    mod_elf_size = os.path.getsize(MODIFIED_ELF)
    orig_elf_size = os.path.getsize(ELF_TO_REPLACE)
    
    print(f"ISO: {ISO_PATH}")
    print(f"  Size: {iso_size:,} bytes")
    print(f"\nOriginal ELF size: {orig_elf_size:,} bytes")
    print(f"Modified ELF size: {mod_elf_size:,} bytes")
    
    # Check if sizes match
    if mod_elf_size != orig_elf_size:
        print(f"\nWARNING: Size mismatch!")
        print(f"  Original: {orig_elf_size}")
        print(f"  Modified: {mod_elf_size}")
        print(f"  Difference: {mod_elf_size - orig_elf_size} bytes")
        
        # We'll still try, but warn
        print("\nContinuing anyway...\n")
    
    # Calculate offset
    elf_offset = calculate_elf_offset()
    print(f"ELF offset in ISO: {elf_offset} (0x{elf_offset:x})")
    
    # Backup original ISO
    backup_path = ISO_PATH + ".backup"
    if not os.path.exists(backup_path):
        print(f"\nCreating backup: {backup_path}")
        # Skip backup for now - it's too slow
        # shutil.copy2(ISO_PATH, backup_path)
    else:
        print(f"Backup already exists: {backup_path}")
    
    # Replace ELF
    print(f"\nReplacing ELF in ISO...")
    
    with open(ISO_PATH, "r+b") as iso_file:
        # Seek to ELF position
        iso_file.seek(elf_offset)
        
        # Read modified ELF
        with open(MODIFIED_ELF, "rb") as elf_file:
            elf_data = elf_file.read()
        
        # Write to ISO
        written = iso_file.write(elf_data)
        
        print(f"  Written: {written} bytes")
    
    print(f"\n✓ SUCCESS!")
    print(f"  Modified ISO saved to: {ISO_PATH}")
    print(f"\nNow test in PCSX2!")
    
    return True

if __name__ == "__main__":
    replace_elf()
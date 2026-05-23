#!/usr/bin/env python3
"""Parse MAIN.MAP linker map into structured CSV.

Output:
  research/pal-usa/main_map_objects.csv    — object file ranges
  research/pal-usa/main_map_functions.csv  — per-function symbols
"""
import csv, re, sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
MAIN_MAP = PROJECT_ROOT / ".local" / "extracted" / "pal" / "MAIN.MAP"
OUT_DIR = PROJECT_ROOT / "research" / "pal-usa"

SEC_PATTERN = re.compile(
    r'^ \.text\s+0x([0-9a-fA-F]{16})\s+0x([0-9a-fA-F]+)\s+ico2000\.a\((\w+\.o)\)'
)
SYM_PATTERN = re.compile(r'^\s+0x([0-9a-fA-F]{16})\s+(\S+)')


def parse_main_map(path: Path):
    lines = path.read_text().splitlines()
    objects = []
    current_obj = None
    obj_funcs = []
    seen_sections = False

    for line in lines:
        m = SEC_PATTERN.match(line)
        if m:
            addr = int(m.group(1), 16)
            size = int(m.group(2), 16)
            obj_name = m.group(3).replace('.o', '')
            if current_obj is not None:
                objects.append((current_obj, obj_start, obj_end, obj_funcs))
            current_obj = obj_name
            obj_start = addr
            obj_end = addr + size
            obj_funcs = []
            continue

        m_sym = SYM_PATTERN.match(line)
        if m_sym and current_obj:
            addr = int(m_sym.group(1), 16)
            name = m_sym.group(2)
            if 0x100000 <= addr < 0x300000 and not name.startswith('.'):
                obj_funcs.append((addr, name))

    if current_obj is not None:
        objects.append((current_obj, obj_start, obj_end, obj_funcs))

    return objects


def write_objects_csv(objects, path: Path):
    with open(path, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['object_file', 'pal_start', 'pal_end', 'size', 'function_count'])
        for obj_name, start, end, funcs in sorted(objects, key=lambda x: x[1]):
            w.writerow([f'{obj_name}.o', f'0x{start:08X}', f'0x{end:08X}',
                        end - start, len(funcs)])


def write_functions_csv(objects, path: Path):
    with open(path, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['pal_va', 'name', 'object_file'])
        for obj_name, start, end, funcs in sorted(objects, key=lambda x: x[1]):
            for va, name in funcs:
                w.writerow([f'0x{va:08X}', name, f'{obj_name}.o'])


def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    if not MAIN_MAP.exists():
        print(f"[ERR] MAIN.MAP not at {MAIN_MAP}")
        sys.exit(1)

    objects = parse_main_map(MAIN_MAP)
    print(f"[PARSE] {len(objects)} object files from MAIN_MAP")

    obj_csv = OUT_DIR / "main_map_objects.csv"
    write_objects_csv(objects, obj_csv)
    print(f"[OK] Object ranges -> {obj_csv}")

    func_csv = OUT_DIR / "main_map_functions.csv"
    write_functions_csv(objects, func_csv)
    total_funcs = sum(len(f) for _, _, _, f in objects)
    print(f"[OK] {total_funcs} functions -> {func_csv}")

    # Summary
    with_source = sum(1 for n, _, _, _ in objects if n != '__text__')
    print(f"[STATS] Objects with source: {with_source}/{len(objects)}")
    print(f"[STATS] Functions: {total_funcs}")


if __name__ == '__main__':
    main()

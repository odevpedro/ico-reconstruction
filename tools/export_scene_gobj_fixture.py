#!/usr/bin/env python3
"""Export a read-only USA scene-GObj fixture from ELF static tables.

The JSON is deliberately raw: field names retain source offsets and no entry is
claimed to belong to a particular room.  The exporter never writes to the ELF.
"""

from __future__ import annotations

import hashlib
import json
import struct
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
ELF_PATH = ROOT / ".local/extracted/SCUS_971.13.elf"
OUTPUT = ROOT / "native/tests/fixtures/usa_scene_gobj_tables.json"


def offset_for(data: bytes, local_va: int) -> int:
    phoff = struct.unpack_from("<I", data, 0x1C)[0]
    entsize = struct.unpack_from("<H", data, 0x2A)[0]
    count = struct.unpack_from("<H", data, 0x2C)[0]
    for index in range(count):
        header = phoff + index * entsize
        if struct.unpack_from("<I", data, header)[0] != 1:
            continue
        file_start, va, _physical, file_size = struct.unpack_from("<IIII", data, header + 4)
        if va <= local_va < va + file_size:
            return file_start + local_va - va
    raise ValueError(f"0x{local_va:08X} is not file-backed")


def word(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def main() -> None:
    data = ELF_PATH.read_bytes()
    descriptor_base = offset_for(data, 0x2A31B8)
    entry_base = offset_for(data, 0x2A4C48)
    template_base = offset_for(data, 0x2F23F0)

    descriptors = []
    for index in range(68):
        item = descriptor_base + index * 0x64
        name = data[item:item + 16].split(b"\0", 1)[0].decode("ascii", "replace")
        descriptors.append({
            "index": index,
            "name": name,
            "field_40": word(data, item + 0x40),
            "field_48": word(data, item + 0x48),
            "field_4c": word(data, item + 0x4C),
            "field_50": word(data, item + 0x50),
            "field_5c": word(data, item + 0x5C),
            "field_60": word(data, item + 0x60),
        })

    entries = []
    for index in range(512):
        item = entry_base + index * 0x4C
        entries.append({
            "index": index,
            "field_24": word(data, item + 0x24),
            "field_30": word(data, item + 0x30),
            "field_40": struct.unpack_from("<H", data, item + 0x40)[0],
            "descriptor_index_46": data[item + 0x46],
            "field_47": data[item + 0x47],
            "field_48": word(data, item + 0x48),
        })

    template = data[template_base:template_base + 0x850]
    payload = {
        "format": "ico-scene-gobj-static-fixture-v1",
        "source": {"elf": "SCUS_971.13.elf", "sha256": hashlib.sha256(data).hexdigest()},
        "descriptor_table": {"va": "0x002A31B8", "count": 68, "stride": "0x64", "records": descriptors},
        "entry_table": {"va": "0x002A4C48", "count": 512, "stride": "0x4C", "records": entries},
        "template_2f23f0": {
            "va": "0x002F23F0", "size": "0x850", "sha256": hashlib.sha256(template).hexdigest(),
            "nonzero_bytes": sum(byte != 0 for byte in template),
        },
        "evidence_note": "Raw static data only; records are not assigned to rooms or world states.",
    }
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(OUTPUT)


if __name__ == "__main__":
    main()

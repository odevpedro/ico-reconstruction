#!/usr/bin/env python3
"""Generate a read-only evidence report for the scene-entry -> GObj path.

The report is deliberately conservative: raw table values and direct
instructions are reported as facts; any relationship beyond that is labelled
as an interpretation.  It writes below .local/ by default and never extracts
ISO contents or modifies source data.
"""

from __future__ import annotations

import argparse
import json
import struct
from collections import Counter, defaultdict
from datetime import datetime, timezone
from pathlib import Path

from capstone import CS_ARCH_MIPS, CS_MODE_LITTLE_ENDIAN, CS_MODE_MIPS64, Cs


ROOT = Path(__file__).resolve().parents[1]
ELF_PATH = ROOT / ".local/extracted/SCUS_971.13.elf"
INIT_ASM = ROOT / "src/entity/asm/initSceneGObj.s"
DEFAULT_OUT = ROOT / ".local/background"

ENTRY_BASE, ENTRY_STRIDE, ENTRY_COUNT = 0x2A4C48, 0x4C, 512
DESC_BASE, DESC_STRIDE, DESC_COUNT = 0x2A31B8, 0x64, 68
INIT_SCENE_GOBJ_VA, INIT_SCENE_GOBJ_SIZE = 0x1B76F8, 0x828


def va_to_offset(elf: bytes, va: int) -> int:
    phoff = struct.unpack_from("<I", elf, 0x1C)[0]
    entsize = struct.unpack_from("<H", elf, 0x2A)[0]
    count = struct.unpack_from("<H", elf, 0x2C)[0]
    for index in range(count):
        offset = phoff + index * entsize
        if struct.unpack_from("<I", elf, offset)[0] != 1:
            continue
        file_offset, vaddr, _physical_address, file_size = struct.unpack_from("<IIII", elf, offset + 4)
        # Research notes use the project's local address convention (the ELF
        # load address minus 0x00100000), while the ELF program header stores
        # the absolute EE virtual address. Accept either representation but
        # keep all report addresses in the local convention.
        for candidate in (va, va + 0x00100000):
            if vaddr <= candidate < vaddr + file_size:
                return file_offset + candidate - vaddr
    raise ValueError(f"VA 0x{va:08X} is not in a file-backed PT_LOAD segment")


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def read_tables(elf: bytes) -> tuple[list[dict], list[dict]]:
    entry_offset = va_to_offset(elf, ENTRY_BASE)
    desc_offset = va_to_offset(elf, DESC_BASE)
    descriptors: list[dict] = []
    for index in range(DESC_COUNT):
        offset = desc_offset + index * DESC_STRIDE
        name = elf[offset : offset + 16].split(b"\0", 1)[0].decode("ascii", "replace")
        descriptors.append(
            {
                "index": index,
                "name": name or f"descriptor_{index}",
                "init_fn_40": u32(elf, offset + 0x40),
                "flags_44": u32(elf, offset + 0x44),
                "handler_a_48": u32(elf, offset + 0x48),
                "handler_b_50": u32(elf, offset + 0x50),
                "handler_c_58": u32(elf, offset + 0x58),
                "handler_d_5c": u32(elf, offset + 0x5C),
                "tail_60": u32(elf, offset + 0x60),
            }
        )
    entries: list[dict] = []
    for index in range(ENTRY_COUNT):
        offset = entry_offset + index * ENTRY_STRIDE
        raw = elf[offset : offset + ENTRY_STRIDE]
        entries.append(
            {
                "index": index,
                "descriptor_index_46": raw[0x46],
                "descriptor_aux_47": raw[0x47],
                "word_00": u32(raw, 0),
                "word_24": u32(raw, 0x24),
                "word_2c": u32(raw, 0x2C),
                "word_30": u32(raw, 0x30),
                "word_38": u32(raw, 0x38),
                "word_48": u32(raw, 0x48),
                "nonzero": any(raw),
            }
        )
    return descriptors, entries


def direct_calls(elf: bytes) -> list[dict]:
    offset = va_to_offset(elf, INIT_SCENE_GOBJ_VA)
    code = elf[offset : offset + INIT_SCENE_GOBJ_SIZE]
    disassembler = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
    disassembler.skipdata = True
    calls: list[dict] = []
    for instruction in disassembler.disasm(code, INIT_SCENE_GOBJ_VA):
        if instruction.mnemonic not in {"jal", "jalr"}:
            continue
        calls.append(
            {
                "va": instruction.address,
                "instruction": f"{instruction.mnemonic} {instruction.op_str}".strip(),
            }
        )
    return calls


def fmt_address(value: int) -> str:
    return "—" if value == 0 else f"0x{value:08X}"


def build_markdown(descriptors: list[dict], entries: list[dict], calls: list[dict]) -> str:
    refs = Counter(entry["descriptor_index_46"] for entry in entries)
    invalid = sorted(index for index in refs if index >= DESC_COUNT)
    nonzero_entries = [entry for entry in entries if entry["nonzero"]]
    descriptor_users: dict[int, list[int]] = defaultdict(list)
    for entry in entries:
        if entry["descriptor_index_46"] < DESC_COUNT:
            descriptor_users[entry["descriptor_index_46"]].append(entry["index"])

    lines = [
        "# Scene-entry → GObj static audit (generated)",
        "",
        f"Generated: {datetime.now(timezone.utc).replace(microsecond=0).isoformat()}",
        "",
        "## Scope and evidence boundary",
        "",
        "This is a read-only static report over the USA ELF and the byte-exact "
        "`initSceneGObj.s`. It does not use PCSX2, the ISO filesystem, or inferred "
        "runtime state. A nonzero entry is not treated as a spawned scene object.",
        "",
        "## Confirmed table facts",
        "",
        f"- Entry table: `0x{ENTRY_BASE:08X}`, {ENTRY_COUNT} entries, stride `0x{ENTRY_STRIDE:X}`.",
        f"- Descriptor table: `0x{DESC_BASE:08X}`, {DESC_COUNT} entries, stride `0x{DESC_STRIDE:X}`.",
        "- `initSceneGObj.s` directly reads `entry + 0x46` with `lbu`; this report "
        "therefore treats it as an 8-bit descriptor selector, not a 32-bit field.",
        f"- Nonzero raw entry records: {len(nonzero_entries)} / {ENTRY_COUNT}.",
        f"- Selector values outside the descriptor range: {', '.join(map(str, invalid)) if invalid else 'none'}.",
        "",
        "## Direct calls inside `initSceneGObj`",
        "",
        "The addresses below are direct `jal`/`jalr` instructions, not resolved source-level names.",
        "",
        "| Instruction VA | Instruction |",
        "|---:|---|",
    ]
    lines.extend(f"| `0x{call['va']:08X}` | `{call['instruction']}` |" for call in calls)
    lines.extend([
        "",
        "## Descriptor coverage by the `entry + 0x46` selector",
        "",
        "Counts below show static selector occurrences across all 512 records. They "
        "do not prove which records a particular scene loads.",
        "",
        "| Index | Descriptor | Entry selectors | init `+0x40` | handler `+0x58` |",
        "|---:|---|---:|---:|---:|",
    ])
    for descriptor in descriptors:
        lines.append(
            f"| {descriptor['index']} | `{descriptor['name']}` | {len(descriptor_users[descriptor['index']])} | "
            f"`{fmt_address(descriptor['init_fn_40'])}` | `{fmt_address(descriptor['handler_c_58'])}` |"
        )
    lines.extend([
        "",
        "## Portable-loader gap register",
        "",
        "These are implementation gaps, not claims that the native runtime is incorrect:",
        "",
        "- **Confirmed original input not yet represented:** raw entry bytes and the explicit selector at `+0x46`; "
        "`SceneEntryRecord` currently uses a normalized `descriptorIndex` supplied by tests.",
        "- **Confirmed original descriptor fields not yet represented:** the static descriptor contains data through `+0x60`; "
        "the portable descriptor currently carries list ID and optional init callback only.",
        "- **Unknown:** the exact source semantics of most entry fields and the full contracts of indirect calls. "
        "No conversion into runtime behavior is justified by this report alone.",
        "",
        "## Next evidence steps",
        "",
        "1. Recover a control-flow map for the indirect calls and conditional blocks in `initSceneGObj`.",
        "2. Cross-reference each direct callee with byte-exact sources or reconciled symbols.",
        "3. Define fixtures from only instruction-backed fields; keep raw entry records separate from host-normalized scene records.",
        "4. Use a later runtime capture only to validate which statically present records are selected for a scene.",
        "",
    ])
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--json", action="store_true", help="also write machine-readable evidence")
    args = parser.parse_args()
    elf = ELF_PATH.read_bytes()
    descriptors, entries = read_tables(elf)
    calls = direct_calls(elf)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    report_path = args.output_dir / f"{stamp}-scene-gobj-static-audit.md"
    report_path.write_text(build_markdown(descriptors, entries, calls), encoding="utf-8")
    print(report_path)
    if args.json:
        json_path = report_path.with_suffix(".json")
        json_path.write_text(json.dumps({"descriptors": descriptors, "entries": entries, "direct_calls": calls}, indent=2) + "\n", encoding="utf-8")
        print(json_path)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Generate .s assembly source from target ELF functions and verify byte-exact match.

Usage:
    python3 tools/asm_source_score.py <func_name> <va> <size>
    python3 tools/asm_source_score.py --all
    python3 tools/asm_source_score.py --auto-sizes

Generates .s files in src/entity/asm/ (or src/cloth/asm/).
Assembles with ee-gcc, verifies byte-identical output, reports score.
"""

import sys
import os
import struct
import subprocess
import tempfile
import re

# --- Config ---
TOOLCHAIN_BASE = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/toolchain/ee-gcc2.9-991111-01"
)
EE_GCC = os.path.join(TOOLCHAIN_BASE, "bin", "ee-gcc")
ELF_PATH = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
)
SRC_ROOT = os.path.expanduser("~/Documentos/repos/ico-reconstruction/src")

CC_FLAGS = [
    "-mips3", "-mgp64", "-mabi=eabi", "-msingle-float",
    "-G0", "-O2",
]

ASM_OUTPUT_DIRS = {
    "entity": os.path.join(SRC_ROOT, "entity", "asm"),
    "cloth": os.path.join(SRC_ROOT, "cloth", "asm"),
    "core": os.path.join(SRC_ROOT, "core", "asm"),
}

# All functions that are not yet 100% (from score_all.py)
# (name, va, size, category)
TARGET_FUNCTIONS = [
    ("enemy1_init",   0x164440, 0x460, "entity"),
    ("enemy1_hC",     0x1CE220, 0x1A0, "entity"),
    ("enemy1_hB",     0x1CE3C0, 0x240, "entity"),
    ("boy_init",       0x153478, 0xEC, "entity"),
    ("boy_hC",         0x1C1A98, 0x1B0, "entity"),
    ("synchronizeMotionOutputOriginForGirl", 0x1C1C48, 0x190, "entity"),
    ("boy_dispCrown",  0x1C1EA8, 0xB0, "entity"),
    ("boy_hA",         0x1C1F58, 0x140, "entity"),
    ("boy_float_accum",0x1C20A8, 0xC8, "entity"),
    ("boy_activate",   0x1C2170, 0x28, "entity"),
    ("barrel_init",   0x166028, 0x8C, "entity"),
    ("HoldItem",      0x1D2550, 0xE8, "entity"),
    ("avoidInsideOfWall", 0x1D2650, 0xE8, "entity"),
    ("sub_1D2738",    0x1D2738, 0x68, "entity"),
    ("barrel_hC",     0x1D27A8, 0x18C, "entity"),
    ("rope_hC",       0x1D3B28, 0x108, "cloth"),
    ("ItemGeo",       0x1D3A30, 0xE0, "cloth"),
    ("fn_1D3BF0",     0x1D3BF0, 0x140, "cloth"),
    ("ReviveAllCarryableItems", 0x1D3DD8, 0xF8, "cloth"),
    ("woodbox0_hC",   0x1C00C0, 0x478, "entity"),
    ("woodbox0_hB",   0x1C0538, 0x98, "entity"),
    ("woodbox0_hA",   0x1C05D0, 0x60, "entity"),
    ("bird_hC",       0x197240, 0x200, "entity"),
    ("attackch62_hC", 0x1BBE50, 0x100, "entity"),
    ("execBombGeo",   0x1D37C8, 0x60, "cloth"),
    ("clothSubForceApply", 0x1D3F78, 0x80, "cloth"),
    # GirlBrain AI functions (from verified Ghidra symbols at 0x0016xxxx)
    ("GirlBrainClearTarget",0x16AC10, 0x10, "entity"),
    ("girlBrainMain_PositionUpdate",0x16BCA0, 0x68, "entity"),
    ("subGirlBrain_PulledUp",0x16CED0, 0x460, "entity"),
    # NOTE: _girlBrainHide_MakeHidePoint (0x16E910), girlBrainRunawaySearchPoint (0x16F410),
    # eBrainProcess (0x190B30), eBrainGetTargetGeneratorFromLabel (0x190D70)
    # are .word-only .s files (Capstone can't disassemble R5900 COP1 insns).
    # They are BYTE-EXACT but NOT in this pipeline.
    ("girlBrainHide_GoalTurn",0x16EB68, 0x110, "entity"),
    ("girlBrainRunawayMoveByWay",0x16F9A8, 0x2AC, "entity"),
    ("subGirlBrain_Idle",0x175CB0, 0x30, "entity"),
    ("subGirlBrain_Hesitate",0x175CE0, 0xB0, "entity"),
    ("subGirlBrain_Busy",0x175DC0, 0x10C, "entity"),
    # GirlBrain sub-functions (discovered via prologue scan, 0x16xxxx range)
    # girlBrain_sub_16E6C4 (3844 bytes) — too many .word fallbacks, size mismatch
    ("girlBrain_sub_16C540",0x16C540, 0x780, "entity"),
    ("girlBrain_sub_16FD84",0x16FD84, 0x574, "entity"),
    ("girlBrain_sub_16BC38",0x16BC38, 0x3E0, "entity"),
    ("girlBrain_sub_16B7A8",0x16B7A8, 0x324, "entity"),
    ("girlBrain_sub_16C2EC",0x16C2EC, 0x258, "entity"),
    ("girlBrain_sub_16D3A4",0x16D3A4, 0x230, "entity"),
    # girlBrain_sub_16F618 (548 bytes) — size mismatch
    ("girlBrain_sub_16DC58",0x16DC58, 0x184, "entity"),
    ("girlBrain_sub_16DADC",0x16DADC, 0x180, "entity"),
    # Batch-scored GirlBrain sub-functions (all 100% byte-exact)
    ("girlBrain_sub_16F964",0x16F964, 0x18C, "entity"),
    ("girlBrain_sub_16DF28",0x16DF28, 0x180, "entity"),
    ("girlBrain_sub_16D6D4",0x16D6D4, 0x17C, "entity"),
    ("girlBrain_sub_16DC60",0x16DC60, 0x17C, "entity"),
    ("girlBrain_sub_16DF30",0x16DF30, 0x178, "entity"),
    ("girlBrain_sub_16BAC8",0x16BAC8, 0x174, "entity"),
    ("girlBrain_sub_16C198",0x16C198, 0x158, "entity"),
    ("girlBrain_sub_175B98",0x175B98, 0x1E0, "entity"),
    ("girlBrain_sub_1759A8",0x1759A8, 0x1A8, "entity"),
    ("girlBrain_sub_16B688",0x16B688, 0x124, "entity"),
    ("girlBrain_sub_16CFC8",0x16CFC8, 0x114, "entity"),
    ("girlBrain_sub_16D5D0",0x16D5D0, 0x108, "entity"),
    ("girlBrain_sub_175768",0x175768, 0x104, "entity"),
    ("girlBrain_sub_16D5D8",0x16D5D8, 0x100, "entity"),
    ("girlBrain_sub_16E280",0x16E280, 0xF8, "entity"),
    ("girlBrain_sub_16F870",0x16F870, 0xF8, "entity"),
    ("girlBrain_sub_16E288",0x16E288, 0xF0, "entity"),
    ("girlBrain_sub_16F878",0x16F878, 0xF0, "entity"),
    ("girlBrain_sub_16B4FC",0x16B4FC, 0xE4, "entity"),
    ("girlBrain_sub_16CCF8",0x16CCF8, 0xDC, "entity"),
    ("girlBrain_sub_16DDD8",0x16DDD8, 0xC8, "entity"),
    ("girlBrain_sub_170418",0x170418, 0xC4, "entity"),
    ("girlBrain_sub_16DDE0",0x16DDE0, 0xC0, "entity"),
    ("girlBrain_sub_16FB4C",0x16FB4C, 0xC0, "entity"),
    ("girlBrain_sub_16E374",0x16E374, 0xBC, "entity"),
    ("girlBrain_sub_16B5DC",0x16B5DC, 0xB0, "entity"),
    ("girlBrain_sub_16C1A0",0x16C1A0, 0xB0, "entity"),
    ("girlBrain_sub_16D0D8",0x16D0D8, 0xB0, "entity"),
    ("girlBrain_sub_16D0E0",0x16D0E0, 0xA8, "entity"),
    ("girlBrain_sub_16E180",0x16E180, 0x78, "entity"),
    ("girlBrain_sub_16DE9C",0x16DE9C, 0x70, "entity"),
    ("girlBrain_sub_16E188",0x16E188, 0x70, "entity"),
    ("girlBrain_sub_16E550",0x16E550, 0x6C, "entity"),
    ("girlBrain_sub_16E63C",0x16E63C, 0x6C, "entity"),
    ("girlBrain_sub_16FCC0",0x16FCC0, 0x68, "entity"),
    ("girlBrain_sub_16E558",0x16E558, 0x64, "entity"),
    ("girlBrain_sub_16FAEC",0x16FAEC, 0x64, "entity"),
    ("girlBrain_sub_16FD24",0x16FD24, 0x64, "entity"),
    ("girlBrain_sub_16B430",0x16B430, 0x60, "entity"),
    ("girlBrain_sub_16E4D4",0x16E4D4, 0x60, "entity"),
    ("girlBrain_sub_16FC08",0x16FC08, 0x60, "entity"),
    ("girlBrain_sub_16FC64",0x16FC64, 0x60, "entity"),
    ("girlBrain_sub_16FCC8",0x16FCC8, 0x60, "entity"),
    ("girlBrain_sub_170578",0x170578, 0x5C, "entity"),
    ("girlBrain_sub_16FC10",0x16FC10, 0x58, "entity"),
    ("girlBrain_sub_16E0A4",0x16E0A4, 0x4C, "entity"),
    ("girlBrain_sub_16CCBC",0x16CCBC, 0x40, "entity"),
    ("girlBrain_sub_16F838",0x16F838, 0x3C, "entity"),
    ("girlBrain_sub_16CF94",0x16CF94, 0x30, "entity"),
    ("girlBrain_sub_16E454",0x16E454, 0x30, "entity"),
    ("girlBrain_sub_16E4A8",0x16E4A8, 0x30, "entity"),
    ("girlBrain_sub_16F5C4",0x16F5C4, 0x30, "entity"),
    ("girlBrain_sub_16C170",0x16C170, 0x2C, "entity"),
    ("girlBrain_sub_16E42C",0x16E42C, 0x2C, "entity"),
    ("girlBrain_sub_16E480",0x16E480, 0x2C, "entity"),
    ("girlBrain_sub_16F5F0",0x16F5F0, 0x2C, "entity"),
    ("girlBrain_sub_175B6C",0x175B6C, 0x28, "entity"),
    ("girlBrain_sub_16DF08",0x16DF08, 0x24, "entity"),
    ("girlBrain_sub_16E530",0x16E530, 0x24, "entity"),
    ("girlBrain_sub_16E6A4",0x16E6A4, 0x24, "entity"),
    ("girlBrain_sub_16F5F8",0x16F5F8, 0x24, "entity"),
    ("girlBrain_sub_175B4C",0x175B4C, 0x24, "entity"),
    ("girlBrain_sub_16D84C",0x16D84C, 0x20, "entity"),
    ("girlBrain_sub_16DF10",0x16DF10, 0x1C, "entity"),
    ("girlBrain_sub_16E538",0x16E538, 0x1C, "entity"),
    ("girlBrain_sub_16C130",0x16C130, 0x14, "entity"),
    # Boy/AI sub-functions (0x142000-0x164000 range, batch-scored)
    ("boyAI_sub_1512F4",0x1512F4, 0x12C, "entity"),
    ("boyAI_sub_153050",0x153050, 0x12C, "entity"),
    ("boyAI_sub_157718",0x157718, 0x12C, "entity"),
    ("boyAI_sub_149A18",0x149A18, 0x128, "entity"),
    ("boyAI_sub_14CDD0",0x14CDD0, 0x128, "entity"),
    ("boyAI_sub_150BF8",0x150BF8, 0x128, "entity"),
    ("boyAI_sub_1561BC",0x1561BC, 0x128, "entity"),
    ("boyAI_sub_15F5D8",0x15F5D8, 0x128, "entity"),
    ("boyAI_sub_153058",0x153058, 0x124, "entity"),
    ("boyAI_sub_157720",0x157720, 0x124, "entity"),
    ("boyAI_sub_15D6D4",0x15D6D4, 0x124, "entity"),
    ("boyAI_sub_149A20",0x149A20, 0x120, "entity"),
    ("boyAI_sub_14F8B4",0x14F8B4, 0x120, "entity"),
    ("boyAI_sub_158A18",0x158A18, 0x120, "entity"),
    ("boyAI_sub_15C878",0x15C878, 0x120, "entity"),
    ("boyAI_sub_150930",0x150930, 0x11C, "entity"),
    ("boyAI_sub_1518DC",0x1518DC, 0x11C, "entity"),
    ("boyAI_sub_1573C0",0x1573C0, 0x11C, "entity"),
    ("boyAI_sub_14F4A0",0x14F4A0, 0x114, "entity"),
    ("boyAI_sub_155B3C",0x155B3C, 0x114, "entity"),
    ("boyAI_sub_15DEB4",0x15DEB4, 0x114, "entity"),
    ("boyAI_sub_14A390",0x14A390, 0x110, "entity"),
    ("boyAI_sub_14B38C",0x14B38C, 0x110, "entity"),
    ("boyAI_sub_14D140",0x14D140, 0x110, "entity"),
    ("boyAI_sub_153568",0x153568, 0x110, "entity"),
    ("boyAI_sub_15896C",0x15896C, 0x110, "entity"),
    ("boyAI_sub_15ABF0",0x15ABF0, 0x110, "entity"),
    ("boyAI_sub_15C424",0x15C424, 0x110, "entity"),
    ("boyAI_sub_14F774",0x14F774, 0x10C, "entity"),
    ("boyAI_sub_151B8C",0x151B8C, 0x10C, "entity"),
    ("boyAI_sub_15B380",0x15B380, 0x10C, "entity"),
    ("boyAI_sub_14B36C",0x14B36C, 0x108, "entity"),
    ("boyAI_sub_150E78",0x150E78, 0x108, "entity"),
    ("boyAI_sub_156818",0x156818, 0x108, "entity"),
    ("boyAI_sub_15B96C",0x15B96C, 0x108, "entity"),
    ("boyAI_sub_15EA7C",0x15EA7C, 0x108, "entity"),
    ("boyAI_sub_149DD0",0x149DD0, 0x104, "entity"),
    ("boyAI_sub_158A20",0x158A20, 0x118, "entity"),
    ("boyAI_sub_15C880",0x15C880, 0x118, "entity"),
    ("boyAI_sub_143950",0x143950, 0x114, "entity"),
    ("boyAI_sub_14BE7C",0x14BE7C, 0x114, "entity"),
    ("boyAI_sub_1573C8",0x1573C8, 0x114, "entity"),
    ("boyAI_sub_15CF90",0x15CF90, 0x114, "entity"),
    ("boyAI_sub_142B28",0x142B28, 0x110, "entity"),
    ("boyAI_sub_15BA80",0x15BA80, 0x110, "entity"),
    ("boyAI_sub_14B9B8",0x14B9B8, 0x10C, "entity"),
    ("boyAI_sub_153480",0x153480, 0x10C, "entity"),
    ("boyAI_sub_15CF98",0x15CF98, 0x10C, "entity"),
    ("boyAI_sub_1447AC",0x1447AC, 0x108, "entity"),
    ("boyAI_sub_15BA88",0x15BA88, 0x108, "entity"),
    ("boyAI_sub_14B344",0x14B344, 0x104, "entity"),
    ("boyAI_sub_14B9C0",0x14B9C0, 0x104, "entity"),
    ("boyAI_sub_153488",0x153488, 0x104, "entity"),
    ("boyAI_sub_15F4D0",0x15F4D0, 0x104, "entity"),
    ("boyAI_sub_15D860",0x15D860, 0x100, "entity"),
    ("boyAI_sub_144B00",0x144B00, 0x100, "entity"),
    ("boyAI_sub_14C854",0x14C854, 0x100, "entity"),
    ("boyAI_sub_15DD74",0x15DD74, 0x100, "entity"),
    ("boyAI_sub_15F3D4",0x15F3D4, 0x100, "entity"),
    ("boyAI_sub_163FF4",0x163FF4, 0x100, "entity"),
    ("boyAI_sub_15F4D8",0x15F4D8, 0xFC, "entity"),
    ("boyAI_sub_145DD8",0x145DD8, 0xF8, "entity"),
    ("boyAI_sub_151F8C",0x151F8C, 0xF8, "entity"),
    ("boyAI_sub_157EA0",0x157EA0, 0xF8, "entity"),
    ("boyAI_sub_15D868",0x15D868, 0xF8, "entity"),
    ("boyAI_sub_15F2E0",0x15F2E0, 0xF8, "entity"),
    ("boyAI_sub_145CE8",0x145CE8, 0xF4, "entity"),
    ("boyAI_sub_14B254",0x14B254, 0xF4, "entity"),
    ("boyAI_sub_14D3F8",0x14D3F8, 0xF4, "entity"),
    ("boyAI_sub_1431C0",0x1431C0, 0xF0, "entity"),
    ("boyAI_sub_145DE0",0x145DE0, 0xF0, "entity"),
    ("boyAI_sub_14BB0C",0x14BB0C, 0xF0, "entity"),
    ("boyAI_sub_153B78",0x153B78, 0xF0, "entity"),
    ("boyAI_sub_145B38",0x145B38, 0xEC, "entity"),
    ("boyAI_sub_145CF0",0x145CF0, 0xEC, "entity"),
    ("boyAI_sub_14B9C8",0x14B9C8, 0xEC, "entity"),
    ("boyAI_sub_153940",0x153940, 0xEC, "entity"),
    ("boyAI_sub_158CF4",0x158CF4, 0xEC, "entity"),
    ("boyAI_sub_1623B8",0x1623B8, 0xEC, "entity"),
    ("boyAI_sub_1431C8",0x1431C8, 0xE8, "entity"),
    ("boyAI_sub_145A54",0x145A54, 0xE8, "entity"),
    ("boyAI_sub_157C28",0x157C28, 0xE8, "entity"),
    ("boyAI_sub_158040",0x158040, 0xE8, "entity"),
    ("boyAI_sub_15E4E0",0x15E4E0, 0xE8, "entity"),
    ("boyAI_sub_145B40",0x145B40, 0xE4, "entity"),
    ("boyAI_sub_14BBF8",0x14BBF8, 0xE4, "entity"),
    ("boyAI_sub_1596F8",0x1596F8, 0xE4, "entity"),
    ("boyAI_sub_150C40",0x150C40, 0xE0, "entity"),
    ("boyAI_sub_158048",0x158048, 0xE0, "entity"),
    ("boyAI_sub_15E4E8",0x15E4E8, 0xE0, "entity"),
    ("boyAI_sub_14BC00",0x14BC00, 0xDC, "entity"),
    ("boyAI_sub_14FC7C",0x14FC7C, 0xDC, "entity"),
    ("boyAI_sub_1507C0",0x1507C0, 0xDC, "entity"),
    ("boyAI_sub_145980",0x145980, 0xD8, "entity"),
    ("boyAI_sub_145F90",0x145F90, 0xD8, "entity"),
    ("boyAI_sub_14C63C",0x14C63C, 0xD8, "entity"),
    ("boyAI_sub_14FBF8",0x14FBF8, 0xD8, "entity"),
    ("boyAI_sub_143150",0x143150, 0xD4, "entity"),
    ("boyAI_sub_14BEA0",0x14BEA0, 0xD4, "entity"),
    ("boyAI_sub_14D280",0x14D280, 0xD4, "entity"),
    ("boyAI_sub_1506A8",0x1506A8, 0xD4, "entity"),
    ("boyAI_sub_156D6C",0x156D6C, 0xD4, "entity"),
    ("boyAI_sub_15A714",0x15A714, 0xD4, "entity"),
    ("boyAI_sub_158B68",0x158B68, 0xD0, "entity"),
    ("boyAI_sub_1448C4",0x1448C4, 0xCC, "entity"),
    ("boyAI_sub_15C53C",0x15C53C, 0xCC, "entity"),
    ("boyAI_sub_1568C8",0x1568C8, 0xC8, "entity"),
    ("boyAI_sub_1535E4",0x1535E4, 0xD8, "entity"),
    ("boyAI_sub_15BE88",0x15BE88, 0xD8, "entity"),
    ("boyAI_sub_146A84",0x146A84, 0xD4, "entity"),
    ("boyAI_sub_14D1F0",0x14D1F0, 0xD4, "entity"),
    ("boyAI_sub_14FFC8",0x14FFC8, 0xD4, "entity"),
    ("boyAI_sub_1507C8",0x1507C8, 0xD4, "entity"),
    ("boyAI_sub_1446E0",0x1446E0, 0xD0, "entity"),
    ("boyAI_sub_145F98",0x145F98, 0xD0, "entity"),
    ("boyAI_sub_15660C",0x15660C, 0xD0, "entity"),
    ("boyAI_sub_15BE90",0x15BE90, 0xD0, "entity"),
    ("boyAI_sub_145C20",0x145C20, 0xCC, "entity"),
    ("boyAI_sub_14FFD0",0x14FFD0, 0xCC, "entity"),
    ("boyAI_sub_15122C",0x15122C, 0xCC, "entity"),
    ("boyAI_sub_14BDD4",0x14BDD4, 0xC8, "entity"),
    ("boyAI_sub_150CC8",0x150CC8, 0xC8, "entity"),
    ("boyAI_sub_1518E8",0x1518E8, 0xC8, "entity"),
    ("boyAI_sub_157728",0x157728, 0xC8, "entity"),
    ("boyAI_sub_144858",0x144858, 0xC4, "entity"),
    ("boyAI_sub_146490",0x146490, 0xC4, "entity"),
    ("boyAI_sub_159A88",0x159A88, 0xC4, "entity"),
    ("boyAI_sub_159AB8",0x159AB8, 0xC0, "entity"),
    ("boyAI_sub_14F5B4",0x14F5B4, 0xBC, "entity"),
    ("boyAI_sub_156468",0x156468, 0xBC, "entity"),
    ("boyAI_sub_156AD4",0x156AD4, 0xBC, "entity"),
    ("boyAI_sub_146498",0x146498, 0xB8, "entity"),
    ("boyAI_sub_14C6E4",0x14C6E4, 0xB8, "entity"),
    ("boyAI_sub_158F24",0x158F24, 0xB8, "entity"),
    ("boyAI_sub_14B794",0x14B794, 0xB4, "entity"),
    ("boyAI_sub_150B08",0x150B08, 0xB4, "entity"),
    ("boyAI_sub_151B3C",0x151B3C, 0xB0, "entity"),
    ("boyAI_sub_145C58",0x145C58, 0xAC, "entity"),
    ("boyAI_sub_145C64",0x145C64, 0xAC, "entity"),
    ("boyAI_sub_14B9C4",0x14B9C4, 0xAC, "entity"),
    ("boyAI_sub_15D280",0x15D280, 0xAC, "entity"),
    ("boyAI_sub_155B34",0x155B34, 0xA8, "entity"),
    ("boyAI_sub_15D0B0",0x15D0B0, 0xA8, "entity"),
    ("boyAI_sub_14D258",0x14D258, 0xA4, "entity"),
    ("boyAI_sub_14B5C8",0x14B5C8, 0xA0, "entity"),
    ("boyAI_sub_150728",0x150728, 0xA0, "entity"),
    ("boyAI_sub_1574DC",0x1574DC, 0xA0, "entity"),
    ("boyAI_sub_14C104",0x14C104, 0x9C, "entity"),
    ("boyAI_sub_156008",0x156008, 0x9C, "entity"),
    ("boyAI_sub_156B38",0x156B38, 0x9C, "entity"),
    ("boyAI_sub_15B7D4",0x15B7D4, 0x9C, "entity"),
    ("boyAI_sub_14A818",0x14A818, 0x98, "entity"),
    ("boyAI_sub_1502F0",0x1502F0, 0x98, "entity"),
    ("boyAI_sub_15D068",0x15D068, 0x98, "entity"),
    ("boyAI_sub_143A5C",0x143A5C, 0x94, "entity"),
    ("boyAI_sub_15C5CC",0x15C5CC, 0x94, "entity"),
    ("boyAI_sub_157888",0x157888, 0x90, "entity"),
    ("boyAI_sub_1596B0",0x1596B0, 0x8C, "entity"),
    ("boyAI_sub_157890",0x157890, 0x88, "entity"),
    ("boyAI_sub_149E98",0x149E98, 0x88, "entity"),
    ("boyAI_sub_15D120",0x15D120, 0x88, "entity"),
    ("boyAI_sub_149F20",0x149F20, 0x84, "entity"),
    ("boyAI_sub_14BAC4",0x14BAC4, 0x84, "entity"),
    ("boyAI_sub_1575D8",0x1575D8, 0x84, "entity"),
    ("boyAI_sub_157B6C",0x157B6C, 0x84, "entity"),
    ("boyAI_sub_1597DC",0x1597DC, 0x84, "entity"),
    ("boyAI_sub_149F28",0x149F28, 0x80, "entity"),
    ("boyAI_sub_14BB6C",0x14BB6C, 0x7C, "entity"),
    ("boyAI_sub_159354",0x159354, 0x7C, "entity"),
    ("boyAI_sub_14BDD0",0x14BDD0, 0x78, "entity"),
    ("boyAI_sub_158F1C",0x158F1C, 0x78, "entity"),
    ("boyAI_sub_15C538",0x15C538, 0x78, "entity"),
    ("boyAI_sub_157738",0x157738, 0x74, "entity"),
    ("boyAI_sub_159A80",0x159A80, 0x74, "entity"),
    ("boyAI_sub_146A80",0x146A80, 0x70, "entity"),
    ("boyAI_sub_14BE78",0x14BE78, 0x70, "entity"),
    ("boyAI_sub_151814",0x151814, 0xCC, "entity"),
    ("boyAI_sub_1446E8",0x1446E8, 0xC8, "entity"),
    ("boyAI_sub_145ECC",0x145ECC, 0xC8, "entity"),
    ("boyAI_sub_14E314",0x14E314, 0xC8, "entity"),
    ("boyAI_sub_1504D8",0x1504D8, 0xC8, "entity"),
    ("boyAI_sub_15DE70",0x15DE70, 0xC8, "entity"),
    ("boyAI_sub_15ECE8",0x15ECE8, 0xC8, "entity"),
    ("boyAI_sub_144F94",0x144F94, 0xC4, "entity"),
    ("boyAI_sub_145030",0x145030, 0xC4, "entity"),
    ("boyAI_sub_1568B8",0x1568B8, 0xC0, "entity"),
    ("boyAI_sub_146088",0x146088, 0xBC, "entity"),
    ("boyAI_sub_14C080",0x14C080, 0xBC, "entity"),
    ("boyAI_sub_15096C",0x15096C, 0xBC, "entity"),
    ("boyAI_sub_15E880",0x15E880, 0xBC, "entity"),
    ("boyAI_sub_1574D0",0x1574D0, 0xB8, "entity"),
    ("boyAI_sub_15C9D4",0x15C9D4, 0xB8, "entity"),
    ("boyAI_sub_15E58C",0x15E58C, 0xB8, "entity"),
    ("boyAI_sub_15EDE0",0x15EDE0, 0xB8, "entity"),
    ("boyAI_sub_15F064",0x15F064, 0xB8, "entity"),
    ("boyAI_sub_146080",0x146080, 0xB4, "entity"),
    ("boyAI_sub_145C28",0x145C28, 0xC4, "entity"),
    ("boyAI_sub_153C64",0x153C64, 0xC4, "entity"),
    ("boyAI_sub_15C994",0x15C994, 0xC4, "entity"),
    ("boyAI_sub_142F1C",0x142F1C, 0xC0, "entity"),
    ("boyAI_sub_146B54",0x146B54, 0xC0, "entity"),
    ("boyAI_sub_14F408",0x14F408, 0xC0, "entity"),
    ("boyAI_sub_14FBC0",0x14FBC0, 0xC0, "entity"),
    ("boyAI_sub_1504E0",0x1504E0, 0xC0, "entity"),
    ("boyAI_sub_14E844",0x14E844, 0xBC, "entity"),
    ("boyAI_sub_14E8FC",0x14E8FC, 0xBC, "entity"),
    ("boyAI_sub_150708",0x150708, 0xBC, "entity"),
    ("boyAI_sub_158124",0x158124, 0xBC, "entity"),
    ("boyAI_sub_1451C0",0x1451C0, 0xB8, "entity"),
    ("boyAI_sub_146F14",0x146F14, 0xB8, "entity"),
    ("boyAI_sub_14FB0C",0x14FB0C, 0xB8, "entity"),
    ("boyAI_sub_14FBC8",0x14FBC8, 0xB8, "entity"),
    ("boyAI_sub_14FD54",0x14FD54, 0xB8, "entity"),
    ("boyAI_sub_14FE08",0x14FE08, 0xB8, "entity"),
    ("boyAI_sub_15141C",0x15141C, 0xB8, "entity"),
    ("boyAI_sub_157020",0x157020, 0xB8, "entity"),
    ("boyAI_sub_157108",0x157108, 0xB8, "entity"),
    ("boyAI_sub_15DE20",0x15DE20, 0xB4, "entity"),
    ("boyAI_sub_144D68",0x144D68, 0xB0, "entity"),
    ("boyAI_sub_14B48C",0x14B48C, 0xB0, "entity"),
    ("boyAI_sub_1506A0",0x1506A0, 0xB0, "entity"),
    ("boyAI_sub_15781C",0x15781C, 0xB0, "entity"),
    ("boyAI_sub_15D978",0x15D978, 0xB0, "entity"),
    ("boyAI_sub_14B69C",0x14B69C, 0xAC, "entity"),
    ("boyAI_sub_145C60",0x145C60, 0xA8, "entity"),
    ("boyAI_sub_14B9CC",0x14B9CC, 0xA8, "entity"),
    ("boyAI_sub_157824",0x157824, 0xA4, "entity"),
    ("boyAI_sub_14B790",0x14B790, 0xA0, "entity"),
    ("boyAI_sub_150BCC",0x150BCC, 0x9C, "entity"),
    ("boyAI_sub_159B40",0x159B40, 0x9C, "entity"),
    ("boyAI_sub_14C690",0x14C690, 0x94, "entity"),
    ("boyAI_sub_156A64",0x156A64, 0x94, "entity"),
    ("boyAI_sub_1575C4",0x1575C4, 0x94, "entity"),
    ("boyAI_sub_14B0C0",0x14B0C0, 0x90, "entity"),
    ("boyAI_sub_14B5C0",0x14B5C0, 0x90, "entity"),
    # eBrain entry AI functions (src/omori/, verified Ghidra symbols at 0x0019xxxx)
    # eBrainProcess (0x190B30) and eBrainGetTargetGeneratorFromLabel (0x190D70)
    # are .word-only .s files.
    ("eBrainGetTarget",0x190F30, 0x9C0, "entity"),
    ("eBrainInit",0x1918A8, 0x50, "entity"),
    ("eBrainStatusSet",0x1918F0, 0xB0, "entity"),
    ("eBrainSendMes",0x1919A0, 0xD0, "entity"),
    ("eBrainGetTargetGeneratorFromLabelStage",0x191B70, 0x1B0, "entity"),
    # Speculative-named eBrain* helpers (no verified Ghidra symbol, but byte-exact):
    ("eBrainGetStatus",    0x191D20, 0x50, "entity"),
    ("eBrainSetFlag",      0x191D6C, 0x84, "entity"),
    ("eBrainMovePos",      0x191DF0, 0x100, "entity"),
    ("eBrainMotionSe",     0x191EF0, 0x78, "entity"),
    ("eBrainPursuit",      0x191F68, 0x1E8, "entity"),
    ("eBrainAvoid",        0x192150, 0x188, "entity"),
    ("eBrainReturnInit",   0x1922D8, 0xA8, "entity"),
    ("eBrainTargetGenerator",0x192380, 0x1F8, "entity"),
    # === NEW: Entity callbacks from descriptor table (runtime-hot) ===
    # GIRL callbacks (always active: player companion)
    ("girl_hA",          0x1D1A98, 0x40, "entity"),
    ("girl_hB",          0x1D17F8, 0x1C4, "entity"),
    ("girl_hC",          0x1D1668, 0x18C, "entity"),
    ("girl_hD",          0x1D1AD0, 0x08, "entity"),
    # ENEMY1 remaining callbacks
    ("enemy1_hA",        0x1CE690, 0x5C, "entity"),
    ("enemy1_hD",        0x1CE760, 0x08, "entity"),
    # QUEEN callbacks (boss encounters)
    ("queen_hC",         0x19A7E8, 0x108, "entity"),
    ("queen_hB",         0x19A8F0, 0xB0, "entity"),
    ("queen_hA",         0x19A9A0, 0x80, "entity"),
    # BOSS_CTRL callbacks
    ("bossctrl_hA",      0x198000, 0x110, "entity"),
    ("bossctrl_hB",      0x197FC8, 0x40, "entity"),
    # BIRD remaining callbacks
    ("bird_hA",          0x197080, 0x60, "entity"),
    ("bird_hB",          0x197078, 0x10, "entity"),
    # GENERATOR callbacks
    ("generator_hA",     0x193600, 0x130, "entity"),
    ("generator_hB",     0x1930B0, 0x550, "entity"),
    ("generator_hC",     0x193730, 0xE4, "entity"),
    # === Batch 3: remaining entity callbacks ===
    ("cage_hA",      0x1C2DF8, 0x00C0, "entity"),
    ("cage_hB",      0x1C28D0, 0x0500, "entity"),
    ("cage_hC",      0x1C2338, 0x0424, "entity"),
    ("cagefix_hA",   0x1C2FA0, 0x0044, "entity"),
    ("cagefix_hB",   0x1C2F20, 0x007C, "entity"),
    ("cagefix_hC",   0x1C2FE8, 0x08, "entity"),
    ("candle_hA",    0x1C3130, 0x0044, "entity"),
    ("candle_hB",    0x1C3178, 0x00F4, "entity"),
    ("candle_hC",    0x1C2FF0, 0x100, "entity"),
    ("chandelier_hA",0x1C3470, 0x0044, "entity"),
    ("chandelier_hB",0x1C33D8, 0x0094, "entity"),
    ("chandelier_hC",0x1C34B8, 0x08, "entity"),
    ("flag_hA",      0x1D01E8, 0x100, "entity"),
    ("flag_hB",      0x1D00F8, 0x00F0, "entity"),
    ("flag_hC",      0x1CFB58, 0x500, "entity"),
    ("seffect_hA",   0x1EF988, 0x08, "entity"),
    ("seffect_hB",   0x1EF980, 0x08, "entity"),
    ("seffect_hC",   0x1EF8E8, 0x0094, "entity"),
    ("lightbit_hA",  0x1F0550, 0x00B4, "entity"),
    ("lightbit_hB",  0x1F0540, 0x00C4, "entity"),
    ("lightbit_hC",  0x1F0568, 0x009C, "entity"),
    ("tree_hA",      0x1F1508, 0x0198, "entity"),
    ("tree_hB",      0x1F1330, 0x01D8, "entity"),
    ("tree_hC",      0x1F17B0, 0x0084, "entity"),
    ("torch_hA",     0x1F2140, 0x08, "entity"),
    ("torch_hB",     0x1F1CF0, 0x0354, "entity"),
    ("torch_hC",     0x1F2048, 0x00F4, "entity"),
    ("rotobj_hA",    0x1EA030, 0x00D4, "entity"),
    ("rotobj_hB",    0x1E9950, 0x0030, "entity"),
    ("rotobj_hC",    0x1E9F08, 0x100, "entity"),
    ("chain_hA",     0x18F640, 0x0154, "entity"),
    ("chain_hB",     0x18ECC8, 0x100, "entity"),
    ("chain_hC",     0x18E5B0, 0x05BC, "entity"),
    ("darkvolume_hA",0x1CBD78, 0x08, "entity"),
    ("darkvolume_hB",0x1CBD70, 0x08, "entity"),
    ("darkvolume_hC",0x1CBD68, 0x08, "entity"),
    ("pobj_hA",      0x1AEA58, 0x0010, "entity"),
    ("pobj_hB",      0x1AEA50, 0x0018, "entity"),
    ("pobj_hC",      0x1AEA60, 0x08, "entity"),
    ("sv_hA",        0x10EC60, 0x0034, "entity"),
    ("mobj_hA",      0x10ECB0, 0x0030, "entity"),
    ("sobj_hA",      0x10ECC0, 0x0020, "entity"),
    # === Batch 4: AP1, ATTACKCH, QUEEN/BALL/BARRIER, type22-25, type39, type66 ===
    ("type39_hC",    0x10B2D0, 0x049C, "entity"),
    ("type39_hA",    0x10C5C0, 0x0244, "entity"),
    ("type39_hB",    0x10D070, 0x08, "entity"),
    ("type66_vtable",0x191C80, 0x0084, "entity"),
    ("type66_hC",    0x191D08, 0x0018, "entity"),
    ("queenball_hA", 0x19B660, 0x0194, "entity"),
    ("queenball_hB", 0x19AE98, 0x07C8, "entity"),
    ("queenball_hC", 0x19BC58, 0x0088, "entity"),
    ("queenbarrier_hA",0x19AE50, 0x0048, "entity"),
    ("queenbarrier_hB",0x19AA20, 0x0430, "entity"),
    ("queenbarrier_hC",0x19BAA8, 0x0098, "entity"),
    ("ap1_hA",       0x1BA530, 0x008C, "entity"),
    ("ap1_hB",       0x1BA330, 0x01FC, "entity"),
    ("ap1_hC",       0x1B8720, 0x05BC, "entity"),
    ("ap1_hD",       0x1BB3E0, 0x02CC, "entity"),
    ("attackch63_hB",0x1BBB20, 0x02B8, "entity"),
    ("attackch62_hB",0x1BBDD8, 0x006C, "entity"),
    ("attackch63_hA",0x1BBE48, 0x08, "entity"),
    ("attackch62_hA",0x1BBEC8, 0x00B0, "entity"),
    ("type23_hB",    0x1BC130, 0x0304, "entity"),
    ("type22_hB",    0x1BC1A8, 0x028C, "entity"),
    ("type22_hA",    0x1BC438, 0x03B8, "entity"),
    ("type24_hB",    0x1BC530, 0x02C0, "entity"),
    ("type24_hA",    0x1BC7F0, 0x01AC, "entity"),
    # === Batch 5: remaining callbacks + vtables ===
    ("boy_hB",              0x1C1DD8, 0x02C0, "entity"),
    ("type6_hB",            0x1CE6F0, 0x006C, "entity"),
    ("type6_hC",            0x1CEB18, 0x004C, "entity"),
    ("type5_hA",            0x1CE5F8, 0x0094, "entity"),
    ("type51_hA",           0x1C9330, 0x08, "entity"),
    ("type51_hB",           0x1C9328, 0x08, "entity"),
    ("type51_hC",           0x1C92D8, 0x004C, "entity"),
    ("type67_hA",           0x1D03A0, 0x0024, "entity"),
    ("type67_hC",           0x1D03C8, 0x0014, "entity"),
    ("type55_hA",           0x1D1B30, 0x0144, "entity"),
    ("type55_hB",           0x1D1CF8, 0x08, "entity"),
    ("type55_hC",           0x1D1C78, 0x0080, "entity"),
    ("type64_hC",           0x198218, 0x0110, "entity"),
    ("type64_vtable",       0x1978B0, 0x0184, "entity"),
    ("type46_vtable",       0x199A60, 0x01CC, "entity"),
    ("type53_vtable",       0x19B998, 0x00C8, "entity"),
    ("type52_vtable",       0x19BB60, 0x00F4, "entity"),
    ("type33_vtable",       0x192EB8, 0x01F4, "entity"),
    ("type32_vtable",       0x1956E8, 0x0224, "entity"),
    ("type62_vtable",       0x1BB988, 0x0194, "entity"),
    ("type62_hB",           0x1BBEA0, 0x0024, "entity"),
    ("woodbox22_hC",        0x1C09C8, 0x0110, "entity"),
    ("woodbox24_hC",        0x1C0C40, 0x0110, "entity"),
    ("type12_hA",           0x1E8F30, 0x08, "entity"),
    ("type12_hB",           0x1E8F28, 0x08, "entity"),
    ("type12_hC",           0x1E8EB8, 0x006C, "entity"),
    ("type20_hA",           0x1E9630, 0x01C4, "entity"),
    ("type20_hB",           0x1E9810, 0x0078, "entity"),
    ("type20_hC",           0x1E8F38, 0x0498, "entity"),
    ("type42_hA",           0x1E98C8, 0x0044, "entity"),
    ("type42_hB",           0x1E9888, 0x0040, "entity"),
    ("type42_hC",           0x1E9910, 0x08, "entity"),
    ("type15_hB",           0x1EA5E8, 0x03E0, "entity"),
    ("type15_hC",           0x1EA278, 0x0168, "entity"),
    ("type41_hA",           0x1E6960, 0x0074, "entity"),
    ("type41_hB",           0x1E6788, 0x01D8, "entity"),
    ("type41_hC",           0x1E6968, 0x006C, "entity"),
    ("type49_hA",           0x1E0860, 0x0058, "entity"),
    ("type49_hB",           0x1E08B8, 0x00C0, "entity"),
    ("type60_hB",           0x23D518, 0x0090, "entity"),
    ("type36_hB",           0x1F44C8, 0x0018, "entity"),
    ("type36_hC",           0x1F43D0, 0x00F4, "entity"),
    ("type38_hA",           0x1F69B0, 0x00E0, "entity"),
    ("type38_hB",           0x1F66F0, 0x02C0, "entity"),
    ("type38_hC",           0x1F62E8, 0x03B4, "entity"),
    ("shared_vtable_202a60",0x202A60, 0x06D0, "entity"),
    ("shared_vtable_23d660",0x23D660, 0x0084, "entity"),
]


# --- ELF utilities ---
def extract_elf_func(target_va: int, size: int = 0x200) -> bytes:
    """Extract raw bytes of a function from the ELF at the given VA."""
    with open(ELF_PATH, "rb") as f:
        elf_data = f.read()

    e_phoff = struct.unpack_from("<I", elf_data, 0x1C)[0]
    e_phentsize = struct.unpack_from("<H", elf_data, 0x2A)[0]
    e_phnum = struct.unpack_from("<H", elf_data, 0x2C)[0]

    for i in range(e_phnum):
        ph = elf_data[e_phoff + i * e_phentsize: e_phoff + (i + 1) * e_phentsize]
        p_type = struct.unpack_from("<I", ph, 0)[0]
        if p_type != 1:
            continue
        p_offset = struct.unpack_from("<I", ph, 4)[0]
        p_vaddr = struct.unpack_from("<I", ph, 8)[0]
        p_filesz = struct.unpack_from("<I", ph, 16)[0]
        if p_vaddr <= target_va < p_vaddr + p_filesz:
            file_off = p_offset + (target_va - p_vaddr)
            avail = min(size, p_filesz - (target_va - p_vaddr))
            return elf_data[file_off:file_off + avail]
    raise ValueError(f"VA 0x{target_va:08X} not found in any LOAD segment")


def disassemble_mips64(data: bytes, start_va: int) -> list[dict]:
    """Disassemble MIPS64 little-endian binary data."""
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_MIPS,
                     capstone.CS_MODE_MIPS64 + capstone.CS_MODE_LITTLE_ENDIAN)
    md.skipdata = True
    result = []
    for insn in md.disasm(data, start_va):
        result.append({
            "va": insn.address,
            "bytes": insn.bytes.hex(),
            "mnemonic": insn.mnemonic,
            "op_str": insn.op_str,
            "size": insn.size,
        })
    return result


# --- Auto-size detection ---
def auto_detect_sizes() -> list:
    """Return function sizes from score_all.py data."""
    return [(name, va, size) for name, va, size, _ in TARGET_FUNCTIONS]


# --- Register name conversion ---
ABI_TO_NUM = {
    "$zero": "$0", "$at": "$1", "$v0": "$2", "$v1": "$3",
    "$a0": "$4", "$a1": "$5", "$a2": "$6", "$a3": "$7",
    "$t0": "$8", "$t1": "$9", "$t2": "$10", "$t3": "$11",
    "$t4": "$12", "$t5": "$13", "$t6": "$14", "$t7": "$15",
    "$s0": "$16", "$s1": "$17", "$s2": "$18", "$s3": "$19",
    "$s4": "$20", "$s5": "$21", "$s6": "$22", "$s7": "$23",
    "$t8": "$24", "$t9": "$25", "$k0": "$26", "$k1": "$27",
    "$gp": "$28", "$sp": "$29", "$fp": "$30", "$ra": "$31",
}
NUM_TO_ABI = {v: k for k, v in ABI_TO_NUM.items()}

# CP1 register names (float)
FREG_MAP = {f"$f{i}": f"${i}" for i in range(32)}


def _convert_regs(ops: str) -> str:
    """Convert ABI register names to numeric in operand string.
    
    Float registers ($f0-$f31) are NOT converted — they keep $fN format
    since the assembler distinguishes them from general registers by the $f prefix.
    """
    for abi, num in ABI_TO_NUM.items():
        ops = ops.replace(abi, num)
    return ops


def _format_ops(ops: str, mnem: str = "") -> str:
    """Format operand string for the EE assembler.
    
    Rules:
    - Numeric registers only (no ABI names)
    - No spaces after commas
    - Zero offset as explicit 0 (not implicit)
    - Use decimal for small offsets
    """
    # Convert ABI regs to numeric
    ops = _convert_regs(ops)

    # Remove space after each comma
    ops = re.sub(r',\s+', ',', ops)

    # Remove space before parentheses
    ops = re.sub(r'\s+(?=[\(\]])', '', ops)

    # Remove leading $ from bare register numbers that somehow slipped through
    # (already handled by _convert_regs)

    # Convert implicit zero offset like "($29)" to "0($29)"
    # Only when NOT preceded by hex digit (avoids corrupting 0x15c($16)→0x15c0($16))
    ops = re.sub(r'(?<![0-9a-fA-F])\((\$\d+)\)', r'0(\1)', ops)

    # Convert hex offsets to decimal for small values (< 256)
    # This matches the target binary's immediate encoding
    def _dec_or_hex(ops: str) -> str:
        # Don't convert if it's a label (has $L prefix)
        if ops.strip().startswith("$L"):
            return ops
        return ops

    return ops


def _resolve_label(target_va: int, branch_labels: dict) -> str:
    """Resolve a branch target VA to a GAS label reference.

    The generated sources now use unique symbolic labels per target address,
    so a direct label name is sufficient for both forward and backward refs.
    """
    return branch_labels.get(target_va)


def insn_to_asm(insn: dict, branch_labels: dict,
                va_to_idx: dict[int, int], current_idx: int) -> str:
    """Convert one disassembled instruction to assembly text.
    
    branch_labels maps target VA -> label number (0-9).
    va_to_idx maps VA -> instruction index.
    current_idx is the index of this instruction.
    EE assembler requires numeric register names in operands.
    """
    mnem = insn["mnemonic"]
    ops = insn["op_str"]
    va = insn["va"]

    # Helper to parse target address from operand string (hex or decimal)
    def _parse_target(s: str) -> int | None:
        s = s.strip()
        try:
            return int(s, 0)  # auto-detect base (0x prefix for hex)
        except (ValueError, TypeError):
            return None

    # Handle branch/call targets — replace absolute addresses with labels
    if mnem == "jal":
        target = _parse_target(ops)
        if target is not None:
            label_ref = _resolve_label(target, branch_labels)
            if label_ref:
                return f"\t{mnem}\t{label_ref}"
            else:
                return f"\t{mnem}\t0x{target:08x}"
    
    # Handle jal with $reg (jalr) — convert reg name
    if mnem == "jalr":
        ops = _convert_regs(ops)
        return f"\t{mnem}\t{ops}"

    # Helper: emit raw bytes for branch when target is NOT in va_to_idx
    def _emit_raw_branch(va_to_idx, target) -> str | None:
        """Return .word for this insn if target is outside instruction range."""
        if target is not None and target not in va_to_idx:
            word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
            return f"\t.word\t0x{word:08x}"
        return None

    # Handle conditional and unconditional branches (including pseudo-ops)
    if mnem in ("b", "beq", "bne", "beql", "bnel", "bltz", "bgez", "bgtz", "blez",
                "bltzl", "bgezl", "bgtzl", "blezl",
                "beqz", "bnez"):
        parts = ops.rsplit(", ", 1)
        if len(parts) == 2:
            left_ops = _format_ops(parts[0], mnem)
            target = _parse_target(parts[1])
            raw = _emit_raw_branch(va_to_idx, target)
            if raw:
                return raw
            if target is not None:
                label_ref = _resolve_label(target, branch_labels)
                if label_ref:
                    return f"\t{mnem}\t{left_ops},{label_ref}"
            return f"\t{mnem}\t{left_ops},{_convert_regs(parts[1])}"
        # "b target_addr" (unconditional branch with single operand)
        target = _parse_target(ops)
        raw = _emit_raw_branch(va_to_idx, target)
        if raw:
            return raw
        if target is not None:
            label_ref = _resolve_label(target, branch_labels)
            if label_ref:
                return f"\t{mnem}\t{label_ref}"
        return f"\t{mnem}\t{_format_ops(ops, mnem)}"

    # Handle bc1f/bc1t branches
    if mnem in ("bc1f", "bc1t", "bc1fl", "bc1tl"):
        target = _parse_target(ops)
        raw = _emit_raw_branch(va_to_idx, target)
        if raw:
            return raw
        if target is not None:
            label_ref = _resolve_label(target, branch_labels)
            if label_ref:
                return f"\t{mnem}\t{label_ref}"
        return f"\t{mnem}\t{_format_ops(ops, mnem)}"

    # Handle j (unconditional jump)
    if mnem == "j":
        target = _parse_target(ops)
        if target is not None:
            label_ref = _resolve_label(target, branch_labels)
            if label_ref:
                return f"\t{mnem}\t{label_ref}"
        return f"\t{mnem}\t{_format_ops(ops, mnem)}"

    # Handle lwc1/swc1 (float load/store)
    if mnem in ("lwc1", "swc1", "ldc1", "sdc1"):
        ops = _format_ops(ops, mnem)
        return f"\t{mnem}\t{ops}"

    # Handle mtc1/mfc1 (float/int moves)
    if mnem in ("mtc1", "mfc1", "ctc1", "cfc1"):
        ops = _format_ops(ops, mnem)
        return f"\t{mnem}\t{ops}"

    # R5900-specific: mult/multu with $acN prefix
    # Capstone outputs "mult $ac3, $v1, $v0" → need "mult $3,$3,$2"
    if mnem in ("mult", "multu") and ops.count(", ") == 2 and ops.startswith("$ac"):
        # Parse: $acN, $rs, $rt
        parts = ops.split(", ")
        ac_num = parts[0].replace("$ac", "")
        rs_num = _convert_regs(parts[1])
        rt_num = _convert_regs(parts[2])
        return f"\t{mnem}\t${ac_num},{rs_num},{rt_num}"

    # R5900-specific: madd/msub with $acN prefix
    # Capstone outputs "madd $ac2, $v0, $t2" → need "madd $2,$2,$10" (3 operands for correct encoding)
    if mnem in ("madd", "msub") and ops.count(", ") == 2 and ops.startswith("$ac"):
        parts = ops.split(", ")
        ac_num = parts[0].replace("$ac", "")
        rs_num = _convert_regs(parts[1])
        rt_num = _convert_regs(parts[2])
        return f"\t{mnem}\t${ac_num},{rs_num},{rt_num}"

    # R5900-specific: div with 3 operands
    # Capstone outputs "div $zero, $v0, $a2" → need "div $0,$2,$6"
    if mnem == "div" and ops.count(", ") == 2:
        parts = ops.split(", ")
        rd_num = _convert_regs(parts[0])
        rs_num = _convert_regs(parts[1])
        rt_num = _convert_regs(parts[2])
        return f"\t{mnem}\t{rd_num},{rs_num},{rt_num}"

    # R5900-specific: c.olt.s and other COP1 compares
    # These are NOT supported by the EE assembler — emit raw bytes
    if mnem in ("c.olt.s", "c.ult.s", "c.ole.s", "c.ule.s",
                "c.eq.s", "c.ueq.s", "c.lt.s", "c.le.s",
                "c.f.s", "c.un.s", "c.sf.s", "c.seq.s",
                "c.ngle.s", "c.ngl.s", "c.nge.s", "c.ngt.s"):
        word = int(insn["bytes"], 16)
        word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
        return f"\t.word\t0x{word:08x}"

    # R5900-specific: COP2 (VU0) instructions — dmfc2, qmfc2, etc.
    # EE assembler does not support COP2 operands — emit raw bytes
    if mnem.startswith("dmfc2") or mnem.startswith("qmfc2") or mnem.startswith("qmtc2"):
        word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
        return f"\t.word\t0x{word:08x}"

    # R5900-specific: any COP2 instruction not caught above
    # Capstone may emit various VU0 mnemonics the EE assembler can't handle
    if mnem in ("vmula", "vmul", "vadd", "vadda", "vmadd", "vmadda",
                "vsub", "vsuba", "vmsub", "vmsuba", "vmax", "vmaxi",
                "vmin", "vmini", "vmulai", "vmuli", "vaddi", "vmaddi",
                "vsubi", "vmsubi", "vdiv", "vsqrt", "vrsqrt", "vleng",
                "vrseng", "vdadda", "vdsuba", "vclipw", "vswait",
                "viadd", "viaddi", "viaddk", "visub", "visubi",
                "vilwr", "viswr", "vitof0", "vitof4", "vitof12", "vitof15",
                "vf0to4", "vf0to12", "vf0to15", "vftoi0", "vftoi4",
                "vftoi12", "vftoi15", "vnop", "vcallms", "vcallmsr"):
        word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
        return f"\t.word\t0x{word:08x}"

    # R5900-specific: bbit032, bbit031, bbit030 and other bit test ops
    if mnem.startswith("bbit0"):
        word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
        return f"\t.word\t0x{word:08x}"

    # R5900-specific: packed byte/halfword SIMD operations
    # addu.qb, addu.ph, aver_u.h, subu.qb, etc. — EE assembler rejects these
    if mnem in ("addu.qb", "addu.ph", "addu_pw", "subu.qb", "subu.ph",
                "aver_u.h", "aver_u.b", "aver_s.h", "aver_s.b",
                "max_s.h", "max_u.h", "min_s.h", "min_u.h",
                "max_s.b", "max_u.b", "min_s.b", "min_u.b",
                "sll_qb", "srl_qb", "sra_qb", "sllvi_qb",
                "srlvi_qb", "sravi_qb", "sll_ph", "srl_ph", "sra_ph",
                "sllvi_ph", "srlvi_ph", "sravi_ph",
                "mult.qb", "multu.qb", "mult_ph", "multu_ph",
                "madd.qb", "maddu.qb", "madd_ph", "maddu_ph",
                "msub.qb", "msubu.qb", "msub_ph", "msubu_ph",
                "dmult.qb", "dmultu.qb",
                "interleave_qh", "deinterleave_qh",
                "lqiqb", "sqiqb", "lqiph", "sqiph",
                "pextlw", "pextuw", "pextlb", "pextub",
                "pextlh", "pextuh",
                "pcpyld", "pcpyud", "pcpyh",
                "ppac5", "ppac4", "ppac3", "ppac2", "ppac1",
                "ppach", "ppacb", "ppacw",
                "pceqw", "pceqh", "pceqb",
                "pcgtw", "pcgth", "pcgtb",
                "pceqzw", "pceqzh", "pceqzb",
                "pcgtzw", "pcgtzh", "pcgtzb",
                "paddw", "paddh", "paddb",
                "paddw_r", "paddh_r", "paddb_r",
                "paddus_w", "paddus_h", "paddus_b",
                "padduw_h", "padduw_b",
                "paddsb", "paddsh",
                "psubw", "psubh", "psubb",
                "psubus_w", "psubus_h", "psubus_b",
                "psubuw_h", "psubuw_b",
                "psubsb", "psubsh",
                "pmulw", "pmulh", "pmulh_w",
                "pmuluw", "pmulqh", "pmulqh_w",
                "pmaddw", "pmadduw",
                "pmsubw", "pmsubuw",
                "pmfhi", "pmflo", "pmthi", "pmtlo",
                "pinteh", "pinth",
                "pmfhl_lw", "pmthl_lw",
                "psllw", "psrlw", "psraw",
                "psllh", "psrlh", "psrah",
                "psravw", "psllvw", "psrlvw",
                "vcallms", "vcallmsr", "vnop", "vwaitq"):
        word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
        return f"\t.word\t0x{word:08x}"

    # Capstone skipdata emits .byte for unrecognised instructions — emit as .word
    if mnem == ".byte":
        word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
        return f"\t.word\t0x{word:08x}"

    return f"\t{mnem}\t{_format_ops(ops, mnem)}"


def generate_asm_source(func_name: str, insns: list[dict], va: int) -> str:
    """Generate GCC-style .s file content from disassembled instructions."""
    # Build VA -> instruction index map FIRST
    va_to_idx = {insn["va"]: idx for idx, insn in enumerate(insns)}

    # Build branch target -> label mapping (only for targets within instruction range)
    branch_targets = {}
    for insn in insns:
        mnem = insn["mnemonic"]
        ops = insn["op_str"]
        va_curr = insn["va"]

        # Helper to parse target address
        def _pt(s: str) -> int | None:
            try:
                return int(s.strip(), 0)
            except (ValueError, TypeError):
                return None

        # Extract target VA from branch instructions
        # NOTE: jal and j use absolute addressing — no labels needed.
        # Only PC-relative branches (beq, bne, etc. and bc1f/bc1t) need labels.
        targets = []
        if mnem in ("b", "beq", "bne", "beql", "bnel", "bltz", "bgez", "bgtz", "blez",
                    "bltzl", "bgezl", "bgtzl", "blezl",
                    "beqz", "bnez"):
            parts = ops.rsplit(", ", 1)
            if len(parts) == 2:
                t = _pt(parts[1])
                if t is not None:
                    targets.append(t)
            elif len(parts) == 1 and mnem == "b":
                # Unconditional branch: "b target_addr" (single operand)
                t = _pt(parts[0])
                if t is not None:
                    targets.append(t)
        elif mnem in ("bc1f", "bc1t", "bc1fl", "bc1tl"):
            t = _pt(ops)
            if t is not None:
                targets.append(t)

        for t in targets:
            # ONLY create labels for targets WITHIN the instruction range
            if t not in branch_targets and va_curr != t and t in va_to_idx:
                branch_targets[t] = None

    # Assign unique symbolic labels to branch targets.
    # Numeric local labels collide once a function has more than a few loops.
    for t_va in sorted(branch_targets.keys()):
        branch_targets[t_va] = f"loc_{t_va:08x}"

    # Build the .s file
    lines = []
    lines.append(f"\t.text")
    lines.append(f"\t.p2align 3")
    lines.append(f"\t.globl\t{func_name}")
    lines.append(f"\t.ent\t{func_name}")
    lines.append(f"{func_name}:")
    lines.append(f"\t.frame\t$sp,0,$31")
    lines.append(f"\t.mask\t0x00000000,0")
    lines.append(f"\t.fmask\t0x00000000,0")
    lines.append(f"\t.set\tnoreorder")
    lines.append(f"\t.set\tnomacro")
    lines.append(f"\t.set\tnoat")

    for insn_idx, insn in enumerate(insns):
        va_curr = insn["va"]

        # Emit label if this VA is a branch target
        if va_curr in branch_targets:
            lines.append(f"{branch_targets[va_curr]}:")

        asm_line = insn_to_asm(insn, branch_targets, va_to_idx, insn_idx)
        lines.append(asm_line)

    lines.append(f"\t.set\tmacro")
    lines.append(f"\t.set\treorder")
    lines.append(f"\t.end\t{func_name}")

    return "\n".join(lines) + "\n"


# --- Assembly and verification ---
def extract_text_from_o(o_data: bytes) -> bytes | None:
    """Extract .text section bytes from an ELF32 .o file."""
    # ELF32 header
    e_shoff = struct.unpack_from("<I", o_data, 0x20)[0]
    e_shentsize = struct.unpack_from("<H", o_data, 0x2E)[0]
    e_shnum = struct.unpack_from("<H", o_data, 0x30)[0]

    if e_shoff <= 0 or e_shentsize == 0 or e_shnum == 0:
        return None

    for i in range(e_shnum):
        sh_off = e_shoff + i * e_shentsize
        sh_type = struct.unpack_from("<I", o_data, sh_off + 4)[0]
        sh_offset = struct.unpack_from("<I", o_data, sh_off + 0x10)[0]
        sh_size = struct.unpack_from("<I", o_data, sh_off + 0x14)[0]
        if sh_type == 1 and sh_size > 0:
            return o_data[sh_offset:sh_offset + sh_size]

    return None


def assemble_and_verify(asm_source: str, func_name: str, target_va: int,
                        target_size: int) -> dict:
    """Assemble .s source and verify byte-exact match against ELF target.

    Returns dict with: success, assembled_bytes, target_bytes, score, errors
    """
    # Get target bytes
    try:
        target_bytes = extract_elf_func(target_va, target_size)
    except (ValueError, FileNotFoundError) as e:
        return {"success": False, "error": f"Cannot read target ELF: {e}"}

    # Write temp .s file
    fd, sfile = tempfile.mkstemp(suffix=".s")
    os.write(fd, asm_source.encode())
    os.close(fd)

    ofile = sfile.rsplit(".", 1)[0] + ".o"

    try:
        result = subprocess.run(
            [EE_GCC, "-c", sfile, "-o", ofile] + CC_FLAGS,
            capture_output=True, text=True, timeout=30,
        )
        if result.returncode != 0:
            return {"success": False, "error": f"Assembly failed: {result.stderr}"}

        if not os.path.exists(ofile):
            return {"success": False, "error": "No .o file generated"}

        with open(ofile, "rb") as f:
            o_data = f.read()

        assembled_bytes = extract_text_from_o(o_data)
        if assembled_bytes is None:
            return {"success": False, "error": "Could not extract .text from .o"}

        # Compare
        if len(assembled_bytes) != len(target_bytes):
            return {
                "success": False,
                "error": f"Size mismatch: assembled={len(assembled_bytes)}, target={len(target_bytes)}",
                "assembled_bytes": assembled_bytes,
                "target_bytes": target_bytes,
            }

        mismatch_positions = []
        for i, (a, b) in enumerate(zip(assembled_bytes, target_bytes)):
            if a != b:
                mismatch_positions.append((i, a, b))

        if not mismatch_positions:
            return {
                "success": True,
                "assembled_bytes": assembled_bytes,
                "target_bytes": target_bytes,
                "score": 100.0,
                "error": None,
            }
        else:
            return {
                "success": False,
                "error": f"Byte mismatch at {len(mismatch_positions)} position(s)",
                "mismatch_positions": mismatch_positions,
                "assembled_bytes": assembled_bytes,
                "target_bytes": target_bytes,
            }

    finally:
        if os.path.exists(sfile):
            os.unlink(sfile)
        if os.path.exists(ofile):
            os.unlink(ofile)


# --- Auto-size by finding real function boundaries ---
def find_function_size(va: int, all_funcs: list[tuple]) -> int:
    """Find function size by scanning for jr $ra (first return)."""
    # Scan up to 0x2000 bytes max (512 instructions)
    max_scan = 0x2000
    try:
        data = extract_elf_func(va, max_scan)
        import capstone
        md = capstone.Cs(capstone.CS_ARCH_MIPS,
                         capstone.CS_MODE_MIPS64 + capstone.CS_MODE_LITTLE_ENDIAN)
        last_insn = 0
        for insn in md.disasm(data, va):
            if insn.mnemonic in ("jr", "jalr") and insn.op_str in ("$ra", "$31", "31"):
                last_insn = insn.address
                break
        if last_insn > 0:
            size = last_insn - va + 4
            if size % 4 != 0:
                size += 4 - (size % 4)
            return size
    except Exception:
        pass

    # Fallback: default to 0x200
    return 0x200


# --- Main conversion ---
def convert_function(func_name: str, va: int, category: str, size: int | None = None, save: bool = True):
    """Convert a single function to .s source and verify."""
    print(f"\n{'='*60}")
    print(f"Converting: {func_name} @ 0x{va:08X}")
    print(f"{'='*60}")

    # Auto-detect size if not provided
    if size is None:
        all_fvas = [(n, v) for n, v, _, _ in TARGET_FUNCTIONS]
        size = find_function_size(va, all_fvas)
        print(f"  Auto-detected size: 0x{size:X} ({size} bytes)")

    # Check if existing .s file is already byte-exact
    out_dir = ASM_OUTPUT_DIRS.get(category, os.path.join(SRC_ROOT, "entity", "asm"))
    existing_path = os.path.join(out_dir, f"{func_name}.s")
    if os.path.exists(existing_path):
        target_bytes = extract_elf_func(va, size)
        with open(existing_path) as f:
            existing_src = f.read()
        result = assemble_and_verify(existing_src, func_name, va, len(target_bytes))
        if result.get("success"):
            print(f"  ✓ EXISTING .s IS BYTE-EXACT — no regeneration needed")
            return result
        # Existing .s failed, regenerate from ELF
        print(f"  Existing .s failed verification, regenerating from ELF...")

    # Extract and disassemble
    target_bytes = extract_elf_func(va, size)
    insns = disassemble_mips64(target_bytes, va)

    print(f"  Instructions: {len(insns)}")
    print(f"  Target bytes: {len(target_bytes)}")

    # Generate .s source
    asm_source = generate_asm_source(func_name, insns, va)

    # Assemble and verify
    result = assemble_and_verify(asm_source, func_name, va, len(target_bytes))

    if result.get("success"):
        print(f"  ✓ ASSEMBLY OK — BYTE-EXACT MATCH (100%)")
    else:
        print(f"  ✗ FAILED: {result.get('error', 'unknown error')}")
        if "mismatch_positions" in result:
            for pos, a, b in result.get("mismatch_positions", [])[:5]:
                insn_idx = pos // 4
                print(f"    Byte {pos} (insn {insn_idx}): assembled=0x{a:02x}, target=0x{b:02x}")
            if len(result.get("mismatch_positions", [])) > 5:
                print(f"    ... and {len(result['mismatch_positions']) - 5} more")

    # Save .s file
    if save:
        out_dir = ASM_OUTPUT_DIRS.get(category, os.path.join(SRC_ROOT, "entity", "asm"))
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, f"{func_name}.s")

        with open(out_path, "w") as f:
            f.write(asm_source)

        print(f"  Saved: {out_path}")
    return result


def convert_all(save=True):
    """Convert all non-100% functions."""
    # Auto-detect sizes first
    funcs_with_size = auto_detect_sizes()
    print("Auto-detected sizes:")
    for name, va, size in funcs_with_size:
        print(f"  {name:25s} @ 0x{va:08X}  {size:5d} bytes ({size//4} insns)")

    results = []
    for name, va, size, category in TARGET_FUNCTIONS:
        result = convert_function(name, va, category, size, save=save)
        results.append((name, va, result.get("success", False)))
        if not result.get("success"):
            print(f"  *** {name} FAILED ***")

    print(f"\n{'='*60}")
    print("SUMMARY")
    print(f"{'='*60}")
    successes = sum(1 for _, _, s in results if s)
    failures = sum(1 for _, _, s in results if not s)
    print(f"  Success: {successes}/{len(results)}")
    print(f"  Failed:  {failures}/{len(results)}")
    for name, va, success in results:
        status = "✓" if success else "✗"
        print(f"  {status} {name:25s} @ 0x{va:08X}")
    return results


if __name__ == "__main__":
    no_save = "--no-save" in sys.argv
    if "--all" in sys.argv:
        convert_all(save=not no_save)
    elif "--auto-sizes" in sys.argv:
        funcs = auto_detect_sizes()
        print("Function sizes:")
        for name, va, size in funcs:
            print(f"  {name:25s} @ 0x{va:08X}  {size:5d} bytes ({size//4} insns)")
    elif len(sys.argv) >= 3:
        name = sys.argv[1]
        va = int(sys.argv[2], 16)
        size = int(sys.argv[3], 16) if len(sys.argv) >= 4 else 0x100
        category = "entity"
        for n, v, s, c in TARGET_FUNCTIONS:
            if n == name:
                category = c
                break
        convert_function(name, va, category, size, save=not no_save)
    else:
        print(f"Usage:")
        print(f"  {sys.argv[0]} <func_name> <va> [size]")
        print(f"  {sys.argv[0]} --all")
        print(f"  {sys.argv[0]} --auto-sizes")
        sys.exit(1)

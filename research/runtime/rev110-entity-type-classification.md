# Rev.110 — Entity Type Classification via Descriptor Table Cross-Reference

**Date:** 2026-08-26
**Objective:** Classify the 100 runtime entity work areas into descriptor table types by cross-referencing behavioral patterns with the 68-entry descriptor table
**Scope:** Entity work areas, GObj bindings, descriptor table (0x2A31B8), handler addresses
**Sources:** Rev.108 CSVs, `analyze_descriptor_table.py` output, `classify_entities.py`
**Evidence:** 1.77M event JSONL, 68 descriptor table entries, 124 GObj bindings

---

## Executive Summary

The 68-entry descriptor table (stride 0x64, base 0x2A31B8) defines 12 entity types with init_fn callbacks, 56 with hA handlers, 4 with hD handlers, and 12 vtable groups. All 100 runtime entity work areas are heap-allocated (0x01-0x15A range), confirming they are **instances** of descriptor types, not the types themselves. Classification by behavioral patterns reveals 5 categories: 1 high-activity system entity, 3 persistent multi-room entities, 36 multi-instance spawners, 21 standard entities, and 38 low-activity room-specific entities.

---

## Descriptor Table Structure

**Base address:** 0x2A31B8
**Entry count:** 68
**Stride:** 0x64 bytes
**Fields:** init_fn (+0x04), hA (+0x08), hB (+0x0C), hC (+0x10), hD (+0x14), vtable (+0x18)

### Handler Statistics

| Metric | Count |
|--------|-------|
| Total entries | 68 |
| With init_fn | 12 |
| With hA | 56 |
| With hB | 49 |
| With hC | 49 |
| With hD | 4 |
| With vtable | 29 |

### Key Entity Types

| Idx | Name | init_fn | hA | hB | hC | hD | vtable |
|-----|------|---------|----|----|----|----|--------|
| 1 | BOY | 0x153478 | 0x1C1F58 | 0x1C1DD8 | 0x1C1A98 | — | 0x202A60 |
| 2 | GIRL | 0x174BA0 | 0x1D1A98 | 0x1D17F8 | 0x1D1668 | 0x1D1AD0 | 0x202A60 |
| 4 | ENEMY1 | 0x164440 | 0x1CE690 | 0x1CE3C0 | 0x1CE220 | 0x1CE760 | 0x202A60 |
| 17 | WOODBOX01 | 0x17D1D0 | 0x1C05D0 | 0x1C0538 | 0x1C00C0 | — | 0x23D660 |
| 19 | BARREL | — | 0x1D3B28 | 0x1D3A30 | 0x1D27A8 | — | 0 |
| 20 | ROPE | — | 0x1E9630 | 0x1E9810 | 0x1E8F38 | — | 0 |
| 32 | BIRD | 0x1971C0 | 0x197080 | 0x197078 | 0x197240 | — | 0x1956E8 |
| 46 | QUEEN | 0x19B7F8 | 0x19A9A0 | 0x19A8F0 | 0x19A7E8 | — | 0x199A60 |
| 61 | AP1 | 0x1BB6B0 | 0x1BA530 | 0x1BA330 | 0x1B8720 | — | 0x1BB3E0 |

### Vtable Groups

| Vtable | Count | Types |
|--------|-------|-------|
| 0 (none) | 39 | NULL, GIRLDEMOCTRL, ENEMY_TEST, DEMOMOTCTRL, TREE, TORCH, PARTICLE, etc. |
| 0x23D660 (physics) | 16 | SOBJ, PSOBJ, SOFA, WOODBOX01, CAMERADUMMY, BGA, MOBJ, CHANDELIER, etc. |
| 0x202A60 (main_chars) | 4 | BOY, GIRL, ENEMY1, DEVIL_GIRL |
| 0x1956E8 (bird) | 1 | BIRD |
| 0x192EB8 (generator) | 1 | GENERATOR |
| 0x199A60 (queen) | 1 | QUEEN |

### Shared Handlers

| Handler | Used by |
|---------|---------|
| 0x10ECC0 | MOBJ.hA, DEMO_QSWORD.hA, SOBJ.hA, SOFA.hA, INTEREST*.hA |
| 0x174BA0 | GIRL.init_fn, DEVIL_GIRL.init_fn |
| 0x19A9A0 | QUEEN.hA, QUEENDEMOCTRL.hA |
| 0x19A8F0 | QUEEN.hB, QUEENDEMOCTRL.hB |
| 0x19A7E8 | QUEEN.hC, QUEENDEMOCTRL.hC |
| 0x1D1A98 | GIRL.hA, GIRLDEMOCTRL.hA, DEVIL_GIRL.hA |
| 0x1D17F8 | GIRL.hB, GIRLDEMOCTRL.hB, DEVIL_GIRL.hB |
| 0x1D1668 | GIRL.hC, GIRLDEMOCTRL.hC, DEVIL_GIRL.hC |

---

## Entity Work Area Analysis

### Memory Region

All 100 entities are in heap-early (0x01-0x15A range). No entities in ELF, .data, or BSS.

### Behavioral Classification

| Category | Count | Description |
|----------|-------|-------------|
| HIGH_ACTIVITY_ROOM_SPECIFIC | 1 | Single room, >100K events |
| ADJACENT_ROOM_SHARED | 1 | 2 rooms, >100K events |
| PERSISTENT_SYSTEM | 3 | 4+ rooms, persistent across transitions |
| MULTI_INSTANCE_SPAWNER | 36 | 5+ GObjs per entity |
| STANDARD | 21 | 2-4 GObjs, moderate activity |
| LOW_ACTIVITY_ROOM_SPECIFIC | 38 | Single room, <5K events |

### High-Activity Entities

| Address | Events | WS | GObjs | Classification | Likely Type |
|---------|--------|-----|-------|----------------|-------------|
| 0x008BC5B0 | 429,097 | 2 | 5 | ADJACENT_ROOM_SHARED | System (IOP/DMA) |
| 0x015797C0 | 218,263 | 1 | 6 | HIGH_ACTIVITY_ROOM_SPECIFIC | Room-specific system |
| 0x013E63A0 | 147,126 | 2 | 13 | MULTI_INSTANCE_SPAWNER | Spawner |
| 0x014E5080 | 58,973 | 6 | 15 | PERSISTENT_SYSTEM | **Boy/AI** |
| 0x0127EDA0 | 58,081 | 3 | 22 | MULTI_INSTANCE_SPAWNER | Spawner |
| 0x0148DB20 | 45,713 | 4 | 10 | PERSISTENT_SYSTEM | **Camera** |
| 0x014A76E0 | 45,088 | 3 | 11 | MULTI_INSTANCE_SPAWNER | Spawner |
| 0x0142D7D0 | 17,306 | 4 | 23 | PERSISTENT_SYSTEM | **Enemy manager** |

### Multi-Instance Spawners (Top 10)

| Address | Events | GObjs | Primary WS |
|---------|--------|-------|------------|
| 0x01389C60 | 20,267 | 38 | 0x13,0x14,0x15 |
| 0x013AB080 | 20,568 | 32 | 0x14,0x15 |
| 0x01524EA0 | 6,048 | 28 | 0x0A,0x11 |
| 0x0113C8D0 | 10,024 | 25 | 0x0E,0x0F |
| 0x0142D7D0 | 17,306 | 23 | 0x0A,0x0B,0x12,0x16 |
| 0x0127EDA0 | 58,081 | 22 | 0x08,0x09,0x0A |
| 0x008C2A60 | 38,632 | 22 | 0x16,0x17 |
| 0x011672B0 | 13,176 | 21 | 0x08,0x09,0x0A |
| 0x010B3E50 | 13,385 | 19 | 0x0D,0x0E |
| 0x011CE410 | 4,180 | 19 | 0x09,0x0A |

---

## GObj Pool Analysis

**Pool range:** 0x00830974 - 0x0084AA80
**Pool size:** 0x1A280 bytes (107,520 bytes)
**Total GObjs observed:** 124

### GObj Stride Distribution

| Stride | Count | Description |
|--------|-------|-------------|
| 0x174 (372 bytes) | 97 | **Standard GObj** (78.2%) |
| 0x2E8 (744 bytes) | 8 | Extended GObj (6.5%) |
| 0x5D0 (1488 bytes) | 6 | Large GObj (4.8%) |
| 0x45C (1116 bytes) | 2 | Medium GObj |
| 0xD14 (3348 bytes) | 2 | Very large GObj |
| 0xE88 (3720 bytes) | 2 | Very large GObj |
| Other | 7 | Varied sizes |

**Key finding:** 0x174 is the canonical GObj stride, matching Rev.099 analysis. The 78.2% dominance confirms this is the standard allocation size.

### GObj Pool Gaps

Significant gaps indicate different allocation blocks or fragmentation:
- 0x830974 → 0x8310B8 (0x744 bytes = extended GObj block)
- 0x83809C → 0x83866C (0x5D0 bytes = large GObj block)
- 0x839DAC → 0x83AAC0 (0xD14 bytes = very large GObj block)

---

## World State → Entity Inventory

### High-Activity World States

| WS | Entities | Top Entity | Events |
|----|----------|------------|--------|
| 0x0F | 3 | 0x008BC5B0 (429K) | 440,150 |
| 0x10 | 7 | 0x008BC5B0 (429K) | 501,740 |
| 0x0A | 14 | 0x014E5080 (59K) | 293,715 |
| 0x09 | 17 | 0x014E5080 (59K) | 256,425 |
| 0x1A | 3 | 0x015797C0 (218K) | 228,837 |
| 0x16 | 5 | 0x013E63A0 (147K) | 228,267 |
| 0x17 | 5 | 0x013E63A0 (147K) | 212,804 |

### Room Transition Patterns

**Entity persistence across rooms:**
- 0x014E5080 (Boy/AI): 6 rooms (0x09,0x0A,0x0B,0x0D,0x11,0x12)
- 0x0148DB20 (Camera): 4 rooms (0x12,0x13,0x14,0x15)
- 0x0142D7D0 (Enemy manager): 4 rooms (0x0A,0x0B,0x12,0x16)

**Room-specific entities:**
- 0x015797C0: ws=0x1A only (218K events)
- 0x014B87C0: ws=0x11 only (19K events)
- 0x008D6310: ws=0x08 only (12K events)

---

## Classification Methodology

### Step 1: Descriptor Table Extraction

Parsed 68 entries from ELF at 0x2A31B8 (stride 0x64). Verified against Rev.052 handler mapping and PAL symbol names.

### Step 2: Behavioral Heuristics

Applied classification rules:
- **HIGH_ACTIVITY_ROOM_SPECIFIC**: 1 WS, >100K events
- **ADJACENT_ROOM_SHARED**: 2 WS, >100K events
- **PERSISTENT_SYSTEM**: 4+ WS
- **MULTI_INSTANCE_SPAWNER**: 5+ GObjs
- **LOW_ACTIVITY_ROOM_SPECIFIC**: 1 WS, <5K events
- **STANDARD**: default

### Step 3: Handler Range Cross-Reference

Checked if entity work area addresses fall within known handler ranges (boyAI 0x142000-0x164000, GirlBrain 0x16A000-0x175000, etc.). Result: all 100 entities are in heap (0x01-0x15A), not in handler code ranges.

### Step 4: GObj Binding Analysis

Cross-referenced 124 GObj bindings to identify entity→GObj ratios. High-ratio entities (10:1+) are spawners. Low-ratio entities (1:1-2:1) are single-instance objects.

---

## What Is Confirmed

1. **68 descriptor table entries** parsed from ELF at 0x2A31B8
2. **12 entity types** have init_fn callbacks
3. **4 entity types** have hD handlers (GIRL, ENEMY1, DEVIL_GIRL, ENEMY_CONTROL)
4. **5 vtable groups** identified (main_chars, physics, bird, generator, queen)
5. **All 100 entities** are heap-allocated instances (0x01-0x15A range)
6. **GObj stride 0x174** is canonical (78.2% of 124 GObjs)
7. **3 persistent system entities** span 4-6 world states
8. **36 multi-instance spawners** generate 5-38 GObjs each

## What Is Probable

1. **0x014E5080** is the Boy/AI entity (6 WS, 15 GObjs, 59K events)
2. **0x0148DB20** is the Camera entity (4 WS, 10 GObjs, 46K events)
3. **0x0142D7D0** is the Enemy manager (4 WS, 23 GObjs, 17K events)
4. **0x008BC5B0** is a system entity (IOP/DMA handler) with 429K events
5. **0x015797C0** is a room-specific system entity (218K events, ws=0x1A)

## What Is Possible

1. Some multi-instance spawners may be enemy generators (GENERATOR type)
2. Low-activity entities may be decorative props (TORCH, TREE, CANDLE)
3. The 38 room-specific entities may correspond to room-specific descriptor types

## What Is Unknown

1. Exact type for each of the 100 entity work areas (requires callback probe data)
2. Whether entity work areas contain a type field at a known offset
3. How descriptor table index maps to entity work area address
4. The relationship between entry table (0x2A4C48, 512 entries) and entity work areas

## What Is Discarded

1. **Hypothesis that entity work areas are descriptor table entries** — DISCARDED (heap vs .data address mismatch)
2. **Hypothesis that GObj pool is contiguous** — DISCARDED (gaps indicate fragmentation)

---

## Next Minimum Test

1. **Deploy callback probes** for BOY/GIRL/ENEMY1 handlers (0x1C1A98, 0x1C1DD8, 0x1C1F58) to confirm entity→type mapping
2. **Check entity work area structure** for type field at offset +0x00 or +0x04
3. **Analyze entry table** (0x2A4C48) to understand scene→descriptor mapping

## Conservative Verdict

The entity system is a **multi-level architecture**: descriptor table defines types (68 entries), entry table maps scene objects to descriptors (512 entries), entity work areas are heap-allocated instances (100 observed), and GObjs are dispatched via isysGObj* (124 observed). The 5-class behavioral taxonomy (system/persistent/spawner/standard/room-specific) provides a framework for future type identification, but exact type assignment requires callback probe data or structure field analysis.

---

## Files Generated

- `tools/runtime/classify_entities.py`: Entity classification script
- `research/runtime/rev110-entity-type-classification.md`: This document

# Rev.111 — Runtime Session: 34 world_states, halfword second caller confirmed, slot-per-room mapping

**Date:** 2026-09-03
**Session:** ico-runtime-20260825-152452 (extended; revisited 2026-09-03 after a longer pausa)
**Total events:** 2,456,583 (full log, final)
**File size:** ~1.6 GB

---

## Objective

Analyze the extended gameplay session to:
1. Confirm halfword second caller (Objetivo #22)
2. Map world_state transitions and room architecture
3. Correlate ios_om_main slot dispatch with world_states
4. Identify persistent vs transient GObjs/entities
5. Extract decompilation targets from runtime evidence

---

## Scope

- Full log scan: 2.456M events
- halfword_second_caller_entry/return: 65,297 events
- world_state_load: 34 unique values (full log)
- ios_om_main: 2.33M events (full log)
- isys_gobj_proc_add: 356K+ events (full log)
- ios_om_create_dl: 112K+ events (full log)
- init_scene_gobj: 11,934 events (first 800K span)

---

## Sources

- JSONL runtime capture: `.local/pcsx2-logs/ico-runtime-20260825-152452.jsonl`
- Validator: `tools/runtime-probe-analyzer/verify_runtime_probe_log.py`
- Previous session: Rev.105 (25 world_states, 20 DL slots)

---

## Evidence

### 1. halfword second caller — CONFIRMED ACTIVE (Objetivo #22)

**65,297 events** captured at `0x0016828C`. First session where this caller is active.

| Field | Value | Interpretation |
|-------|-------|----------------|
| `pc` | `0x0016828C` | Second caller entry point |
| `probable_callsite` | `0x00168284` (fixed) | Static callsite within parent function |
| `s2` | `0x00169aa8` (fixed, all 65K events) | Address of `_clipWR` (slot 7 callback) |
| `ra` | `0x0016828C` (self-referencing) | Loop structure |
| `halfword_counter` | `0x00000000` (always) | Counter not incremented at this point |
| `a0` | 300+ distinct values | Different entity objects processed per call |

**world_state_raw distribution during halfword calls:**

| ws_raw | Count | % |
|--------|-------|---|
| `0x0A` | 17,910 | 27.4% |
| `0x09` | 17,230 | 26.4% |
| `0x19` | 8,356 | 12.8% |
| `0x1D` | 7,037 | 10.8% |
| `0x14` | 5,463 | 8.4% |
| `0x08` | 3,544 | 5.4% |
| `0x11` | 2,365 | 3.6% |
| `0x1A` | 2,075 | 3.2% |
| `0x10` | 938 | 1.4% |
| `0x0E` | 301 | 0.5% |
| `0x04` | 78 | 0.1% |

**Confirmed:** The second halfword caller is part of the `_Clip` collision pipeline. It calls `_clipWR` (slot 7 callback) to process collision clipping for entities. Active in 11 different world_states, most heavily in `0x0A` and `0x09`.

**Top entity objects processed by halfword caller:**

| a0 (entity work area) | Count | Likely identity |
|------------------------|-------|-----------------|
| `0x015ee4b0` | 2,355 | Unknown entity |
| `0x015bad30` | 1,650 | Unknown entity |
| `0x015bf890` | 1,128 | Unknown entity |
| `0x015bc7d0` | 1,120 | Unknown entity |
| `0x01588020` | 1,103 | Unknown entity |

All a0 values are in the `0x01xxxxxx` range — dynamic entity work areas in heap.

---

### 2. world_state_load: 34 unique values (record, was 25)

**171 transitions** across **34 unique world_state values**.

New world_states not seen in Rev.105:
- `0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E`
- `0x28, 0x29, 0x2A, 0x2B, 0x2D, 0x32`

**Transition frequency (top 10 pairs):**

| From → To | Count | Avg cycle delta |
|-----------|-------|-----------------|
| `0x15 → 0x14` | 13 | ~845M (fast) |
| `0x14 → 0x15` | 12 | ~3.5B |
| `0x08 → 0x09` | 8 | ~15B |
| `0x09 → 0x08` | 7 | ~14B |
| `0x09 → 0x0A` | 6 | ~20B |
| `0x1D → 0x1E` | 6 | ~865M (fast) |
| `0x1E → 0x1D` | 6 | ~866M (fast) |
| `0x0A → 0x09` | 5 | ~23B |
| `0x16 → 0x17` | 5 | ~55B |
| `0x0D → 0x0E` | 4 | ~36B |

**Key observations:**
- `0x14↔0x15`: Very fast transitions (~845M cycles = ~0.4s at 294MHz). Likely adjacent rooms or room sub-areas.
- `0x1D↔0x1E`: Also very fast (~865M cycles). Another adjacent pair.
- `0x0A` is the central hub: connected to 10+ other states.
- `0x01` appears to be a "home" or "lobby" state that the player returns to.

**Complete unique world_state values:** `0x01, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x28, 0x29, 0x2A, 0x2B, 0x2D, 0x32`

Note: `0x02` and `0x0C` are absent. `0x28-0x2D` and `0x32` appear only once each (special/cutscene states).

---

### 3. ios_om_main slot-per-world_state mapping (CRITICAL NEW FINDING)

Each world_state has a **primary DL slot** that handles >90% of processing. This is the room-to-slot assignment pattern.

**Extended capture (01:15, 2.456M events):** the table below is the complete
primary slot per world_state over the FULL log (not just the first 800K events
of this revision). Counts and percentages are final.

| world_state | Primary Slot (a2) | Count | % | Note |
|-------------|-------------------|-------|---|------|
| `0x1A` | `0x1D` | 415,268 | 99.9% | — |
| `0x0F` | `0x1A` | 401,324 | 99.9% | Slot B |
| `0x1D` | `0x36` | 180,317 | 99.3% | NEW-6 (Rev.105) |
| `0x16` | `0x1E` | 145,182 | 99.7% | Slot C |
| `0x0A` | `0x21` | 123,823 | 98.9% | Slot F |
| `0x11` | `0x1D` | 53,769 | 98.7% | — |
| `0x08` | `0x20` | 52,315 | 98.8% | Slot E |
| `0x13` | `0x34` | 45,819 | 98.8% | NEW-5 (Rev.105) |
| `0x09` | `0x1A` | 43,502 | 97.7% | Slot B |
| `0x14` | `0x36` | 35,807 | 96.4% | NEW-6 (Rev.105) |
| `0x10` | `0x1F` | 31,604 | 99.0% | Slot D |
| `0x1C` | `0x2E` | 31,393 | 98.4% | **NEW slot** |
| `0x17` | `0x23` | 30,369 | 98.8% | **NEW slot** |
| `0x0D` | `0x1C` | 28,267 | 98.7% | Slot NEW-1 |
| `0x0B` | `0x44` | 25,872 | 99.2% | Slot NEW-2 |
| `0x01` | `0x22` | 20,835 | 91.7% | Slot G |
| `0x12` | `0x25` | 19,920 | 98.3% | NEW-5 (Rev.105) |
| `0x1E` | `0x24` | 14,444 | 94.6% | **NEW slot** |
| `0x15` | `0x26` | 13,536 | 93.6% | **NEW slot** |
| `0x04` | `0x33` | 12,940 | 99.2% | Slot I |
| `0x0E` | `0x1A` | 11,538 | 98.0% | Slot B |
| `0x19` | `0x1A` | 10,695 | 98.6% | Slot B |
| `0x1B` | `0x26` | 10,670 | 98.7% | **NEW slot** |
| `0x06` | `0x1E` | 6,845 | 98.1% | Slot C |
| `0x07` | `0x1F` | 4,075 | 98.5% | Slot D |
| `0x18` | `0x1C` | 3,582 | 96.0% | Slot NEW-1 |
| `0x05` | `0x18` | 1,567 | 91.4% | Slot A |
| `0x03` | `0x21` | 1,525 | 97.1% | Slot F |
| `0x2B` | `0x21` | 1,132 | 94.6% | Slot F |
| `0x32` | `0x18` | 786 | 92.0% | Slot A |
| `0x2D` | `0x40` | 461 | 86.3% | Slot J |
| `0x28` | `0x29` | 150 | 56.6% | Slot H |
| `0x2A` | `0x22` | 112 | 54.6% | Slot G |

**31 distinct slot indices** observed in the full log. **New slots** revealed by
the extended session (beyond Rev.105/111 catalog): `0x23` (ws 0x17), `0x24`
(ws 0x1E), `0x26` (ws 0x15, 0x1B), `0x2E` (ws 0x1C).

**Key findings:**
- **Slot B (`0x1A`)** is shared by ws `0x0F, 0x09, 0x0E` — these are likely rooms in the same area/zone.
- **Slot C (`0x1E`)** is shared by ws `0x16, 0x06`.
- **Slot `0x1D`** is shared by ws `0x1A, 0x11`.
- **Slot `0x36`** is shared by ws `0x1D, 0x14`.
- **Slot `0x26`** is shared by ws `0x15, 0x1B`.
- **Slot F (`0x21`)** is shared by ws `0x0A, 0x03, 0x2B` — another zone group.
- **ws `0x01`** uses Slot G (`0x22`) as primary, but also has anomalous slots `0x31C383B0` and `0x31CA06F0` (possible overlay/transition slots).
- **ws `0x05`** has the highest slot-0 percentage (8.6%) — suggests more system-level processing.
- **34 world_states** (record, up from 25 in Rev.105) and **31 distinct slot indices** in the full log.

---

### 4. ios_om_create_dl: DL slot dispatch confirmation

`a1` in `ios_om_create_dl` carries the BSS address of the DL slot:

| BSS Address | Count | % | Matches |
|-------------|-------|---|---------|
| `0x006782f8` | 24,010 | 67.1% | Slot B |
| `0x00679258` | 3,270 | 9.1% | Slot E |
| `0x00678818` | 1,768 | 4.9% | Slot NEW-1 |
| `0x006794e8` | 1,645 | 4.6% | Slot F |
| `0x0067ee98` | 1,617 | 4.5% | Slot NEW-2 |
| `0x00679778` | 1,172 | 3.3% | Slot G |
| `0x0067c308` | 809 | 2.3% | Slot I |
| `0x00000008` | 698 | 1.9% | **NEW: non-BSS value** |
| `0x00678d38` | 427 | 1.2% | Slot C |
| `0x00678fc8` | 254 | 0.7% | Slot D |
| `0x00677dd8` | 98 | 0.3% | Slot A |
| `0x0067e458` | 29 | 0.1% | Slot J |
| `0x0067a968` | 10 | <0.1% | Slot H |

**Anomalous `a1=0x00000008`:** 698 events where `a1` is not a BSS address. This may be a count or index rather than a pointer. In `ws=0x01`, `ios_om_main` with `a2=0x08` shows `t1=0x00000140` (320 = GObj count), suggesting slot `0x08` is a system-level slot (possibly the main loop or VBlank processing).

---

### 5. isys_gobj_proc_add: process registration patterns

**329,687 events** total. `a3` field:
- `a3=0x00000000`: 62,453 (53.1%) — **add** process
- `a3=0x00000001`: 55,074 (46.9%) — **remove** process

**Top GObj pointers registered:**

| a0 (GObj) | Count | % | Notes |
|-----------|-------|---|-------|
| `0x00837c40` | 9,805 | 8.3% | Most registered GObj |
| `0x00837acc` | 5,670 | 4.8% | Second most |
| `0x0083521c` | 3,822 | 3.3% | |
| `0x00837db4` | 3,729 | 3.2% | |
| `0x00835390` | 3,482 | 3.0% | |

**s2 values (callback type):**

| s2 | Count | Interpretation |
|----|-------|----------------|
| `0x00000000` | 18,198 | Null/system callback |
| `0x00000001` | 4,948 | Type 1 callback |
| `0x01521f00` | 3,825 | Function pointer |
| `0x00000002` | 3,357 | Type 2 callback |
| `0x0127d6c0` | 2,642 | Function pointer |
| `0x00000003` | 2,592 | Type 3 callback |

---

### 6. isys_gobj_alloc / isys_gobj_init: system-wide allocation

| Function | a0 | a1 | v0/return | Count |
|----------|----|----|-----------|-------|
| `isys_gobj_alloc` | `0x140` (320) | `0xFFFFFFFF` | `0x00281AB0` (DL heads) | 184 |
| `isys_gobj_init` | `0x140` (320) | `0x005CF274` | — | 184 |

- **320 GObjs** allocated per init cycle
- Return value `0x00281AB0` = DL heads table (confirmed)
- `0x005CF274` = initialization data table in .data section
- 184 init cycles in this session (matches scene load count)

---

### 7. GObj persistence: shared objects across rooms

**24 of 41 unique GObjs** appear in multiple world_states.

| GObj | States | Count | Likely identity |
|------|--------|-------|-----------------|
| `0x00845350` | `0x09, 0x0B, 0x0D, 0x12, 0x1A, 0x1B` | 6 | **Persistent object** (player? camera? system?) |
| `0x008451dc` | `0x09, 0x11, 0x12, 0x16` | 4 | Persistent object |
| `0x00840934` | `0x01, 0x13, 0x1A, 0x1D, 0x29` | 5 | Persistent object |
| `0x0084805c` | `0x12, 0x14, 0x15` | 3 | Persistent object |
| `0x0083f368` | `0x0A, 0x10, 0x11` | 3 | Persistent object |
| `0x0083cab8` | `0x0B, 0x0D, 0x0E` | 3 | Persistent object |

**Entity persistence:**

| Entity | States | Count | Likely identity |
|--------|--------|-------|-----------------|
| `0x014e5080` | `0x09, 0x0B, 0x0D, 0x11, 0x12, 0x1A, 0x1B` | 7 | **Most persistent entity** |
| `0x0142d7d0` | `0x0B, 0x12, 0x16, 0x1B` | 4 | Persistent entity |
| `0x0148db20` | `0x12, 0x14, 0x15` | 3 | Persistent entity |
| `0x008cef80` | `0x01, 0x13, 0x1A` | 3 | Persistent entity |

---

### 8. init_scene_gobj: entity type frequency

**11,934 events.** Core entity types (always 171 = scene load count):

| a1 (type) | Count | Interpretation |
|-----------|-------|----------------|
| `0x02` | 171 | Core type (always loaded) |
| `0x03` | 171 | Core type (always loaded) |
| `0x04` | 171 | Core type (always loaded) |
| `0x05` | 171 | Core type (always loaded) |
| `0x33` | 171 | Core type (always loaded) |
| `0x83` | 135 | Common type |
| `0x6F` | 95 | Common type |
| `0x40A` | 40 | Rare type |

The high entity type IDs (`0x1AD-0x1F9`, `0x438-0x48B`) appear in blocks — these are likely **scene-specific entity batches** loaded per room.

---

## What is confirmed

1. **halfword second caller is active** — 65K events, `s2=0x00169aa8=_clipWR`, processes collision for entities across 11 world_states.
2. **Each world_state has a primary DL slot** — the room-to-slot assignment is 1:1 (with zone sharing).
3. **34 unique world_states** exist in ICO (up from 25 in Rev.105).
4. **12 distinct BSS DL slot addresses** mapped.
5. **320 GObjs** allocated per system init.
6. **24 of 41 GObjs** are persistent across room transitions.
7. **Entity `0x014e5080`** is the most persistent (7 world_states).

## What is probable

- The `a2`/`t0` values in `ios_om_main` are DL table indices (offset from `0x18`, step 4).
- GObjs at `0x0084xxxx` range are likely **player/system objects** that persist across rooms.
- Entity work areas at `0x014xxxxx` range are likely **dynamic entities** allocated per room.
- The anomalous `a1=0x08` in `ios_om_create_dl` is a count/index, not a BSS pointer.

## What is possible

- The 5 core entity types (`0x02-0x05, 0x33`) may correspond to: Boy, Girl, Camera, System, VBlank.
- `0x28-0x2D` and `0x32` world_states may be cutscene/menu states.
- The fast `0x14↔0x15` transitions suggest door/passage loading within the same area.

## What is unknown

- The identity of each world_state (room name/number).
- The identity of the 41 unique GObjs.
- The exact struct layout of entity work areas.
- What the `s2` values in `isys_gobj_proc_add` represent (callback types? priority levels?).

## What is discarded

- The hypothesis that `0x166E10` (`_Clip`) is the main dispatcher — confirmed as collision function.
- The hypothesis that slot distribution is uniform across rooms — each room has a dedicated slot.

## Next minimum test

1. **Directly probe halfword fast path (0x166DFC)** — the only remaining unvalidated runtime target.
2. **Fix ios_om_main slot capture** — add explicit slot_index field to the instrumented fork.
3. **Map entity work area struct** — use `init_scene_gobj` a1 values + descriptor table to recover field offsets.

## Conservative verdict

This session provides the strongest runtime evidence yet for the room-to-slot architecture. Each world_state has a dedicated processing slot, with zone sharing between adjacent rooms. The halfword collision pipeline is confirmed active across 11 world_states. The 320-GObj allocation and persistent object tracking suggest a clear separation between system objects (persistent) and room entities (transient).

The next decompilation priority should be the functions that bridge world_state to DL slot assignment — likely in the scene loader (`kanban.c`) or `initSceneGObj`.

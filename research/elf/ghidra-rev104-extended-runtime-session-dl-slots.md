# Rev.104 — Extended Runtime Session: 7 New World States, 2 New DL Slots

## Date
2026-08-25

## Objective
Analyze the 203K new events captured during the extended gameplay session (552K → 755K events), identifying new world_states, DL slot patterns, and dispatch behavior changes.

## Scope
- Full session log: `ico-runtime-20260825-152452.jsonl` (755,778 events)
- Labels analyzed: `ios_om_main`, `isys_gobj_proc_add`, `ios_om_create_dl`, `isys_gobj_add`, `init_scene_gobj`, `world_state_load`
- Session duration: ~5.3 hours of gameplay (from boot to whirlpool area)

## Sources Used
- `tools/runtime-probe-analyzer/verify_runtime_probe_log.py`
- Manual analysis of world_state_load timeline
- Register pattern analysis of ios_om_main (a1, a2, t0 registers)

## Evidence

### 1. Seven New World States

Rev.103 documented 13 unique world_state values (0x00-0x07, 0x28-0x2B, 0x2D, 0x39). This session discovered **7 new values**:

| World State | Rev.103 Status | Rev.104 Status | Total Events |
|-------------|----------------|----------------|--------------|
| 0x08 | — | NEW | 52,934 ios_om_main |
| 0x09 | — | NEW | 42,298 ios_om_main |
| 0x0A | — | NEW | 24,094 ios_om_main |
| 0x0B | — | NEW | 26,083 ios_om_main |
| 0x0D | — | NEW | 28,646 ios_om_main |
| 0x0E | — | NEW | 11,774 ios_om_main |
| 0x0F | — | NEW | **304,336 ios_om_main** (57% of total) |

**Total unique world_states**: 20 (was 13)

### 2. World State Transition Timeline

Complete sequence of all 67 world_state_load events with cycle-accurate timing:

| # | Cycle | Duration | ws | Event |
|---|-------|----------|----|-------|
| 1 | 3.4B | — | 0x01 | Boot/initial scene |
| 2-11 | 3.4B-113B | — | 0x01 | Boot re-inits (11 total) |
| 12 | 28.8B | 18.1s | 0x29 | Jail corridor A |
| 13 | 30.7B | 5.6s | 0x2A | Jail corridor B |
| 14 | 32.7B | 6.0s | 0x2B | Warehouse approach |
| 15 | 44.5B | 34.9s | 0x2D | Warehouse/meet Yorda |
| 16 | 49.8B | 15.6s | 0x28 | Bridge area |
| 17 | 52.4B | 7.7s | 0x03 | Post-bridge room A |
| 18 | 67.9B | 45.7s | 0x05 | Post-bridge room B |
| 19 | 78.2B | 30.5s | 0x04 | Room revisit |
| 20 | 143.2B | 191.8s | 0x05 | Back to 0x05 |
| 21 | 149.7B | 19.5s | 0x04 | Back to 0x04 |
| 22 | 213.4B | 188.0s | 0x06 | Save point area |
| 23 | 277.3B | 188.6s | 0x07 | Post-save area |
| 24 | 318.1B | 120.4s | 0x06 | Back to 0x06 |
| 25 | 322.8B | 14.5s | 0x08 | **NEW: Water area A** |
| 26 | 754.3B | **1273.4s** | 0x09 | **NEW: Water area B** (21 min!) |
| 27 | 772.4B | 53.7s | 0x0A | **NEW: Water area C** |
| 28-44 | 772B-1284B | — | 0x08↔0x09↔0x0A | Heavy oscillation (puzzle?) |
| 45 | 1314.9B | 15.7s | 0x0B | **NEW: Water area D** |
| 46-54 | 1315B-1669B | — | 0x0A↔0x0B↔0x0D | Continued oscillation |
| 55 | 1669.0B | 28.5s | 0x0D | **NEW: Water area E** |
| 56-58 | 1669B-1788B | — | 0x0D↔0x0E | **NEW: Water area F** |
| 59-66 | 1910B-2071B | — | 0x0D↔0x0E | Continued oscillation |
| 67 | 2155.6B | 248.8s | 0x0F | **NEW: Current area (whirlpool)** |

### 3. New World State Characterization

#### ws=0x08, 0x09 (Water Area A/B)
- **Heaviest entity load**: 731+973 init_scene_gobj inits
- **Longest dwell time**: ws=0x09 sustained for 1273.4s (21.2 minutes) — longest single state in session
- **Heavy oscillation**: 0x08↔0x09↔0x0A transitions suggest puzzle or backtracking
- **Slot B dominant**: 85.6% of slot B activity during ws=0x0F, but ws=0x09 also uses slot B significantly

#### ws=0x0A, 0x0B (Water Area C/D)
- **624+252 init_scene_gobj inits**
- **2,644+1,630 ios_om_create_dl events**
- **ws=0x0B has NEW DL slot 0x67EE98** — unique to this state

#### ws=0x0D, 0x0E (Water Area E/F)
- **384+185 init_scene_gobj inits**
- **ws=0x0D has NEW DL slot 0x678818** — unique to this state
- **Oscillation pattern**: 0x0D↔0x0E (6 transitions)

#### ws=0x0F (Current Area — Whirlpool)
- **304,336 ios_om_main events** — 57% of all session activity
- **20,091 ios_om_create_dl events** — highest of any state
- **Duration**: 3168.9B cycles (~9356 seconds, 2.6 hours)
- **Single slot**: ALL ios_om_main events during ws=0x0F go through slot B (0x6782F8)
- **Only 1,133 proc_add events** — relatively few callback registrations despite massive dispatch volume

### 4. Two New DL Slot Addresses

Rev.103 documented 10 DL slot addresses (A-J). This session discovered **2 new slots**:

| Slot | Address | a2/t0 Index | First Seen | Unique WS | ios_om_main Count |
|------|---------|-------------|------------|-----------|-------------------|
| A | 0x677DD8 | 0x18 | Rev.089 | all | 1,567 |
| B | 0x6782F8 | 0x1A | Rev.089 | 0x09, 0x0E, 0x0F | 361,536 |
| **NEW-1** | **0x678818** | **0x1C** | Rev.104 | **0x0D only** | **28,267** |
| C | 0x678D38 | 0x1E | Rev.089 | — | 6,845 |
| D | 0x678FC8 | 0x1F | Rev.089 | — | 4,075 |
| E | 0x679258 | 0x20 | Rev.089 | — | 52,315 |
| F | 0x6794E8 | 0x21 | Rev.089 | — | 26,292 |
| G | 0x679778 | 0x22 | Rev.089 | — | 18,753 |
| H | 0x67A968 | 0x29 | Rev.089 | — | 150 |
| I | 0x67C308 | 0x33 | Rev.089 | — | 12,940 |
| J | 0x67E458 | 0x40 | Rev.089 | — | 461 |
| **NEW-2** | **0x67EE98** | **0x44** | Rev.104 | **0x0B only** | **25,872** |

**Total DL slots**: 12 (was 10)

### 5. DL Slot Address Layout (BSS)

The 12 DL slot addresses are NOT evenly spaced:

| Slot | Address | Offset from Previous | Stride |
|------|---------|---------------------|--------|
| A | 0x677DD8 | — | — |
| B | 0x6782F8 | +0x520 (1312) | 1312 |
| NEW-1 | 0x678818 | +0x520 (1312) | 1312 |
| C | 0x678D38 | +0x520 (1312) | 1312 |
| D | 0x678FC8 | +0x290 (656) | 656 |
| E | 0x679258 | +0x290 (656) | 656 |
| F | 0x6794E8 | +0x290 (656) | 656 |
| G | 0x679778 | +0x290 (656) | 656 |
| H | 0x67A968 | +0x11F0 (4592) | — |
| I | 0x67C308 | +0x19A0 (6560) | — |
| J | 0x67E458 | +0x2150 (8528) | — |
| NEW-2 | 0x67EE98 | +0xA40 (2624) | — |

**Pattern**: Slots A-B-NEW-1-C form a contiguous block with stride 1312. Slots D-E-F-G form another contiguous block with stride 656. Slots H-I-J-NEW-2 are scattered.

### 6. a2/t0 Register = DL Slot Type Index

The `a2` and `t0` registers at `_iosOmMain` entry consistently encode the DL slot type:

| a2/t0 Value | Slot Address | Notes |
|-------------|--------------|-------|
| 0x18 (24) | A (0x677DD8) | |
| 0x1A (26) | B (0x6782F8) | Most active slot |
| 0x1C (28) | NEW-1 (0x678818) | ws=0x0D only |
| 0x1E (30) | C (0x678D38) | |
| 0x1F (31) | D (0x678FC8) | |
| 0x20 (32) | E (0x679258) | |
| 0x21 (33) | F (0x6794E8) | |
| 0x22 (34) | G (0x679778) | |
| 0x29 (41) | H (0x67A968) | |
| 0x33 (51) | I (0x67C308) | |
| 0x40 (64) | J (0x67E458) | |
| 0x44 (68) | NEW-2 (0x67EE98) | ws=0x0B only |

The a2 values are NOT sequential slot indices. They may represent:
- Bitmask positions in a 64-bit or 128-bit type mask
- Entry indices into a dispatch table
- Combined type+priority encoding

### 7. proc_add Lifecycle Balance

| a3 Value | Count | Meaning |
|----------|-------|---------|
| 0x00000000 | 62,453 | proc_add (registration) |
| 0x00000001 | 55,074 | proc_remove (deregistration) |

**Ratio**: 53% add / 47% remove — nearly balanced, confirming proper lifecycle management.

### 8. World State → Entity Load Correlation

| World State | init_scene_gobj | ios_om_create_dl | proc_add | ios_om_main |
|-------------|-----------------|-------------------|----------|-------------|
| 0x01 (boot) | 461 | 1,602 | 11,425 | — |
| 0x08 (water A) | 731 | 3,308 | 20,897 | 52,934 |
| 0x09 (water B) | 973 | 2,644 | 21,347 | 42,298 |
| 0x0A (water C) | 624 | 1,506 | 17,918 | 24,094 |
| 0x0B (water D) | 252 | 1,630 | 6,026 | 26,083 |
| 0x0D (water E) | 384 | 1,789 | 15,878 | 28,646 |
| 0x0E (water F) | 185 | 737 | 7,737 | 11,774 |
| 0x0F (whirlpool) | — | 20,091 | 1,133 | **304,336** |

**Key insight**: ws=0x0F has the highest dispatch volume (304K) but the LOWEST proc_add count (1,133). This means the game is running a very active dispatch loop with relatively few callback registrations — likely a gameplay-heavy area (whirlpool combat/puzzle) rather than a scene-loading area.

## What is Confirmed

1. **20 unique world_state values** now documented (was 13)
2. **12 DL slot addresses** now mapped (was 10)
3. **a2/t0 register** at `_iosOmMain` entry encodes the DL slot type index
4. **ws=0x0F** is the most active state (57% of all dispatch events)
5. **ws=0x08↔0x09↔0x0A oscillation** suggests puzzle or backtracking gameplay
6. **ws=0x0B** has unique DL slot 0x67EE98
7. **ws=0x0D** has unique DL slot 0x678818
8. **proc_add lifecycle** is balanced (53% add / 47% remove)
9. **DL slots form clusters** in BSS with two stride patterns (1312 and 656 bytes)

## What is Probable

1. ws=0x08-0x0F represent **water/aqueduct areas** of the game (based on the user playing through to the whirlpool)
2. The oscillation patterns (0x08↔0x09, 0x0D↔0x0E) suggest **room transitions within a connected area**
3. The a2/t0 values may be **bitmask positions** (0x18=24, 0x1A=26, 0x1C=28, etc.)

## What is Possible

1. The scattered H-I-J-NEW-2 slots may be for **specialized subsystems** (camera, UI, audio)
2. The stride patterns (1312 vs 656) may indicate **different GObj sizes** for different entity types

## What is Unknown

1. The semantic meaning of each world_state value (which room/area each represents)
2. Why ws=0x0F has such massive dispatch volume but minimal proc_add
3. The exact structure of the BSS DL slot data (what fields each slot contains)
4. Whether the a2/t0 values map to a specific table in the binary

## What is Discarded

1. Previous assumption that only 10 DL slots exist — now confirmed 12
2. Previous assumption that slot distribution is uniform across world states — highly variable

## Next Minimum Test

1. **Probe a2/t0 register** during scene load to see how slot assignments change
2. **Memory watchpoint on 0x678818 and 0x67EE98** to understand what data they store
3. **Map world_state values to room names** by analyzing init_scene_gobj entity counts
4. **Investigate ws=0x0F's massive dispatch** — what callbacks are firing 304K times?

## Conservative Verdict

This session significantly expanded the runtime understanding:
- **+7 world states** (20 total, up from 13)
- **+2 DL slots** (12 total, up from 10)
- **a2/t0 register mapping** discovered for slot type identification
- **ws=0x0F is the most active gameplay state** (57% of all dispatch events)
- **DL slot clusters** in BSS with two distinct stride patterns

The world state values 0x08-0x0F likely represent the water/aqueduct section of ICO, with heavy backtracking between connected rooms. The whirlpool area (ws=0x0F) is the most computationally active area observed so far.

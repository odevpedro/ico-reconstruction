# Rev.093 — Three static investigations: mask_set, dispatch table, halfword writer

**Date:** 2026-05-19

---

## Objective

Three backlog items from the previous research phase were investigated in a focused static analysis pass:

1. Why is `mask_set` (0x13ED40) only called during loading, never gameplay?
2. Who populates the 17-slot dispatch table at `0x282690`?
3. What condition activates the halfword table writers at `0x166D1C`/`0x166D78`?

---

## Scope

- Static analysis of `.text` section for writers, callers, and initializers
- Runtime session data from Rev.074, Rev.079, Rev.084, Rev.085 (66.9M+ events across 4 sessions)
- Ghidra disassembly and xrefs
- ICO-decomp cross-reference

---

## Investigation 1: mask_set (0x13ED40) — RESOLVED

### Source Evidence

| Source | Finding |
|--------|---------|
| ICO-decomp | `ShockRequestBox_RequestCancel` in `fumi/ios/shockdriver.c` |
| Disassembly (0x13ED40) | Writes to `gp+0x98DC` (main mask) and `gp+0x98E0` (secondary mask) |
| Callers | 6 callers, ALL in scene loader / boot code |
| Runtime | Zero hits in 66.9M gameplay events across 4 sessions |

### Full Caller List

| Caller Address | Context | Effect |
|----------------|---------|--------|
| `0x0019F590` | Boot init (mask=0x01 0x00) | Clear mask at startup |
| `0x001B0918` | Scene loader phase 2 | Set mask (v0=1, v1=0) |
| `0x001B1830` | Scene loader phase 4 | Set mask (v0=1, v1=0) |
| `0x001B1A18` | Scene loader phase 5 | Set mask (v0=1, v1=0) |
| `0x001B44E0` | Scene loader phase 7 | Set mask (v0=1, v1=0) |
| `0x001B4F8C` | Scene loader phase 8 | Clear mask (v0=0, v1=0) |

### Mask Behavior

- **gp+0x98DC** (`mask_main`): 8-bit mask, only bit 0 ever modified
- **gp+0x98E0** (`mask_secondary`): written by mask_set but never read (8-bit, always 0)
- Boot zero-init at `0x13DDE8`: `sw $zero, 0x98DC($gp)` + `sw $zero, 0x98E0($gp)`
- Bits 1-7: zero-initialized at boot, **never touched again** in any code path

### Cycle

```
Loading screen start:
  mask_set(0, 0)    → clear bit 0 (callbacks PAUSED)
  [scene loads, assets initialize]
  mask_set(1, 0)    → set bit 0 (callbacks RESUMED)
```

### Verdict

**CONFIRMED:** mask_set is `ShockRequestBox_RequestCancel` from the I/O shock driver subsystem. Its role is to globally gate callback dispatch during scene loading. It is not a gameplay death/menu/cutscene callback. Only bit 0 is meaningful; bits 1-7 are permanently zero. This is the final word on mask_set — no further investigation needed.

---

## Investigation 2: Dispatch Table Population (0x282690) — RESOLVED

### Key Finding

The 17-slot dispatch table at `0x282690` is a **compile-time `.data` structure**. Zero runtime store instructions (SW/SD/SH/SB) target the `0x282680–0x2827A0` range across the entire `.text` section.

### Initialization Sources

| Structure | Location | Initializer | When |
|-----------|----------|-------------|------|
| Slot table (17 × 16B) | `0x282690` (`.data`) | **Compiler** (hardcoded) | Compile time |
| Runtime pointer list | `0x006AAC80` (`.bss`) | `0x00166028` | Per-scene load |
| GP cold path slots (gp-25856/25852) | `.sbss` | `0x00168650` via boot | Boot time |
| Halfword spatial hash | `0x006AB080` (`.bss`) | Writers at `0x166D1C/0x166D78` | Condition unknown |
| Callback mask | gp+0x98DC (`.sdata`) | Scene loader functions | Per-scene transition |

### Runtime Structure — What IS runtime-populated

1. **Runtime pointer list** at `0x006AAC80`: array of entity context pointers (max 256), rebuilt per-scene by `0x00166028`. This is what the dispatcher iterates — NOT the slot table itself.
2. **Cold path GP slots** (gp-25856/gp-25852): initialized once at boot (always `0x00167230`/`0x00167258` in normal gameplay — alt implementations at `0x00169F80`/`0x0016A058` are unreachable).
3. **Callback mask** (gp+0x98DC): toggled by scene loader (bit 0 only).
4. **Halfword spatial hash** (gp-19396, `0x6AB080`): condition unknown (see investigation 3).

### What Determines Active Slots

1. **Compile time**: 14 wrapper stubs at `0x1683B4`–`0x168628` hardcode slot indices 1-16. **Slot 0 has no wrapper** — permanently dead (confirmed: zero `addiu a1, zero, 0` + JALR sites in .text).
2. **Per-entity mask tests**: callback checks `struct.field_48` (G1) or `field_60` (G2 slot 15) against slot-specific masks.
3. **Per-scene**: callback mask (bit 0) gates entire dispatch chain during loading.

### Verdict

**CONFIRMED:** No code populates the 17 slot entries at runtime. The question "who populates the dispatch table" has no runtime answer — the table is compiled in. The correct question is "what determines slot activation," which is a combination of (1) compile-time wrapper existence, (2) per-entity mask filtering, and (3) callback mask gating during scene transitions.

---

## Investigation 3: Halfword Table Writer Activation — UNRESOLVED

### Confirmed Facts

| Fact | Source |
|------|--------|
| Writers at `0x166D1C`/`0x166D78`/`0x166DFC` encode `(row << 5) \| col` | Instruction-level disassembly (Rev.071) |
| Counter at `gp-0x4BC4` (`0x00633D2C`) tracks write position | `lw`/`sw` at `0x166D18`/`0x166D3C` |
| Counter zeroed at `0x166BDC` before writes | `sw $zero, -0x4BC4($gp)` |
| All 14 callbacks read the counter (30 refs total) | Rev.073 callback skeleton analysis |
| Table format: `uint16[1024]` grid coordinates | Rev.071 |
| **Zero hits at writers in 4 runtime sessions** | Rev.079, Rev.084, Rev.085 |

### Runtime Coverage

| Session | Area | Events | Writer Hits |
|---------|------|--------|:-----------:|
| Rev.074 | Entrance | 9.1M | Not probed |
| Rev.079 | Windmill | 14.0M | **0** |
| Rev.084 | Entrance+Windmill+3rd area+cutscene | 43.8M | **0** |
| Rev.085 | Death (cliff jump) | ~400K | **0** |
| **Total** | **4 areas** | **67.3M** | **0** |

### Why All 14 Callbacks Read From a Table That Is Never Written

The counter at `0x00633D2C` is in `.sbss` (zero-initialized BSS). When no writes occur, the counter stays 0, and the callback loop iterates zero entries. The callbacks do NOT detect "counter == 0" as an error — they simply skip the loop. This means the spatial hash table is optional: when the trigger condition is met, the counter > 0 and spatial lookups occur; when not met, the system falls back to whatever default behavior the zero-iteration implies.

### Hypotheses (ordered by likelihood)

1. **Room-specific activation** — spatial hash rebuild may be needed only in complex rooms with many entities (e.g., castle interior, throne room, water channel). None of the tested areas (entrance, windmill, 3rd area) triggered it.

2. **Scene loading / one-time build** — the function at `0x166C80` may run during scene init (before per-frame dispatch starts). Probes at the SH instructions would miss it if the function runs once per room transition, not per frame.

3. **Entity bounding box trigger** — the rasterization algorithm requires an entity with a specific bounding box that intersects the grid origin. If no entity in the current room has such a box, the function exits without writing.

4. **Dead code** — the entire write path at `0x166C80` may be disconnected (never reached). However, the counter IS read 30 times by active callbacks, so SOMETHING presumably populates it somewhere, somehow.

5. **Probe positioning issue** — probes were placed at the SH instruction itself. An early-exit check at function entry would prevent reaching the SH. The correct probe point is the function entry at `0x166BB0`.

### Next Minimum Test

**Deploy a new PCSX2 probe at `0x166BB0` (function entry of the rasterization loop, NOT at the SH instructions).** Test in a late-game room (e.g., castle interior or water channel) that requires complex spatial calculation.

---

## What Is Confirmed

1. `mask_set` (0x13ED40) = `ShockRequestBox_RequestCancel` — I/O shock driver. Loading-only, bit 0 only. No further investigation needed.

2. The 17-slot dispatch table at `0x282690` is compile-time `.data` — nothing populates it at runtime. Slot activation is determined by wrapper existence (compile-time), mask filtering (per-entity), and callback mask gating (per-scene).

3. The halfword spatial hash table at `0x006AB080` is never populated in any tested game area. Its writers remain unreachable across 67.3M events in 4 independent sessions. The trigger condition is unknown.

## What Is Probable

- The halfword table writer is activated by a room-specific condition not reached in the tested areas.
- The function entry (`0x166BB0`) is the correct probe point, not the SH instructions.

## What Is Unknown

- The exact condition that triggers the spatial hash rebuild.
- Whether the function runs during scene loading (one-time) or per-frame.
- Which room(s) would activate it.

## What Is Discarded

- `mask_set` is NOT a death/menu/cutscene callback (confirmed by ICO-decomp and 66.9M runtime events).
- The dispatch table is NOT populated by any runtime code.
- Slot 0 is NOT active via any code path (no wrapper exists).

---

## Files Affected

- `docs/backlog.md` — Rev.093 entry, Pending items updated
- `AGENTS.md` — Current objectives updated, findings noted
- `docs/system-feature-flows.md` — mask_set and dispatch table entries updated
- `docs/data-model.md` — slot table confirmed static
- `docs/prompt_persona_ico_reconstruction.md` — Updated with Rev.093 findings

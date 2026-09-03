# Rev.115 — thread-path helpers of `_iosOmMain`: `processPoll` / `processStep` (.s byte-exact)

**Date:** 2026-09-03

---

## Objective

Close the last reserved gap of the `_iosOmMain` Pass 2 thread path (Rev.114):
recover `0x13D8A0` and `0x13D928`, the two process helpers called when a
process node has `type_mask (+0x10) == 0`.

## Scope

- Add two byte-exact `.s` to `src/core/asm/`:
  - `processPoll.s`  (0x0013D8A0, 0x3C)
  - `processStep.s` (0x0013D928, 0x18)
- Document the decode and the resulting `_iosOmMain` Pass 2 thread-path
  behavior model.

## Evidence

### `processPoll` (0x0013D8A0 = `iosThreadGetPri` per 2026-05-21 exploration)

```
a0 = process (or 0)
if a0 == 0:
    s0 = 0x6A6F30                    # thread table
    id = 0x00100410()                # current thread id
    a0 = thread_table[id]
return a0->0x18                      # status / priority field
```

- If a process pointer is given, returns its `+0x18` value directly.
- If NULL, resolves the current thread id via `0x00100410` and indexes the
  thread table `0x6A6F30` before reading `+0x18`.

### `processStep` (0x0013D928 = `iosThreadWakeup` per exploration)

```
0x00100450(process->0x30)            # kernel semaphore/thread op on +0x30
```

- A single kernel call on the process `+0x30` field (dereferenced from `a0`),
  with no return-value handling in this wrapper.

### Resulting `_iosOmMain` Pass 2 thread-path model (type_mask == 0)

```
status = processPoll(process)          # +0x18
if status == 0x22:
    isysGObjProcRemove(process)        # 0x13F6B8
else:
    processStep(process)               # 0x13D928
```

This is the poll–step–remove schedule the Rev.098/114 notes described. The
actual kernel operations (`0x00100410`, `0x00100450`, `0x00100470`) live in
the low-level thread layer, not the game core.

## Verification

- `asm_source_score.py processPoll 0x13D8A0 0x3C` → **BYTE-EXACT (100%)**
- `asm_source_score.py processStep 0x13D928 0x18` → **BYTE-EXACT (100%)**
- Files placed in `src/core/asm/` (consistent with `isysGObjProcRemove.s`).

## Confirmed / probable / unknown

- **Confirmed:** byte layout, call targets, field offsets (+0x18 poll read,
  +0x30 step arg), thread table `0x6A6F30`.
- **Probable:** 0x22 is the "done/remove" status; `0x00100410/0x00100450`
  are current-thread-id / semaphore ops.
- **Unknown:** full semantics of `0x00100410/0x00100450/0x00100470`.

## Conservative verdict

The `_iosOmMain` thread path now has byte-exact ground truth for all three of
its helpers (`processPoll`, `processStep`, `isysGObjProcRemove`). The remaining
gap is the kernel-level thread ops themselves, which are out of the game-core
reconstruction scope.

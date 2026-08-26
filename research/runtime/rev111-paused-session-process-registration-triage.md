# Rev.111 — paused-session process-registration triage

## Scope

This note analyzes the paused PCSX2 runtime capture
`ico-runtime-20260825-152452`.  It follows the runtime-first sequence in
`AGENTS.md`: capture, analyze, then select reconstruction targets.  The raw
JSONL remains under `.local/` and is not changed by this analysis.

## Capture integrity

- The capture contains one malformed JSONL record at physical line 1,039,188.
- The original file was retained intact.  Aggregations below skip only that
  record, so this is a read-only recovery measure rather than log repair.
- The malformed line means full-file JSONL validation must report the capture
  as imperfect; it does not invalidate the independently parseable records.

## Confirmed runtime observation

Filtering `isys_gobj_proc_add` events with `regs.a2 == 0x001c34c0` yields
**50,903** records after the malformed record is skipped.

All of those records have:

| Register | Observed value | Meaning supported by byte-exact wrapper |
|---|---|---|
| `ra` | `0x0013f7f8` | Return point of `isysGObjProcAdd_NoCallback` at `0x0013f7d8` |
| `a1` | `0x00000000` | The wrapper supplies a null direct callback argument |
| `a2` | `0x001c34c0` | A non-null process/type entry passed on to the central registrar |
| `a3` | `0x00000000` | Input type/flag value is zero in this registration path |

The `a0` values cover **42 distinct GObj addresses**.  This directly confirms
that `0x001c34c0` is associated with a shared process-registration path, not
with a single gameplay actor.

## Static cross-check

`src/core/asm/isysGObjProcAdd_Wrapper.s` shows that the `0x13f7d8` entry point
sets the central registrar's direct callback argument to zero and forwards the
caller's second argument as its `a2` value.  `isysGObjProcAdd_` then passes
that value to the IOS thread-creation helper at `0x13d1b0` and records it in
the allocated process node.

Instruction-level cross-check: the central registrar stores this `a2` value at TCB `+0x1C`; `process_node_init` receives the same value and stores it at TCB `+0x5C` (its input pointer is TCB `+0x24`). The helper stores its `a3` input at TCB `+0x58`.

Therefore, the runtime probe field named `a2` must **not** by itself be
described as an executed callback.  It is confirmed as a process-registration
input associated with the created thread/process node.  Its exact runtime role
(for example, thread entry versus type descriptor) remains probable, pending
a probe at thread startup/dispatch.

### Existing dispatcher evidence, distinguished from this capture

Earlier byte-level analysis documents a dispatcher at `0x0013FB70` that loads `node + 0x1C` and invokes the resulting pointer with an object/context argument (see `ghidra-rev034-callback-signature-and-record-selection.md` and `ghidra-rev035-entry-table-and-descriptor-correction.md`). This confirms that the `+0x1C` field is a dispatchable callback slot in the general system.

It does **not** establish that the particular value `0x001C34C0` registered in this session reaches that dispatcher. That last association remains the next runtime validation target.

## Byte-exact preservation selected from the capture

The imported Ghidra symbol listing names `0x001c34c0` as
`actClipCollisionCore`; its boundary is the following symbol at `0x001c36e0`.
The following targets have now been extracted and scored from the USA ELF:

| Target | Size | Result | Evidence status |
|---|---:|---|---|
| `actClipCollisionCore` (`0x001c34c0`) | 544 B | 100% byte-exact | Function name from imported Ghidra symbols; registration association confirmed at runtime |
| `CreateClipCollisionManagerGObj` (`0x001c36e0`) | 64 B | 100% byte-exact | Function name from imported Ghidra symbols; static constructor behavior confirmed |

| `process_node_init` (`0x0013d1b0`) | 536 B | 100% byte-exact | IOS thread-creation helper that receives the registration input |

| `RequestClipCollision` (`0x001c3720`) | 64 B | 100% byte-exact | Directly forwards `0x001c34c0` through the collision request path |

The short constructor calls `0x00240ea0` with `t0 = 0x001c3760` and stores the
returned object pointer at `gp-0x564c`.  It does not directly register
`0x001c34c0`.  Existing Rev.076 evidence identifies `0x001c3760` as a
cooperative spin-loop thread start.  This rejects the earlier tempting but
unsupported inference that the constructor directly installs the core routine.

`RequestClipCollision` calls `0x00203b40` with `a0 = 0x001c34c0` and `a1 = 0x15`, clears the caller-provided word at `+0x00`, then stores that caller pointer at `returned_object + 0x20`. This confirms the static collision-request association with `0x001c34c0`; it still does not independently prove dynamic execution of that address.

`process_node_init` is stored under `src/core/asm/`, alongside the existing `isysGObj*` and IOS routines. Its preservation makes the next static step auditable without claiming a semantic C implementation prematurely.

## Next evidence-driven action

Add a narrowly scoped probe or inspect an existing thread-start/dispatch
record to determine how the `0x13d1b0` process field populated from `a2` is
consumed.  Only then can a semantic C++ contract for this collision-process
path be written safely.

# Rev.114 — Scene GObj static-data bridge

**Date:** 2026-08-27
**Scope:** USA ELF static evidence only. No runtime capture or ISO filesystem
access was used.

## Resolved helper: `isysGObjLinkObjDL`

The direct `CreateGObj` call at `0x00240E34` targets `0x0013F130`, reconciled
as `isysGObjLinkObjDL`.

For the exact call from `CreateGObj`:

```text
a0 = newly created GObj
a1 = descriptor + 0x48
a2 = 0
a3 = caller-provided category value
t0 = -1
```

When `descriptor+0x48` is nonzero, the helper:

- stores it at `GObj+0x48`;
- stores incoming `t0` at `GObj+0x50`;
- calls the DL-list linking helper at `0x0013EE60` with the low byte of `a2`
  and incoming `a3`.

This resolves the call as a display-list linkage operation. `descriptor+0x48`
remains an offset-based raw field; its source-level type is not promoted.

## Resolved `descriptor+0x60` contract

`CreateGObj` loads `descriptor+0x60` and passes it as `a0` to
`isysGObjAdd(…, a1=0, a2=0)`. `isysGObjAdd` directly stores its first argument
at `GObj+0x28` before inserting the object.

Therefore the instruction-backed contract is:

```text
descriptor+0x60 -> isysGObjAdd a0 -> GObj+0x28
```

It is **not** a call through that value in this path. Its source-level name and
meaning remain unknown.

## Template-backed object allocation

`0x0019F310` allocates `0x850` bytes, copies the static bytes at `0x002F23F0`,
and returns the new block. In this caller, the return is stored at `GObj+0x15C`.

Only these post-copy offsets are directly established in the analyzed function:

| Offset | Observed access |
|---:|---|
| `+0x08C` | read and tested before an internal call |
| `+0x810` | receives a separately allocated `0x35`-byte buffer |
| `+0x814` | receives `entry+0x30` in `initSceneGObj` |
| `+0x820` | read and tested before an internal call |

These offsets describe the allocated copy, not independently named template
members. No other part of the `0x850` layout is named by this revision.

## Read-only fixture

`tools/export_scene_gobj_fixture.py` exports
`native/tests/fixtures/usa_scene_gobj_tables.json` from the ELF. It includes:

- all 68 descriptor records with only fields used by the reconstructed path;
- all 512 entry records with the raw fields used by that path;
- ELF and template SHA-256 fingerprints plus the template's nonzero-byte count.

The fixture intentionally has no room/world-state association. It is a stable
input for a future loader adapter, not proof that every record is active.

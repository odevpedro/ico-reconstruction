# Rev.112 — initSceneGObj registration gates

**Date:** 2026-08-27
**Scope:** Static evidence only: USA ELF plus byte-exact `initSceneGObj.s`. No
PCSX2 execution, ISO filesystem access, or native-runtime behavior was used.

## Objective

Resolve the two direct callsites from `initSceneGObj` to
`isysGObjProcAdd_Wrapper` (`0x0013F7A8`) without assigning unsupported source
names to the entry-table fields.

## Confirmed instruction flow

`initSceneGObj` obtains its input record as:

```text
entry = 0x002A4C48 + entry_index * 0x4C
descriptor = 0x002A31B8 + u8(entry + 0x46) * 0x64
```

The code at `0x001B7A90..0x001B7AD4` implements exactly one registration
attempt per successfully-created GObj:

```text
t0 = 0x1800
if u16(entry + 0x40) != 0:
    t0 = u16(entry + 0x40) << 10

if u32(entry + 0x24) != 0:
    isysGObjProcAdd_Wrapper(gobj, u32(entry + 0x24), 0, 0x13, t0)
else if u32(descriptor + 0x40) != 0:
    isysGObjProcAdd_Wrapper(gobj, u32(descriptor + 0x40), 0, 0x13, t0)
else:
    no registration through these two callsites
```

Direct evidence:

| VA | Instruction effect |
|---:|---|
| `0x001B7A90` | loads `u16(entry+0x40)` |
| `0x001B7A98` | uses default `t0=0x1800` if the halfword is zero |
| `0x001B7AA0` | loads `u32(entry+0x24)` |
| `0x001B7AA4` | selects fallback only when that word is zero |
| `0x001B7AB0` | calls `0x0013F7A8` with entry override |
| `0x001B7AC0` | loads `u32(descriptor+0x40)` |
| `0x001B7AC4` | skips registration when the fallback is zero |
| `0x001B7ACC` | calls `0x0013F7A8` with descriptor fallback |
| both delay slots | set `a3=0x13` |

The wrapper moves the caller's `t0` into `t1` before tailing into
`isysGObjProcAdd_`; the halfword-derived value is therefore an observed
argument of the central registration path. Its original source-level meaning
remains unknown.

## Related confirmed behavior

- `descriptor+0x58` is independently invoked via `jalr` at `0x001B7A80`, before
  the registration gate.
- The factory at `0x00240D40` is called before the registration gate and returns
  the GObj passed as `a0` to the wrapper.
- The entry selector at `+0x46` is read with `lbu`; all 512 static selectors in
  the USA table are within the 68-entry descriptor range.

## What this resolves

It resolves the precedence and absence behavior for the two `initSceneGObj`
registration callsites: entry override has priority, descriptor fallback is
only used when no override exists, and neither is called when both are zero.

## What remains unknown

- semantic names for `entry+0x24`, `entry+0x40`, and `descriptor+0x40`;
- which entries are selected by a particular scene/world state;
- the full meaning of callback type `0x13` and the `t0/t1` value;
- contracts of the factory and indirect descriptor callbacks.

## Native-port consequence

The portable loader should eventually model these as raw, separately-preserved
fields and implement the observed precedence rule. It should not collapse the
two callback sources into a single `initFn`, nor infer that every static record
is active in every scene.

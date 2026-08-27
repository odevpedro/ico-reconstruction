# Rev.113 — Scene GObj factory contract

**Date:** 2026-08-27
**Scope:** Static only. USA ELF disassembled directly with
`tools/disasm_local_range.py`; byte-exact `initSceneGObj.s` used for caller
context. No PCSX2, ISO filesystem, or host-runtime observations.

## Confirmed creation chain

For the successful `initSceneGObj` path:

```text
entry / descriptor selection
  -> CreateGObj (0x00240D40)
       -> isysGObjAdd (0x0013E8D8)
       -> registrations at types 0x16, 0x17, 0x18 and conditionally 0x13
  -> allocator at 0x0019F310
  -> store returned pointer at GObj+0x15C
  -> store entry+0x30 at returned_object+0x814
  -> descriptor indirect callbacks at +0x58, then later +0x34
```

The diagram is execution order at this caller. It does not prove source-level
names or the semantic purpose of every callback.

## `CreateGObj` (0x00240D40)

### Direct instruction-backed facts

1. Its first argument is used as a base for loads at `+0x40`, `+0x48`, `+0x4C`,
   `+0x50`, `+0x5C`, and `+0x60`.
2. It calls `isysGObjAdd` (`0x0013E8D8`) with:

   ```text
   a0 = word(first_arg + 0x60)
   a1 = 0
   a2 = 0
   ```

   The returned GObj is retained in `s4` and returned to the caller.
3. Before returning, it sets confirmed GObj words `+0x04=1`, `+0x08=-1`,
   `+0x0C=-1`, `+0x164=0`, and `+0x16C=1`.
4. It calls the fixed-`0x1800` wrapper at `0x0013F778` three times, sourcing
   callbacks from first-argument `+0x5C`, `+0x50`, and `+0x4C`, respectively.
   The three callsites set `a3` to `0x16`, `0x17`, and `0x18`.
5. It makes one additional direct call at `0x00240E34` using first-argument
   `+0x48` and caller-provided `a3`; the callee's source-level role is unknown.
6. When its incoming `t0` is nonzero and first-argument `+0x40` is nonzero, it
   registers that `+0x40` word using `0x0013F7A8`, `a2=0`, `a3=0x13`, and
   `t0=0x1800`.

### Conservative interpretation

`0x00240D40` is confirmed as a GObj factory plus process-registration helper.
It is not safe yet to call every loaded word a callback: `+0x60` is passed as
the first argument of `isysGObjAdd`, while `+0x48` goes to an unresolved direct
callee.

## `isysGObjAdd` (0x0013E8D8)

This reaffirms the Rev.109 ABI boundary:

- scans the GObj pool for a slot with `GObj+0x00 == 0`;
- stores its first argument to `GObj+0x28`;
- marks `GObj+0x00` with its own address;
- initializes `+0x164`, `+0x170`, `+0x15C`, `+0x04`, `+0x08`, `+0x2C`, `+0x30`,
  `+0x58`, and `+0x0C`;
- inserts by the low byte of its second argument.

No host pointer is introduced by this fact: all original values remain
`ico_ptr32` at the portability boundary.

## Allocator at `0x0019F310`

### Confirmed facts

- Allocates `0x850` bytes via `0x0013A0F8`.
- Copies `0x850` bytes from a static template beginning at `0x002F23F0`.
- Returns the allocated pointer in `v0`.
- In `initSceneGObj`, that pointer is stored at `GObj+0x15C` and receives
  `entry+0x30` at offset `+0x814`.

The rest of this allocator makes several calls conditional on its arguments and
allocated contents. It is therefore confirmed as a template-backed allocation
step, but its concrete object type and all internal fields remain unknown.

## Descriptor callback ordering at the caller

For the path observed in `initSceneGObj.s`:

1. If descriptor `+0x38` is nonzero, `jalr` calls it with `(stack_transform,
   first_helper_result)` before `CreateGObj`.
2. After factory/allocation/binding, if descriptor `+0x58` is nonzero, `jalr`
   calls it with `(GObj, stack_transform)`; its return is stored through the
   object pointer at `GObj+0x15C`, offset `+0x800`.
3. After the registration gate, if the first helper result exists and descriptor
   `+0x34` is nonzero, `jalr` calls it with `(GObj, first_helper_result)`.

`+0x38`, `+0x58`, and `+0x34` are offset names, not semantic field names.

## Native-port consequence

The portable factory can now be expanded in a later increment as a staged
operation with explicit raw descriptor fields and a host callback registry.
It must not execute PS2 addresses directly, and it must keep the template
allocation and `GObj+0x15C` object relationship separate from the GObj pool.

## Remaining static work

1. Disassemble and constrain the direct helper called at `0x00240E34`.
2. Recover the source/ABI contract of the `+0x60` value passed to
   `isysGObjAdd`.
3. Classify fields in the `0x850` template only where direct accesses establish
   offsets; do not bulk-name the structure.

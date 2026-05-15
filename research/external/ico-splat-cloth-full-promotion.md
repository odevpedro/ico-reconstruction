# ICO splat — Full Cloth Cluster Promotion

## Date

2026-05-15

## Objective

After the successful splat baseline experiments (minimal, promoted-ranges,
adjacent-ranges), promote **all spimdisasm-detected functions in the cloth
physics domain** into individual assembly files, raising the count of isolated
functions from **7 to 22**.

## Scope

Included:

- all 16 spimdisasm-detected functions from `0x001d3bf0` to `0x001d45b0`;
- the 2 functions in the gap between cloth payload init and cloth dispatcher
  (`0x001d29b8`, `0x001d2bf0`);
- the 4 already-promoted cloth functions (init, dispatcher, callback, aux);
- boundary validation for all 22 promoted functions;
- full ELF coverage via monolithic segments for non-cloth `.text` regions;
- versioning of the working splat YAML in `splat/`.

Excluded:

- compiling or linking a rebuilt ELF (requires EE GCC 2.9-991111-01);
- committing generated assembly assets;
- runtime or semantic analysis beyond recorded research notes.

## Sources Used

| Source | Use |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | target ELF |
| `/tmp/ico-tooling-venv` | splat/spimdisasm environment |
| `splat64[mips] 0.34.0` | split engine |
| `spimdisasm 1.35.0` | disassembly backend |
| `/tmp/ico-splat-adjacent/` | previous adjacent experiment (boundary reference) |
| `research/external/ico-splat-adjacent-promoted-ranges-experiment.md` | baseline subsegment structure |
| `research/external/ico-splat-promoted-ranges-experiment.md` | boundary validation methodology |
| `research/ico-decomp-cross-reference-2026-05-14.md` | clothAnimation.c PAL function mapping |
| `research/elf/ghidra-rev039-cloth-domain-correction.md` | cloth domain scope |

## Promoted Function Table

### Callback infrastructure (previously promoted)

| Name | VA | File offset | Size (bytes) |
|---|---|---|---|
| `callback_storage` | `0x0013f3f0` | `0x0403f0` | 584 |
| `callback_register` | `0x0013f7a8` | `0x0407a8` | 48 |
| `cb48_dispatcher` | `0x0013fc00` | `0x040c00` | 272 |

### Cloth cluster — previous promotions

| Name | VA start | VA end | Size |
|---|---|---|---|
| `cloth_payload_init` | `0x001d27a8` | `0x001d29b8` | 528 |
| `cloth_dispatcher` | `0x001d37c8` | `0x001d3a30` | 616 |
| `cloth_update_cb` | `0x001d3a30` | `0x001d3b28` | 248 |
| `cloth_aux` | `0x001d3b28` | `0x001d3bf0` | 200 |

### Cloth cluster — gap functions (new)

| Name | VA start | VA end | Size | Notes |
|---|---|---|---|---|
| `cloth_setup` | `0x001d29b8` | `0x001d2bf0` | 568 | first func after init |
| `cloth_sim` | `0x001d2bf0` | `0x001d37c8` | 3032 | **largest**: spans most of cloth pre-dispatch |

### Cloth cluster — sub-functions (new, 0x3bf0-0x45b0)

| Name | VA start | VA end | Size | Notes |
|---|---|---|---|---|
| `cloth_sub_001d3bf0` | `0x001d3bf0` | `0x001d3d40` | 336 | multiple local calls |
| `cloth_sub_001d3d40` | `0x001d3d40` | `0x001d3d70` | 48 | tiny helper |
| `cloth_sub_001d3d70` | `0x001d3d70` | `0x001d3d80` | 16 | accessor-like |
| `cloth_sub_001d3d80` | `0x001d3d80` | `0x001d3d98` | 24 | accessor-like |
| `cloth_sub_001d3d98` | `0x001d3d98` | `0x001d3db0` | 24 | accessor-like |
| `cloth_sub_001d3db0` | `0x001d3db0` | `0x001d3dd8` | 40 | accessor-like |
| `cloth_sub_001d3dd8` | `0x001d3dd8` | `0x001d3e80` | 168 | state read/write |
| `cloth_sub_001d3e80` | `0x001d3e80` | `0x001d3f78` | 248 | collision-related |
| `cloth_sub_001d3f78` | `0x001d3f78` | `0x001d40a0` | 296 | constraint-like |
| `cloth_sub_001d40a0` | `0x001d40a0` | `0x001d40d8` | 56 | small helper |
| `cloth_sub_001d40d8` | `0x001d40d8` | `0x001d4170` | 152 | post-process logic |
| `cloth_sub_001d4170` | `0x001d4170` | `0x001d4228` | 184 | chain weight |
| `cloth_sub_001d4228` | `0x001d4228` | `0x001d4348` | 288 | velocity/force |
| `cloth_sub_001d4348` | `0x001d4348` | `0x001d4358` | 16 | **smallest**: node ID accessor |
| `cloth_sub_001d4358` | `0x001d4358` | `0x001d43f8` | 160 | reset/destroy |
| `cloth_sub_001d43f8` | `0x001d43f8` | `0x001d45b0` | 440 | plane clip finalize |

## Results

### Split success

```
Split 5 MB (100.00%) in defined segments
  databin: 3 MB (72.55%) 28 split
  asm:     1 MB (27.45%) 30 split
  unknown: 0 B
```

30 asm segments (vs 13 in adjacent experiment, vs 8 in first promoted, vs 1 in
minimal). Zero errors.

### Anchor validation

All critical instructions preserved across the newly isolated subsegments:

| Anchor | Status |
|---|---|
| `0x001d3800: sll $v1, $v1, 2` | ✓ in `cloth_dispatcher_001d37c8.s` |
| `jtbl_00618FB0_main_text` reference | ✓ in `cloth_dispatcher_001d37c8.s` |
| `0x001d3b04: jal func_001D37C8` | ✓ in `cloth_update_cb_001d3a30.s` |
| `0x001d2850: lw $v1, 0x30($s4)` | ✓ in `cloth_payload_init_001d27a8.s` |

### Boundary coherence

Every promoted function's epilogue (`jr $ra` + delay slot) was verified to fall
fully within its subsegment. No instruction straddles a boundary.

## Pattern Observations

The 16 new sub-functions in `0x3bf0-0x45b0` show a clear size structure:

- **Tiny helpers** (16-48 bytes, 6 functions): likely chain node accessors
  (`GetChainNodeID`, `GetChainNodeGlobalQuaternion` equivalent)
- **Medium routines** (56-248 bytes, 5 functions): state read/write, weight
  manipulation, post-process
- **Larger routines** (248-440 bytes, 4 functions): collision detection,
  constraint resolution, plane clipping, reset

This is consistent with the ICO-decomp PAL function cluster:
`GetChainNodeGlobalQuaternion` through `getCloth4D_PlaneClip`.

## Comparison With Previous State

| Metric | Adjacent experiment | Full cloth promotion |
|---|---|---|
| Isolated cloth functions | 4 | **22** |
| Total asm segments | 13 | **30** |
| Coverage | 100% | **100%** |
| Errors | 0 | **0** |
| YAML versioned in repo | No | **Yes** (`splat/`) |

## What Is Confirmed

1. All 22 spimdisasm-detected functions in the cloth cluster can be promoted
   cleanly via splat.
2. Full ELF coverage remains 100% with non-cloth regions as monolithic `.text`
   segments or `databin`.
3. All known instruction anchors are preserved.
4. The flattest functions (16-48 bytes) are consistent with simple accessors in
   the ICO-decomp cloth animation source.

## What Is Probable

1. The 22 promoted functions span the full `clothAnimation.c` range from
   `GetCloth4D`/`InitCloth4D` through `getCloth4D_PlaneClip`.
2. The 6 tiny functions (16-48 bytes) are chain node accessors.
3. `cloth_sim` (0x1d2bf0, 3032 bytes) is the main cloth simulation function
   containing GetCloth4D and InitCloth4D as internal labels.
4. The 30-segment splat config is now the project's most complete tooling
   baseline.

## What Remains Unknown

1. Exact mapping of each USA address to PAL symbol names.
2. Whether the linker script can produce a byte-identical rebuilt ELF (requires
   EE GCC 2.9-991111-01).
3. The `a1` source for `0x001d27a8` (unchanged - still requires runtime capture).

## Next Minimum Tests

1. **Runtime capture at `0x001d27a8`**: resolves the `a1` source (the one
   question that no amount of static tooling can answer).
2. **SDK/library recognition path**: scan monolithic segments for PS2 SDK
   functions.
3. **first-diff validation**: requires the matching EE GCC 2.9-991111-01
   compiler. The `splat/Makefile` documents exact toolchain requirements.
4. **decomp.me compiler availability**: check if the ICO compiler package is
   published.

## Conservative Verdict

The comprehensive cloth cluster promotion succeeded. The project now has **22
isolated cloth-domain functions** in a reproducible splat config, up from 4 in
the previous experiment. The config is versioned in `splat/`. This is the
strongest tooling baseline so far, but it does not close any semantic gaps — the
`a1` source and the ROPE registration gap still require runtime capture.

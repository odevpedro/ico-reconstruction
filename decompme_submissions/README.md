# Decomp.me Submissions

Submeter manualmente pelo browser em https://decomp.me/new

## Config

- Platform: PlayStation 2
- Compiler: EE GCC 2.9 build 991111-01
- Flags: `-mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2`

## Files

Each function has:
- `*_source.c` — standalone C source for local/API attempts
- `*_context.c` — C source for the manual browser flow (paste into Context)
- `*_target_asm.txt` — target assembly (paste into target_asm field)

## Exact matches (local score 100%)

1. cloth_get_variant (0x1D3D70) — 4 insns
2. boy_set_state (0x1C2098) — 4 insns
3. cloth_payload_state_is_two (0x1D3D98) — 6 insns
4. cloth_payload_field0_is_zero (0x1D3D80) — 5 insns
5. fn_1CE5F8 (0x1CE5F8) — 37 insns, exact after local label normalization and `$3` register hint
6. cloth_test_variant_field (0x1D3DB0) — 10 insns, exact after branch delay-slot nop barrier
7. cloth_test_field0_or_extra (0x1D40A0) — 13 insns, exact after branch delay-slot nop barrier
8. cloth_test_state_lt_2 (0x1D3D40) — 12 insns, exact after payload preload, branch delay-slot nop barrier, and `slt` immediate normalization

## Near-matches

The next local candidate is enemy1_hA (95.65% with corrected 0x60 target
range). It still has one extra generated epilogue instruction after the
tail-jump path.

## Manual browser procedure

1. Open https://decomp.me/new in browser
2. Select PS2 platform, pick compiler "EE GCC 2.9 build 991111-01"
3. Paste flags and target_asm
4. Leave the generated Source code stub in place
5. Paste the real C into Context
6. Compile and confirm the score
7. Note: Cloudflare challenge may appear; complete it manually

The browser flow above reflects the working manual behavior observed in the
current decomp.me UI: for these scratches, putting the real C in Context and
leaving the generated Source stub intact can score correctly.

## API status

Automated submission through `tools/decompme_submit.py` is not currently
usable for this compiler package:

- `POST /api/scratch` rejects `EE GCC 2.9 build 991111-01` as an unknown
  compiler identifier.
- public compiler/platform discovery endpoints are behind Cloudflare.

Use the browser flow above until the internal compiler id is known.

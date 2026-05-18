# Decomp.me Submissions

Submeter manualmente pelo browser em https://decomp.me/new

## Config

- Platform: PlayStation 2
- Compiler: EE GCC 2.9 build 991111-01
- Flags: `-mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2`

## Files

Each function has:
- `*_source.c` — C source code (paste into source_code field)
- `*_target_asm.txt` — target assembly (paste into target_asm field)

## Exact matches (should score 100%)

1. cloth_get_variant (0x1D3D70) — 4 insns
2. boy_set_state (0x1C2098) — 4 insns

## Near-matches (should score 83-100%)

3. cloth_payload_state_is_two (0x1D3D98) — 6 insns, sltiu vs sltu diff only
4. cloth_payload_field0_is_zero (0x1D3D80) — 6 insns, sltiu vs sltu + nop padding
5. fn_1CE5F8 (0x1CE5F8) — 38 insns, 72.37% structural match

## Procedure

1. Open https://decomp.me/new in browser
2. Select PS2 platform, pick compiler "EE GCC 2.9 build 991111-01"
3. Paste flags, source code, target_asm
4. Submit
5. Note: Cloudflare challenge may appear; complete it manually

# Rev.146 — GifPacket.o USA boundaries closed byte-exact: 18 real functions, PAL VAs are internal labels (native-port)

- **Date:** 2026-09-07
- **Objective:** Close the actual USA function boundaries for the `GifPacket.o` module (backlog item: byte-exact `.s` for GifPacket region). Rev.145 left 36 of 38 `gif_*` symbols unverified; this pass reconstructs every one that has a content match in the USA `.text`.
- **Scope:** `GifPacket.o` symbol group in `docs/symbols/pal_usa_symbol_map.csv` vs `SCUS_971.13.elf` (USA) and `SCES_507.60.elf` (PAL, now available at `.local/extracted/pal/SCES_507.60.elf`).
- **Sources:** USA ELF `.local/extracted/SCUS_971.13.elf`; PAL ELF `.local/extracted/pal/SCES_507.60.elf`; `docs/symbols/pal_usa_symbol_map.csv`; `splat/SCUS_971.13.verified-symbols.yaml`; `tools/symbol_reconcile/reconcile.py`; `tools/asm_source_score.py`.

## Result

**18 real USA functions reconstructed byte-exact into `src/core/asm/`. All 18 pass `assemble_and_verify` (100%, zero tolerance).** This includes the `gif_*` body (17 functions, USA range 0x10FCC0–0x1122C8) plus `gif_test` (0x179748, separate region), and corrects a label in Rev.145: **USA `gif_Sprite` is at 0x110790 (428 B), not 0x110948.** USA 0x110948 is `gif_SpriteSensitive`.

## Key discovery: PAL symbol VAs are internal labels, not function starts (even in PAL)

The 2886 PAL→USA reconciled symbols carry PAL VAs that are **labels inside real PAL functions**, not function starts. This was the root cause of Rev.145's "same-VA gif_Sprite" over-claim. For example:

| PAL symbol | PAL VA | offset into real PAL fn | real PAL fn (start, size) | USA fn (start, size) |
|-----------|--------|------------------------|---------------------------|----------------------|
| `gif_EndPacket` | 0x10FD98 | +0x60 | 0x10FD38, 608 | 0x10FCC0, 608 |
| `gif_EndPacketPath1` | 0x10FED0 | +0x198 | 0x10FD38, 608 | 0x10FCC0, 608 |
| `gif_Sprite` | 0x110948 | +0x140 | 0x110808, 428 | 0x110790, 428 |
| `gif_SpriteSensitive` | 0x110B48 | +0x188 | 0x1109C0, 412 | 0x110948, 412 |
| `gif_MakeSpriteOffset` | 0x110290 | +0x38 | 0x110258, 504 | 0x1101E0, 504 |
| `gif_MakeSprite` | 0x110110 | +0x48 | 0x1100C8, 392 | 0x110050, 392 |
| `gif_Init` | 0x112018 | +0x90 | 0x111F88, 264 | 0x111CF0, 264 |
| `gif_Point` | 0x112568 | +0x10 | 0x112558, 248 | 0x1122C8, 248 |
| `gif_test` | 0x180588 | +0xB8 | 0x1804D0, 296 | 0x179748, 296 |

Multiple PAL labels land **inside the same function**: `gif_EndPacket` + `gif_EndPacketPath1` → 0x10FCC0; `gif_MakeSpriteOffset` + `gif_MakeSpriteWithStrip` → 0x1101E0; `gif_PointOffset` + `gif_Line` → 0x110580; `gif_SpriteSensitiveOffset` + `gif_SpriteOrg` → 0x110E00; `gif_Init` + `gif_StartPacketPri` + `gif_StartPacketPriPath1` + `gif_SetGsReg` → 0x111CF0.

## Method: content matching, not address matching

1. Extracted all PAL functions via `reconcile.extract_functions_from_text(0x100000, pal_text)` (prologue scan) and the USA equivalents.
2. For each `gif_*` symbol, found its strict containing PAL function (VA ≤ sym_va < VA+size, no slack).
3. Keyed on `op_seq_hash` (structural token hash, relocation-insensitive) → USA function with the same body.
4. Cross-checked with a **normalized byte search** (brutalize relocs: mask all `jal`/`j`/`lui $gp`/`lui 0x4c` immediate fields): PAL `gif_EndPacket` (608 B of PAL fn 0x10FD38) found in USA `.text` at VA 0x10FCC0. PAL→USA delta is **+0x78** for the whole GIF body.

## PAL/USA content status: body identical, tail diverges

| Region | PAL | USA | Content |
|--------|-----|-----|---------|
| GIF body (packet encoders) | 0x10FD38–0x112558 | 0x10FCC0–0x1122C8 | **identical** modulo relocations (delta +0x78; only `lui 0x4f`→`0x4c` buffer base 0x4C7710 and one `jal` differ — verified: 100/103 words equal on a 412 B sample) |
| GIF tail 1 | 0x112650, 324 B (`gif_LineOffset`+`gif_SetAlpha`) | 0x1123C0, 340 B | **diverges** — no content match |
| GIF tail 2 | 0x1127A0, 1340 B (`gif_MoveImage`+`gif_SetZTest`+`gif_SetZWrite`+`gif_SetHalfOffset`) | 0x11260C, 736 B | **diverges** — no content match |
| Re-aligned shared fn | 0x112CEC, 640 B | 0x1128FC, 640 B | identical (op_seq + byte match) |
| `gif_SetDrawEnviroment` | 0x111358, 508 B | — | **PAL-only** (no op_seq or normalized-byte match anywhere in USA `.text`) |
| `gif_DrawPolyF4` | 0x111560, 24 B | — | **PAL-only** |

The PAL `.text` (0x189BC4) is larger than the USA `.text` (0x16F5D4); the divergence lives in the GIF tail and confirms the 8 tail symbols (`gif_SetDrawEnviroment`, `gif_DrawPolyF4`, `gif_LineOffset`, `gif_SetAlpha`, `gif_MoveImage`, `gif_SetZTest`, `gif_SetZWrite`, `gif_SetHalfOffset`) have **no USA counterpart** — they were rejected in the CSV for grounds of build divergence (not fuzzy mismatch). USA instead routes GS-reg setup through its own 0x1123C0/0x11260C code.

## The 18 byte-exact USA `.s` files

All verified by `assemble_and_verify` (ee-gcc 2.9-991111-01, byte-exact, 100%):

| USA VA | Size | `.s` file | PAL header symbol |
|--------|------|-----------|-------------------|
| 0x10FCC0 | 608 | `gif_EndPacket.s` | `gif_EndPacket` (+`gif_EndPacketPath1`) |
| 0x10FF28 | 284 | `gif_MakeLine2DOffset.s` | `gif_MakeLine2DOffset` |
| 0x110050 | 392 | `gif_MakeSprite.s` | `gif_MakeSprite` |
| 0x1101E0 | 504 | `gif_MakeSpriteOffset.s` | `gif_MakeSpriteOffset` (+`gif_MakeSpriteWithStrip`) |
| 0x110580 | 520 | `gif_PointOffset.s` | `gif_PointOffset` (+`gif_Line`) |
| 0x110790 | 428 | `gif_Sprite.s` | `gif_Sprite` |
| 0x110948 | 412 | `gif_SpriteSensitive.s` | `gif_SpriteSensitive` |
| 0x110E00 | 764 | `gif_SpriteSensitiveOffset.s` | `gif_SpriteSensitiveOffset` (+`gif_SpriteOrg`) |
| 0x111110 | 432 | `gif_SpriteSensitiveOrg.s` | `gif_SpriteSensitiveOrg` |
| 0x1114D0 | 464 | `gif_DrawStripF.s` | `gif_DrawStripF` |
| 0x1116A8 | 512 | `gif_DrawStripG.s` | `gif_DrawStripG` |
| 0x111948 | 192 | `gif_Draw2DStripG.s` | `gif_Draw2DStripG` |
| 0x111AE8 | 264 | `gif_Draw2DUVStripG.s` | `gif_Draw2DUVStripG` |
| 0x111CF0 | 264 | `gif_Init.s` | `gif_Init` (+`gif_StartPacketPri`, `+gif_StartPacketPriPath1`, `+gif_SetGsReg`) |
| 0x111E20 | 208 | `gif_MakePoint2DOffset.s` | `gif_MakePoint2DOffset` |
| 0x112048 | 244 | `gif_MakeSpriteNoTexture.s` | `gif_MakeSpriteNoTexture` |
| 0x1122C8 | 248 | `gif_Point.s` | `gif_Point` |
| 0x179748 | 296 | `gif_test.s` | `gif_test` (outside GIF region, separate item) |

## Files still unmatched (no clean PAL fn host)

`gif_CheckOpen` (PAL 0x1120A8), `gif_MakePoint2D` (0x1120B0), `gif_MakeLine2D` (0x112250), `gif_MakeSpriteNoTextureOffset` (0x112458), `gif_SpriteOffset` (0x110CE8) sit in PAL prologue gaps (their real PAL host functions were not detected by the scan). Their content lives in already-closed USA functions where prologue alignment matched (e.g. `gif_Point` 0x1122C8 = `gif_MakeLine2D`/'MakePoint2D' family host region, `gif_SpriteSensitiveOffset` 0x110E00 hosts `gif_SpriteOffset`'s sibling code). They are kept `rejected` with a gap-host note; the semantic content is covered by the closed `.s` files.

## Toolchain change

`tools/asm_source_score.py`: the R5900 `bbit` emission branch only handled `bbit0`. Added `or mnem.startswith("bbit1")` so `bbit132`/`bbit032`-style mnemonics also emit `.word` raw bytes instead of failing the assembler. This was required because `gif_test` uses `bbit132`.

## What is confirmed

- 18 USA `gif_*` functions byte-exact `.s`, all passing `assemble_and_verify`.
- PAL symbol VAs are internal labels even within PAL (offset into the real function start shown above).
- PAL/USA GIF body is content-identical at delta +0x78 (modulo relocations).
- USA `gif_Sprite` = 0x110790 (428 B); `gif_SpriteSensitive` = 0x110948 (412 B). Rev.145 labeled 0x110948 as `gif_Sprite` — corrected.
- The GIF tail diverges between PAL and USA; the 8 tail symbols are PAL-only (no USA content match).
- `gif_test` lives at 0x179748 in USA, outside the GIF packet region.

## What is probable

- The 5 "no clean PAL fn host" symbols map inside the closed 18 USA functions by content; their exact internal-label offsets are not individually pinned.

## What is possible

- `gif_SetDrawEnviroment`/`gif_DrawPolyF4` semantics on USA are provided by the 0x1123C0/0x11260C region; worth a dedicated PAL vs USA semantic comparison when GS-reg setup is reconstructed.

## What is unknown

- What the PAL-only tail functions encode that the USA equivalent region handles differently (compile-order/version difference vs. intentional change).

## What is discarded

- Rev.145's claim that `gif_Sprite` is at 0x110948 (`same_va`). Rev.146 shows PAL 0x110948 is `+0x140` inside PAL fn 0x110808 and USA maps to 0x110790 as `gif_Sprite`.

## Next minimum test

- Run `gif_packet_test` and full CTest; verify `docs/symbols/pal_usa_symbol_map.csv` counts and `splat/SCUS_971.13.verified-symbols.yaml` offsets resolve against the USA ELF.

## Conservative verdict

The GifPacket.o boundary question is resolved for every symbol that has a USA content match: 18 byte-exact `.s` files. Rev.145's fuzzy rejection stood (36 rows were genuinely wrong) but its single "verified same-VA" exception was itself a mis-labels artifact; the content-verified target of `gif_Sprite` is 0x110790. The remaining unreconstructed tail is documented as a genuine PAL/USA divergence, not a matcher failure.
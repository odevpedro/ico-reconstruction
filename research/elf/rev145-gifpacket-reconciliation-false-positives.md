# Rev.145 — GifPacket.o reconciliation false positives: only `gif_Sprite` verified on USA ELF (native-port)

- **Date:** 2026-09-07
- **Objective:** Determine what the native GifPacket bridge milestone (backlog #42) actually lacks, per verify-before-reconstruct. Answer: the bridge already covers all 38 GifPacket.o names and is wired; the real finding is a **reconciliation false positive** in `docs/symbols/pal_usa_symbol_map.csv`.
- **Scope:** `GifPacket.o` symbol group (PAL range 0x10FD40–0x112A30, 38 functions) vs `SCUS_971.13.elf` (USA). Bridge surface in `native/src/engine/GifPacket.{h,cpp}`.
- **Sources:** USA ELF `.local/extracted/SCUS_971.13.elf`; `docs/symbols/pal_usa_symbol_map.csv`; `splat/SCUS_971.13.verified-symbols.yaml`; `research/pal-usa/call_signature_candidates.csv`; `GifPacket.cpp` disassembly.
- **Evidence:** byte-level disassembly with capstone; prologue scan over the GifPacket region; verification statuses in the symbol map.

## Finding

**Of the 38 `GifPacket.o` symbols in `pal_usa_symbol_map.csv`, only `gif_Sprite` (PAL 0x00110948 → USA 0x00110948, `same_va`, confidence 0.85, verified, USA size=412) is real on the USA ELF. The other 36 "candidate" fuzzy rows and 1 unmatched row are unusable: 36 then point at USA addresses of unrelated functions.**

The native `GifPacketBridge` (38 method names matching the recovered GifPacket.o API) is a **semantic** bridge — it is not derived from byte-exact USA code and must not be presented as such.

## Table of the 36 rejected candidates

All 36 were fuzzy-matched against unrelated USA functions. Representative rows:

| PAL symbol | PAL VA | wrong USA VA | confidence | why wrong |
|-----------|--------|--------------|-----------|-----------|
| `gif_EndPacket` | 0x0010FD98 | **0x0013F9D0** | 0.90 | 0x13F9D0 = `_iosOmMain` (verified Rev.098, 17-slot dispatcher). Same bogus VA was assigned to `SubVectorXYZ` and `gsb_SetVSMatrix` too. |
| `gif_EndPacketPath1` | 0x0010FED0 | 0x00163B40 | 1.00 | unrelated |
| `gif_MakeSprite` | 0x00110110 | 0x0014EB28 | 0.95 | unrelated |
| `gif_MakeSpriteOffset` | 0x00110290 | 0x001315A8 | 0.73 | unrelated |
| `gif_MakeSpriteWithStrip` | 0x00110428 | **0x001D0F68** | 0.93 | in cloth/physics range, unrelated |
| `gif_PointOffset` | 0x00110690 | 0x00142890 | 0.88 | unrelated |
| `gif_Line` | 0x001107B8 | 0x001AE8F0 | 0.90 | unrelated |
| `gif_SpriteSensitive` | 0x00110B48 | 0x001684E8 | 1.00 | unrelated |
| `gif_SpriteOrg` | 0x001110B0 | 0x00140048 | 0.98 | unrelated |
| `gif_SetDrawEnviroment` | 0x001113A0 | 0x0010B5A8 | 1.00 | unrelated |
| `gif_DrawPolyF4` | 0x00111568 | 0x001681A8 | 0.98 | unrelated |
| `gif_DrawStripG` | 0x00111A30 | 0x00104240 | 1.00 | unrelated |
| `gif_MoveImage` | 0x001127B0 | 0x0010CF18 | 0.90 | unrelated |
| `gif_SetHalfOffset` | 0x00112948 | 0x0010B440 | 0.82 | unrelated |

`gif_Point` (PAL 0x00112568) is `unmatched` (no USA VA) — kept as unmatched, not upgraded.

## The GifPacket.o region DOES exist in USA — at the same VAs

The fuzzy rows are wrong *targets*, not wrong *regions:* the module lives at the same VA offset in USA. Evidence:

1. `gif_Sprite` = 0x00110948 verified same-VA (USA size 412, `0x19C`). Prologue scan finds `addiu $sp,$sp,-0x40` (`0x27BDFFC0`) at 0x00110948 — exact boundary hit.
2. Disassembly at other claimed PAL VAs shows genuine GIF-primitive encoding into the same packet buffer, e.g.:
   - 0x001113A0 (`gif_SetDrawEnviroment`) and 0x00111568 (`gif_DrawPolyF4`): compute RGBAQ/XYZ from a color table and append to buffer at `0x4C7710` (`lui $t?,0x4c` / `addiu $t?, $t?, 0x7710`) via the common packet-pointer slot (+0x10), with `c.ole.s`/`bltzl` screen checks.
   - 0x00110CE8 (`gif_SpriteOffset`): emits the A+D tag quad with half-offset (`ori 0x8000`), TFX `0xFE00`/UV layout identical to `gif_Sprite`'s verified code.
3. 12 of the claimed VAs contain the `0x4c` buffer-base `lui` hint within the first 0x80 bytes; 21 contain a `jr $ra` within 0x100 bytes.

So **GifPacket.o is present in USA at essentially the PAL addresses**, but the per-function Palo→USA fuzzy matcher assigned 36 of them to wrong output addresses, and the `call_signature_candidates.csv` note already warned these rows were "candidate evidence only." `splat/SCUS_971.13.verified-symbols.yaml` contains only the single verified `gif_Sprite` (offset 0x11948), which is consistent.

## Root cause of the false positives (probable)

| Factor | Weight | Evidence |
|--------|--------|----------|
| Fuzzy op_hash matching is collision-prone for short/uniform GIF-thunk bodies | high | 3 different PAL symbols → same USA 0x13F9D0 (`SubVectorXYZ`, `gif_EndPacket`, `gsb_SetVSMatrix`) |
| `sz_diff` non-zero on 10+ rows (up to 44) tolerated instead of rejected | medium | rows carry `sz_diff=24`, `sz_diff=8`, … yet stayed `candidate` |
| No cross-validation against verified labels | medium | the conflicting target (0x13F9D0) already had verified names; the matcher did not discard |
| Result rows advertise confidence 0.73–1.00 despite being wrong | medium | confidence reflects only fingerprint similarity |

## What is confirmed

- Only `gif_Sprite` (0x00110948) is byte-verified same-VA on USA.
- The other 36 `GifPacket` rows in `pal_usa_symbol_map.csv` are rejected as false positives; `gif_Point` remains unmatched.
- The GifPacket.o module exists in USA in the same ~0x10FD40–0x112A30 range, with genuine GIF packet encoding (buffer 0x4C7710).
- The native `GifPacketBridge` is a semantic API-surface bridge (name-based), wired into `main.cpp` and `KanbanSceneLoader`, with 9 tests in `native/tests/gif_packet_test.cpp` — sufficient for the native render milestone; it is not byte-exact and is not claimed as such.

## What is probable

- The 37 non-verified GifPacket functions in USA live at (or very near) the same VAs as their PAL symbol addresses, but **individual USA boundaries were not re-verified** beyond `gif_Sprite`. A per-function prologue/boundary pass (like the Rev.116f target method) is the next reliable step if USA byte-exact `.s` of GifPacket.o is ever needed.

## What is possible

- The fuzzy matcher may also produce false positives for other small uniform modules (e.g. `GsBase.o`, `girl_act.o`, `end.o` were the other zero-callee candidates). A sweep of `status=candidate` rows with `sz_diff>0` + conflicting verified targets is warranted before trusting them in downstream work.

## What is unknown

- Exact USA boundary/size for 36 of the GifPacket functions.
- Whether the PAL/USA GifPacket.o code is byte-identical modulo relocations (only `gif_Sprite` was compared).

## Action taken

1. `docs/symbols/pal_usa_symbol_map.csv`: 36 `gif_*` rows flipped `candidate` → `rejected` with a Rev.145 note; `gif_Sprite` kept `verified`; `gif_Point` kept `unmatched`. No other module was touched.
2. This note.

## Next minimum test

- For any future use of a GifPacket symbol on USA: verify with `assemble_and_verify` (prologue + size + tail) before reconstructing `.s` or claiming byte-exact status.

## Conservative verdict

The GifPacket symbol map rows were false positives of the fuzzy reconciler, not evidence-free speculation. The correction is conservative: it downgrades 36 rows to `rejected` based on byte-level disassembly and the verified `_iosOmMain` label, and keeps the single verified row. The native bridge milestone doesn't require fixing the boundary of the other 36 — that is only needed if byte-exact `.s` of GifPacket.o becomes a target.
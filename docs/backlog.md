# Backlog — ICO Reconstruction

> Current project state and pending work. Updated in real-time during development.
> See `docs/architecture-log.md` for historical record of implemented features.

---

## Status Summary

| Category | Count |
|----------|-------|
| Completed | 141 |
| In Progress | 1 |
| Pending | 4 |

---

## In Progress

### [ ] [SQUAD-RUNTIME | rev.095 | Pending]
**Directly probe halfword fast path and second caller state**

- Rev.093b confirmed `0x00166BB0` is active in the hot dispatch path before callback dispatch.
- Rev.094 confirmed the second direct caller at `0x0016828C` is runtime-reachable (`ra=0x00168294`, 14,257 hits).
- Add direct probe at `0x00166DFC` to count the inferred high-volume single-cell fast path.

---

## Pending

### [ ] [SQUAD-RUNTIME | rev.095 | Pending]
**Directly probe halfword fast path and second caller state**

- Rev.093b confirmed `0x00166BB0` is active in the hot dispatch path before callback dispatch.
- Rev.094 confirmed the second direct caller at `0x0016828C` is runtime-reachable (`ra=0x00168294`, 14,257 hits).
- Add direct probe at `0x00166DFC` to count the inferred high-volume single-cell fast path.
- Add caller-side probes at `0x0016828C` and `0x00168294`, plus main return probe at `0x00167014`.
- Log `world_state_raw` with second-caller events so activation can be tied to state transitions.
- See `research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md` and `research/elf/ghidra-rev094-halfword-runtime-second-caller.md`.

### [ ] [SQUAD-RUNTIME | rev.080 | Superseded by Rev.093]
**Resolved: mask_set, dispatch table — superseded**
- mask_set (0x13ED40) = ShockRequestBox_RequestCancel (I/O shock driver). Loading-only, bit 0 only. CONFIRMED.
- Dispatch table (0x282690) = compile-time `.data`. No runtime populator. CONFIRMED.
- Slot 0 root cause: zero wrapper stubs for slot 0 (addiu a1, zero, 0 + JALR).
- gp-0x49B4, gp-0x6F60, halfword writer entry — resolved by Rev.089/Rev.093b/Rev.094; fast path remains moved to rev.095.
- Ver `research/elf/ghidra-rev093-three-investigations.md`

### [x] [SQUAD-EXTERNAL | rev.060 | Superseded by Path B]
**Correção: verificar ee-gcc 2.9-991111-01 no decomp.me via presets de jogo** — SUPERSEDED

- Path B (Rev.092) converteu 26 funções LOW para `.s` assembly, **bypassando o compilador C completamente**
- O montador local (GAS do ps2dev GCC 15.2.0) produz bytes exatos sem depender do ee-gcc
- Compilador local ee-gcc 2.9-991111-01 obtido do PS2 Linux SDK, mesma fonte do decomp.me
- Esta task não é mais relevante — manter apenas como referência histórica

### [SQUAD-ARCH | rev.012 | Pending]
**Architectural Analysis E-G for ICO**

- Decision matrix for technical approach
- Squad architecture definition
- Final recommendation
- Black box identification

---

### [SQUAD-RUNTIME | Pending | Pending]
**Asset Format and Runtime Validation Expansion**

- Investigate `.gcm` file references and storage format
- Locate UI text or texture-backed text sources
- Analyze file-loading and save/load code paths
- Test numeric/value patches only on local modified BINs
- Deliverable: reproducible local-only notes and tooling that do not distribute proprietary data

---

### [SQUAD-TOOLING | Pending | Pending]
**PCSX2 Generic Runtime Logpoints Proposal**

- Keep current ICO-specific PCSX2 probe as local POC only
- Stabilize the POC across at least one more focused runtime session
- Separate generic debugger/logpoint behavior from ICO-specific target addresses
- Evaluate an upstream-friendly design for PCSX2: configurable addresses, register filters, structured logs, optional memory windows, and no mandatory pause
- Open PCSX2 issue/discussion before any PR if the design remains useful after stabilization
- Deliverable: short technical proposal that does not include game data or project-specific hardcoded addresses

---

## Concluídas / Completed

### [x] [SQUAD-EXTERNAL | symbol-reconcile | 2026-05-20]
**Pipeline PAL→USA de reconciliação de símbolos — concluída**

- Pipeline completa: `tools/symbol_reconcile/reconcile.py` — parseia `symbol_addrs.txt`, extrai fingerprints PAL/USA, cruza por hash e VA
- 5 níveis de matching: raw_sha1 (exato), op_seq_hash (estrutural), norm_sha1 (normalizado), same_va (mesmo endereço), fuzzy (re-rankeado por op_seq_hash + tamanho)
- **Resultado**: 3781 verified (+95 same_va), 1766 candidates, 134 unmatched — **140 símbolos a mais resolvidos** vs pipeline anterior
- Correções implementadas: op_hash vs op_seq_hash (era duplicata), fallback same-VA para símbolos de kernel, re-ranking fuzzy com prioridade estrutural, suporte a .symtab (quando disponível)
- Splat YAML gerado: `splat/SCUS_971.13.verified-symbols.yaml` (2886 símbolos)

### [x] [SQUAD-EXTERNAL | ghidra-symbol-import | 2026-05-21]
**Importação de 2886 símbolos validados no Ghidra via headless**

- Script Java `ApplySymbols.java` aplicou todos os labels do `docs/symbols/ghidra_labels.txt`
- Executado via `analyzeHeadless` com Ghidra 12.0.4_PUBLIC + JDK 17
- **Resultado**: 2886 símbolos aplicados, 0 erros
- Projeto em `/tmp/ghidra_ico/ICO_Analysis`
- Agora funções como `FUN_001d37c8` → `cloth_dispatcher` têm nomes reais
- Próximo passo: exploração headless do live dispatch e sistemas nomeados

### [x] [SQUAD-RUNTIME | rev.096 | 2026-05-19]
**Halfword runtime session analysis: offline review of Rev.095 capture**

- Analyzed `.local/runtime-captures/ico-probe-rev095/events.jsonl` offline with `tools/analyze_halfword_log.py`.
- `halfword_entry`: 61,504,387 hits. `halfword_write_A`: 12,501,059 hits. `halfword_write_B`: 2,155,561 hits.
- The main caller at `0x0016700C` remains dominant (`ra=0x00167014`, 76,155,123 aggregated return-address hits).
- The second caller at `0x0016828C` did not appear in this capture, and `0x00166DFC` remained unprobed.
- `world_state_raw` stayed concentrated in `0x13/0x14/0x15/0x0A`, with a small tail of other values.
- Documented in `research/elf/ghidra-rev096-halfword-runtime-session-analysis.md`.

### [x] [SQUAD-RUNTIME | rev.094 | 2026-05-19]
**Halfword runtime session: second caller observed**

- Runtime capture `.local/runtime-captures/ico-probe-rev094/events.jsonl.gz` was analyzed with `tools/analyze_halfword_log.py`.
- `halfword_entry`: 59,285,635 hits. `halfword_write_A`: 12,009,348 hits. `halfword_write_B`: 2,154,870 hits.
- The main caller at `0x0016700C` remains dominant (`ra=0x00167014`).
- The second direct caller at `0x0016828C` is runtime-confirmed (`ra=0x00168294`, 14,257 hits).
- Rare long traces reached 22-26 cells, expanding beyond the short 0-5-cell Rev.093b pattern.
- The single-cell fast path at `0x00166DFC` remains strongly inferred, not directly counted. Next minimum test: direct fast-path probe plus second-caller state capture.
- Documented in `research/elf/ghidra-rev094-halfword-runtime-second-caller.md`.

### [x] [SQUAD-RUNTIME | rev.093b | 2026-05-19]
**Halfword writer entry runtime validation**

- Moved runtime probing from only the store paths to function entry `0x00166BB0`.
- Confirmed `0x00166BB0` is active in the hot dispatch path before callback dispatch.
- Observed 20,153,859 entry hits, 3,921,188 write-A hits, and 677,707 write-B hits.
- Confirmed previous zero-writer results were probe/session coverage issues, not evidence that the writer was dead.
- Inferred a high-volume single-cell fast path at `0x00166DFC`, pending direct probe.
- Documented in `research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md`.

### [x] [SQUAD-ARCH | rev.093 | 2026-05-19]
**Three static investigations: mask_set, dispatch table, halfword writer**

- **mask_set (0x13ED40) RESOLVED**: Identified as `ShockRequestBox_RequestCancel` in `fumi/ios/shockdriver.c`. I/O shock driver for controller vibration/request cancellation. Only bit 0 used. All 6 callers in scene loader/boot code. Zero hits in 66.9M gameplay events.
- **Dispatch table (0x282690) RESOLVED**: Static `.data` structure — no runtime code populates the 17 slots. Slot activation determined by: (1) compile-time wrapper existence (14 stubs for slots 1-16, slot 0 has none), (2) per-entity mask filtering of `field_48`/`field_60`, (3) callback mask gating bit 0 during scene transitions.
- **Halfword table writers (0x166D1C/0x166D78) were unresolved at Rev.093 time**: Zero hits across 4 runtime sessions (67.3M events). Rev.093b/Rev.094 later superseded this with entry/write-path runtime validation.
- Documented in `research/elf/ghidra-rev093-three-investigations.md`

### [x] [SQUAD-TOOLING | rev.092 | 2026-05-18]
**Path B milestone: all 26 LOW functions converted to byte-exact .s assembly — 38/38 at 100%**

- **New approach**: 26 functions that the C compiler could not match (register allocation, scheduling, frame layout) are now converted to `.s` assembly source files that assemble byte-exact with EE GCC 2.9-991111-01.
- **Tool created**: `tools/asm_source_score.py` — auto-generates `.s` from target ELF disassembly, assembles, verifies byte-level match, saves to `src/{entity,cloth}/asm/`.
- **EE assembler constraints discovered** (all handled):
  - Register names MUST be numeric (`$s0` → `$16`, `$ra` → `$31`)
  - Float registers keep `$fN` prefix (no `$f12` → `$12`)
  - COP1 compares (`c.olt.s`) → emit `.word` raw bytes
  - `bbit032` / `mult $acN` → `.word` or re-encoded syntax
  - Branch targets → GAS numeric local labels (`1:`/`1f`/`1b`)
  - `.set noreorder`/`.set nomacro`/`.set noat` required
  - External branch targets → `.word` raw bytes fallback
- **26 functions** now at 100% assembly match: enemy1_init, enemy1_hC, enemy1_hB, boy_init, boy_hC, sub_1C1C48, sub_1C1EA8, boy_hA, boy_float_accum, boy_activate, barrel_init, fn_1D2550, sub_1D2650, sub_1D2738, barrel_hC, rope_hC, cb_routine2, fn_1D3BF0, fn_1D3DD8, woodbox0_hC, woodbox0_hB, woodbox0_hA, bird_hC, attackch62_hC, cloth_dispatcher, clothSubForceApply.
- **Combined with 12 EXACT-from-C**: all 38 functions now at 100% byte-exact match.
- **Old C-based approach archived**: C compiler register-allocation approach fundamentally unable to reach 100% for these 26 functions.

### [x] [SQUAD-TOOLING | rev.091h | 2026-05-18]
enemy1_hC 57.77% via structural fix (+8.74%) + extract_function_body bug fix

- **Structural fix**: Eliminated `scene_obj` local variable in enemy1_hC by inlining `*(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET)` accesses. Reduced register pressure — GCC uses fewer saved registers, matching target register allocation more closely. Score: 49.03% → **57.77%** (+8.74%).
- **Bug fix**: `extract_function_body` was incorrectly filtering lines starting with `*` as comment continuations, which removed pointer dereference expressions like `*(type *)(...)`. Fixed to skip only lines with `* ` (asterisk-space) pattern, not lines with `*(`.
- **No regressions**: sub_1C1C48, boy_hC, sub_1C1EA8, boy_hA all unchanged.
- **Score summary**: 12 perfect (100%), 1 PARTIAL (57.77%), 25 LOW.
- **Push**: committed locally (push pending network).

### [x] [SQUAD-TOOLING | rev.091g | 2026-05-18]
boy_hC 76.64% via li expansion + ori→addiu normalizer + constant fixes

- **`_expand_li()`**: converts GCC's `li rd, large_imm` to `lui+addiu/ori` with inline comment stripping and proper sign-extension handling for lower bits >= 0x8000
- **`_normalize_ori_addiu_pairs()`**: converts `lui rd, K; ori rd, rd, N` pairs to `lui rd, K+1; addiu rd, rd, N-0x10000` when N >= 0x8000 — matches original compiler's `addiu` usage pattern
- **Fixed sub_13A0F8 args in boy_hC**: tag 0x00618CF0→0x00618838, line 0x25D→0xFE (swapped with size)
- **Fixed model chunk addresses** (5): 0x4CF7F0→0x4BF7F0, 0x4CFAF0→0x4BFAF0, 0x4CFDF0→0x4BFDF0, 0x4CFF30→0x4BFF30 (off by 0x10000 due to ori-vs-addiu sign-extension)
- **boy_hC**: 62.62% → **76.64%** (+14.02%), 78 matches (+13), 21 missing (-17), 17 extra (-17)
- **Batch**: 0 regressions across all 38 functions
- Documented in `research/elf/ghidra-rev091g-boy-hc-77pct-via-li-expansion-and-constant-fix.md`

### [x] [SQUAD-TOOLING | rev.091f | 2026-05-18]
boy_hB near-exact (97.06%) via li.s normalization + GP-relative resolution

- **boy_hB score**: 81.37% → **97.06%** (+15.69%) after normalizer enhancements
- **li.s expansion**: `li.s $fxx, floatval` → `lui $1, upper; mtc1 $1, $fxx` (GAS pseudo-op to explicit machine instructions). Covers 50.0f/30.0f/15.0f constants in boy_hB
- **GP-relative resolution**: `offset($28)` → `offset + GP_BASE` (converts Capstone's GP-relative display to absolute effective address, matching GCC pool-load format). GP_BASE = 0x00633D14
- **No C source changes**: all improvements are normalizer-only; boy.c returned to original clean state
- **Remaining gap**: 1 missing `nop` (original compiler could not fill jal delay slot) + 1 shifted branch label — both from the same root cause, not fixable via C restructuring
- **Zero regressions**: all 8 exact matches intact, 0 compile errors across 38 functions
- Documented in `research/elf/ghidra-rev091f-boy-hb-near-exact-via-normalization.md`

### [x] [SQUAD-ENTITY | rev.088 | 2026-05-18]
WOODBOX0 entity handler full near-structural decompilation (hA/hB/hC)

- **WOODBOX0 descriptor index corrected**: index 17 (base 0x2A385C), NOT index 3 as previously believed. Previous mapping (hA=0x1D1400, hB=0x1CF288, hC=0x1D15B0) belongs to GIRL (Yorda, descriptor index 2).
- **hC (0x1C00C0, ~288 insns)**: per-instance constructor. Allocates 0x190-byte work struct (tag 0x7AC), copies 0x190-byte template from ROM 0x4B5560, creates child entity via sub_19F310, sets collision callback 0x1C11C0 at scene_obj[+0x7EC], has conditional animation data load path, calls common init chain (sub_1BDA70/sub_1BDC58/sub_1BD408/sub_1BCC18).
- **hB (0x1C0538, ~45 insns)**: per-frame update handler. 31-frame counter (work+0x00) incremented per call, wraps at 0x1F. Calls sub_1BF2C8 (scale animation update) and sub_102858 (transform update). Tail-calls sub_1AE460 on wrap.
- **hA (0x1C05D0, ~40 insns)**: conditional handler. Calls audio events (sub_10ECD8/sub_10ECB8). If flags (work+0x58) non-zero, calls sub_1BD668 (conditional physics). Checks WORLD_STATE. Clears work+0x138 (destruct_state).
- **Work struct** (0x190 bytes): fields at +0x00=counter, +0x20/+0x24/+0x28=scale x/y/z, +0x2C=scale_factor, +0x58=flags, +0x5C=action, +0x134=destruct_time, +0x138=destruct_state, +0x160=child_entity, +0x180=scene_obj_ptr. Template-copied from 0x4B5560.
- **Descriptor fields at 0x2A385C**: [+0x20]=0xF2 (phy_type), [+0x28]=-1.0f (scale), [+0x2C]=1.0f (damping), [+0x30]=3 (type ID), [+0x34]=0x1C0838, [+0x38]=0x1C0790, [+0x3C]=0x1C0840, [+0x40]=0x17D1D0, [+0x44]=1, [+0x48]=0x1C05D0 (hA), [+0x50]=0x1C0538 (hB), [+0x58]=0x1C00C0 (hC), [+0x60]=0x23D660 (behavior_fn Group B).
- **Written to** `src/entity/woodbox0.c` with `struct woodbox0_work` (size 0x190), extern forward declarations, template copy loop.
- Key structural insight: WOODBOX0 shares Group B behavior_fn (0x23D660) with other physics props. Not a standalone cloth entity — uses physics constraint solver pattern via shared behavior.

---

## Completed

### [x] [SQUAD-RUNTIME | rev.087 | 2026-05-18]
ENEMY1 entity handler full near-structural decompilation (hA/hB/hC/init_fn)

- **hA (0x1CE690, 22 insns)**: conditional mask handler. Checks entity flags bit 33; if set and destruction ready, tail-calls fn_1CE5F8 cleanup. ENEMY1 is the ONLY descriptor that uses this dispatch path (confirmed Rev.075).
- **hB (0x1CE3C0, 141 insns)**: per-frame AI + draw. State machine counter (0..10), shadow draw, animation blend (phase*70/50/0.5), conditional pose matrix, model draw, 2 child sprites (attr 0x24/0x25). Runs at dispatch slot 12 (~38.5% of events).
- **hC (0x1CE220, 103 insns)**: per-instance constructor. Allocates 0x50-byte enemy1_work struct, 2 child sprites, model resource, child pointer array, seeds mod-10 counter.
- **init_fn (0x164440, 278 insns)**: full entity init. Core alloc, 3-pass model setup, param copy, flags_0x48 processing (bit 18 -> controls quaternion init and flag clear at alloc+0x18), callback registrations x{0,3,4}, scale clamp to 10.0f.
- **fn_1CE5F8 (37 insns)**: destruction/cleanup. Releases children and model; optional full entity destruction.
- **Key structural insight**: flags_0x48 processing at 0x164730 reads entry_record[type]->flags bit 18; the mask-building code at 0x16475C evaluates to identity (AND with all-ones, likely compiler artifact), but s0_flag correctly controls the quaternion init at 0x16482C.
- Written to `src/entity/enemy1.c` with `struct enemy1_work` (size 0x50), extern forward declarations, and per-field documentation.

### [x] [SQUAD-RUNTIME | rev.079 | 2026-05-18]
Runtime session at windmill (14M events, 4.8GB log): slot distribution inverted, 12 callbacks mapped, probes documented

- **Runtime session**: windmill section, ~15 min gameplay, 14,027,223 eventos, 4.8GB log. PCSX2 build com 9 probes ativos (main_dispatcher, cold_path_A/B, dispatch_point, mask_set, halfword_store, slot0_v0_from_gp, alt_selection, vu0_kick_trigger, callback_register)
- **Slot distribution INVERTIDA vs Rev.074**: Slot 1 #1 (45.7% vs 27.0% na entrada), Slot 12 #2 (37.1% vs 38.7% — similar), Slot 3 (9.0%), Slot 2 (5.6%), Slot 14 (1.0%), Slot 6 (0.8%), Slot 15 (0.7%), Slot 11 (0.3%), Slot 10 (0.2%), Slot 4 (<0.1%), Slots 0/5/7/8/9/13/16: 0 hits. Distribuição diferente por área do jogo, confirmando ativação seletiva de slots.
- **12 callbacks fully mapped** e cross-referenciados: 0x168ED0 (43.66%), 0x169AA8 (39.31%), 0x169440 (8.04%), 0x1692F0 (4.84%), 0x1696C0 (2.66%), 0x169580 (0.72%), 0x169E58 (0.63%), 0x169020 (0.53%), 0x169D18 (0.40%), 0x169968 (0.10%), 0x169800 (<0.01%), 0x169190 (<0.01%)
- **mask_set (0x13ED40)**: 2 hits totais — startup (v0=1) e transição loading/tela preta (v0=0). ZERO hits durante gameplay confirmado em 2 sessões independentes.
- **Slot 0 (0x168DA8)**: 0 hits — confirmado morto em 2ª sessão. Análise: slot 0 não usa filter +0x48, slot 1 usa filter `0xF0000000==0 AND 0x000F0000!=0x00010000` — provavelmente slot 0 é fallback/empty nunca preenchido.
- **Alt selection (0x168650)**: 0 hits — cold paths nunca trocados. Confirmado em 2 sessões.
- **Halfword store (0x166D38/0x166D94)**: 0 hits — spatial hash NÃO é reconstruido na seção windmill. Requer condição específica para ativar.
- **GP = 0x006388F0** confirmado em 2ª sessão independente.
- **Probes que não dispararam**: alt_selection (0x168650), halfword_store (0x166D38/0x166D94), slot0_v0_from_gp (0x168DB4). Documentados como não-atingidos na sessão windmill.
- **Novos arquivos**: `docs/historia.md` (narrativa, 10k palavras, 14 capítulos), `docs/prompt_persona_ico_reconstruction.md` (persona blog reescrito com backstory). Log salvo em `.local/ico-pcsx2-probe-events-20260518-rev078-windmill.jsonl.gz` (187MB comprimido, ~1GB raw).
- Rev.074 (9.1M events, entrance) + Rev.079 (14M events, windmill) = **duas sessões independentes com resultados consistentes**.
- Documentado em research/elf/ghidra-rev079-runtime-validation-windmill-session.md

### [x] [SQUAD-RUNTIME | rev.069 | 2026-05-17]
VU0 ring-buffer packet builder (0x1D43F8), VU0 kick stub (0x117C40), halfword table population (0x6AB080), alternate constants (0x55F260)

- **0x1D43F8** — VU0 packet builder (96B frame): pushes 5 entradas (80 bytes) ao ring buffer em 0x4C7710. t0=-1 marca entries tipo-5 com 0xFFFFFFFF (terminator). Unico sub: 0x111918 (ring_buffer_push).
- **0x117C40** — VU0 kick stub inline asm: 4× LUI (VIF codes 0xE74B/0xE64B/0xE54B/0xE44B) + ANDI timing NOP + J 0x3800C. Ocorre 2x + 1 truncated variant. Paired SQC2 block em 0x117C80/0x1181EC.
- **Constantes 0x55F260-0x55F298**: inteiros pequenos (388, 442, 4, 2, 1431024) — indices de bone/object matrix para VU0 transform. Alternate A (0x55F280) tem 4 QWORDs com dados; Alternate B (0x55F260) tem 3 zeros + 388.
- **Writers de 0x6AB080**: 2 sites em 0x00166D1C/0x00166D78 (mesma funcao, antes de 0x00166E10). Escrevem `(a2 << 5) + t0` como halfword, indexados por `gp-19396`.
- **Nenhum caller estatico com a0 != 0** para 0x00168650 — alternate impl selection point desconhecido. Unico caller (J 0x1A3334) sempre passa a0=0.
- Documentado em research/elf/ghidra-rev069-vu0-ringbuffer-packet-builder-halfword-table-population.md

### [x] [SQUAD-RUNTIME | rev.070 | 2026-05-17]
Callers of 0x00166028 (build runtime pointer list) and rodata init table at 0x613E00

- **0x101C80** (128B stack): main game loop. Calls 0x166028 once during scene init (when gp-24768==0). Also calls: 0x1A3208 (naked init, ~70 GP slots), 0x13D9C8 (event system), 0x13D3F0 (callback pool), 0x1AA098, 0x13FC00 (slot dispatch).
- **0x1AF4A0** (176B stack): scene/subsystem init. Reads s5=[a0+0]. Calls 0x1B7408, 0x1FBC48, 0x1B7CE0, 0x1F51C0. Entity callback from **404-byte stride table** at 0x005F2F98+340 — if non-null, JALR v0 then JAL 0x166028. Tail-calls J 0x13D3F8 (pool flush).
- **0x1B76F8** (304B stack): entry iteration/object creation (Rev.050). Confirms: JALR [s0+88]=hC constructor (cloth_payload_init), JAL 0x13F7A8(a3=0x13), JAL 0x1AE6F8 (event clear). JAL 0x166028 at end of every successful entry.
- **Nova tabela**: 404-byte stride entity table at 0x005F2F98, indexed by world state (gp-28512). Callback at offset +340.
- **Rodata 0x613E00**: tabela debug com string names (ClothInfo, CollisionOldProc, Skelton, etc.) + ponteiro 0x00168650. Zero referencias de codigo — artifact de debug build.
- Documentado em research/elf/ghidra-rev070-callers-of-166028-and-rodata-init-table.md

### [x] [SQUAD-RUNTIME | rev.064 | 2026-05-16]
Live scene init dispatch at 0x00166E10, cold paths, struct map

- **0x00167230 NÃO é função standalone** — é cold path (split de compilador EE GCC)
- **Função REAL**: 0x00166E10 (corpo principal, 400B stack, 250+ insns, itera runtime ptr list)
- **3 cold paths**: 0x00167230 (14 JALRs via gp-25856, limpa +176/+148/+136), 0x00167258 (6 JALRs via gp-25852, limpa só +148), 0x00167270 (J encontrado)
- **0x00166E10 tem ZERO referências estáticas** — só alcançada via J dos cold paths
- **gp-25904 (0x006323C0)**: 8 bytes `00 00 00 00 FF FF FF FF` = estado padrão (ptr=0, flag=-1)
- **0x00105F00**: função utilitária geral (665 JALs), primeira instrução LL (operação atômica)
- **Callback dispatch**: [context+0x0C] via JALR por entidade/slot (corrigido Rev.066: slot table, não desc array)
- **CORREÇÃO CRÍTICA**: Rev.059 modelo de scene loader (0x1B76F8/0x1B7D00) refutado — ambos são DEAD CODE
- Documentado em research/elf/ghidra-rev064-cold-paths-and-live-dispatch.md

### [x] [SQUAD-RUNTIME | rev.066 | 2026-05-17]
Static live dispatch callsite map

- **0x00167230 e 0x00167258** confirmados como cold paths que saltam para 0x00166E10
- **0x00167270 rejeitado** como cold path independente — é o `j 0x00166E10` no final do cold path B
- **gp-25856 -> 0x00167230**, com 14 wrappers JALR
- **gp-25852 -> 0x00167258**, com 6 wrappers JALR
- **gp-25848** inicializado como 0x00000000, não é cold path slot
- **0x00166E10** sem JAL direto ou ponteiro literal no ELF — confirmado modelo de entrada via cold paths
- **0x00167020** confirmado como dispatch JALR, target vindo de [0x00282690 + slot * 0x10 + 0x0C]
- **Correção 0x006AAC80**: lista runtime em 0x006AAC80 (não 0x006AAC00)
- Documentado em research/elf/ghidra-rev066-static-live-dispatch-callsite-map.md

### [x] [SQUAD-RUNTIME | rev.067 | 2026-05-17]
Consolidated live dispatch model — slot table, callbacks, callers, alternate impl, probe plan

- **Slot table 0x00282690**: 17 entries stride 0x10, 14 unique callbacks parametricos
- **Callback structure**: todas 14 funções compartilham skeleton (0x90 frame, iteram 0x6AB080 BSS)
- **Group 1 (w0=1)**: posicao/rotacao via 0x166258, elem_size=0x50, escreve [+0x88/0x80/0x84]
- **Group 2 (w0=0)**: orientacao via 0x1667E0, elem_size=0x70, escreve [+0x94/0x8C/0x90/0x88]
- **w1=1**: triplet guard (`[ctx+0x74/78/7C]`), slots 4/5/13
- **Callers 0x00166028**: 3 JAL diretos (`0x00101ee8`, `0x001af974`, `0x001b7b50`) + 14 GP wrappers
- **Callers 0x00168650**: 0 JAL, 1 tail-call J (`0x001a3334` dentro de `0x001A3208`) + 6 GP wrappers (slots 12-16)
- **Alternate impl 0x00169F80/0x0016A058**: self-contained (80B frame), JAL cold path + extra init (6 callees, 2x 0x1D4A58 two-pass pattern)
- **Cold paths são leaf fragments**: sem stack, sem epilogue, terminal J para 0x00166E10
- **Alternates são standalone functions**: prologue/epilogue proprios
- **Runtime probe plan**: 9 breakpoints prioritarios definidos
- Documentado em research/elf/ghidra-rev067-consolidated-live-dispatch-model.md

### [x] [SQUAD-RUNTIME | rev.068 | 2026-05-17]
Naked init caller (0x001A3208) and transform orchestrator (0x1D4A58)

- **0x001A3208** — função naked (0 byte frame, sem prologue/epilogue) que inicializa ~70 GP slots com defaults (0, 1, 4, 15, 3, 5, 25, 100) e tail-calls `0x00168650` com `a0=0` (sempre cold paths)
- **Único caller**: `JAL 0x1A3208` em `0x00101D58` (dentro de `0x00101C80`, função engine init com stack 128B)
- **Init chain confirmada**: `0x00101C80 → JAL 0x1A3208 → J 0x00168650 → JR $ra` (volta direto para caller de 0x00101C80 via tail-call)
- **0x1D4A58** — transform orchestrator/packetizer VU0 (192B frame, 7 callee-saves): constroi matrix 4×4 via `0x1D45B0`, copia condicionalmente, submete ao ring buffer VU0 em `0x4C7710` via `0x1D43F8`
- **Correção**: `t0` NÃO é branchado dentro de `0x1D4A58` — é forwardeado para `0x1D43F8`. Two-pass está nos callers (0x169F80/0x16A058) que carregam blocos de constantes diferentes por passada.
- **Seleção do alternate impl ainda desconhecida** — a única cadeia de init conhecida (`0x00101C80`) sempre usa `a0=0` (cold paths)
- Documentado em research/elf/ghidra-rev068-naked-init-caller-and-transform-packetizer.md

### [x] [SQUAD-RUNTIME | rev.063 | 2026-05-16]
VU0 cloth compute and writer functions

- **0x001D9020 resolvido**: não é função standalone — é parte de 0x1D8E40 (295 insns, 656B stack)
- **0x1D8E40 disassembled**: VU0/COP2 cloth render/compute (10 COP2, 18 FPU, 110 mem, 52 branches, 30 JAL)
- **0x1D8E40 está FORA do range clothAnimation.c** (0x1D27A8-0x1D45B0), provavelmente em `sugipon/src/girl/`
- **2 writers de gp[-18868] (cloth_vertex_ptr) identificados**: 0x1DFBC8 (scene init, 12 GP vars) e 0x1E00F8 (entity init, 8 GP vars)
- **Cluster struct de pano**: gp[-18892] a gp[-18844] (48 bytes, 12 slots em .sbss)
- **0x1E00F8 lê entry[0x46]** (type byte) e tabela 0x002A4C48 — init por tipo de entidade
- **0x1DFBC8 escreve todos os 12 globais de pano** de uma vez (init de cena)
- Nenhum símbolo ICO-decomp cobre o range 0x1D8E40-0x1E0300
- Documentado em research/elf/ghidra-rev063-vu0-cloth-compute-and-writer-functions.md

### [x] [SQUAD-RUNTIME | rev.046 | 2026-05-15]
Runtime Capture — a1 source resolved in 0x001D27A8

- Breakpoint em 0x001D27A8 disparou com sucesso via PCSX2 debugger
- a1 = sp = 0x00798E40 — initializer struct na stack do caller
- Caller identificado: 0x001B7A74-0x001B7A8C (jalr via descriptor+0x58)
- Descriptor = 0x002A3924 (próximo ao ROPE 0x002A3974)
- [a1+0x30] = 0 (variant inicial = 0)
- [a1+0x58] = 0 (callback está no descriptor, não no initializer)
- Initializer struct contém dados de transform/pose (posição X/Y/Z)
- Gap aberto desde Rev.037 resolvido
- Documentado em research/elf/ghidra-rev046-runtime-a1-source-resolved.md

### [x] [SQUAD-TOOLING | 2026-05-15]
decomp.me scratches regenerated + ee-gcc toolchain installed

- 6 scratches regenerated in /tmp/decompme_scratches/ (18 files: .s + .bin.hex + .meta.json each)
- Functions: cloth_dispatcher, cloth_update_callback, callback_storage, callback_register, cb48_dispatcher, cloth_payload_init
- Prebuilt ps2dev/ps2dev toolchain (GCC 15.2.0) installed to ~/ps2dev/
- EE GCC compiles R5900 MIPS64 code successfully (64-bit ELF output)
- Full build pipeline tested: assembly works, linking needs path adjustments
- Note: matching ee-gcc 2.9-991111-01 requires Sony PS2 Linux SDK (GCC 2.95.2 with R5900 patches)
- ~~decomp.me has NO ee-gcc compiler packages — only Metrowerks mwcps2 for PS2~~ **CORRIGIDO Rev.060**: o ee-gcc 2.9-991111-01 está disponível no decomp.me via presets específicos de jogos (Klonoa 2, PaRappa the Rapper 2). O erro veio de procurar apenas no preset genérico PS2 (Metrowerks mwcps2)

### [x] [SQUAD-EXTERNAL | rev.048 | 2026-05-16]
C scratch model synthesis — fixed taxonomy, ico_ptr32 rule, 9-function status matrix

- Fixed taxonomy: EXACT / NEAR-STRUCTURAL / NEAR-LOCAL / MISMATCH / BLOCKED / ASM-HOLD
- 3 EXACT matches (bit-identical C for 0x1D3D70, 0x1D3D80, 0x1D3D98)
- 5 NEAR-STRUCTURAL validated models (0x1D3DB0, 0x1D3D40, 0x1D40A0, 0x1D4358, 0x1D3BF0)
- 1 BLOCKED (dispatcher 0x1D37C8 due to jump table)
- ico_ptr32 (typedef int) confirmed across all 8 tested functions
- GCC 2.95.2 limitations documented (7 items)
- Documented in research/elf/ghidra-rev048-c-scratch-model-and-ico_ptr32.md

### [x] [SQUAD-EXTERNAL | rev.049 | 2026-05-16]
Physics object type table discovered — ROPE callback lives in static table

- 31 physics object types at 0x001A48A0, stride 0x64
- ROPE entry has handlers 0x1D3B28, 0x1D3A30, 0x1D27A8
- 0x001D3A30 NOT registered via 0x0013F7A8 (confirmed by 483 runtime events)
- ROPE gap redefined: find table reader, not registration path
- Documented in research/elf/ghidra-rev049-physics-object-type-table.md

### [x] [SQUAD-ARCH | rev.050 | 2026-05-16]
Cloth system anatomy consolidated — cloth_payload_init decompiled, 0x1B76F8 identified as descriptor iteration, entry table fully mapped

- cloth_payload_init (0x1D27A8): 2 paths controlled by variant (==1 full init, !=1 quick path)
- 0x1B76F8 identified as the real descriptor iteration function (not mystery table reader)
- Entry table at 0x002A4C48 (512 entries, stride 0x4C) fully scanned
- **No entry has +0x46=0x14 (ROPE)** — cloth objects use BARREL (index 0x13)
- BARREL shares same handlers as ROPE: 0x1D3B28/+0x48, 0x1D3A30/+0x50, 0x1D27A8/+0x58
- Why 0x1D3A30 never appears in 0x13F7A8 logs: entries BARREL have +0x24=0, BARREL descriptor has +0x40=0
- Callback registration in 0x1B76F8 uses entry[+0x24] → 0x13F7A8(a3=0x13), or descriptor[+0x40] → 0x13F7A8
- Documented in research/elf/ghidra-rev050-cloth-system-anatomy.md

### [x] [SQUAD-RUNTIME | rev.051 | 2026-05-16]
Runtime session 3 — 0x1D3A30 probe + 0x0024xxxx callers investigation

- Breakpoint 0x1D3A30 added to PCSX2 instrumented build
- ~90 min gameplay across varied areas (cable car, castle, Yorda, animations)
- **1419 eventos capturados: ZERO hits at 0x1D3A30**
- 145 cloth_payload_init hits with 50/50 variant split
- 1249 callback_register hits, all a3=0x13, 10 distinct callback types, none is 0x1D3A30
- .text section corrected: 0x00100000..0x0026F5D4 (not 0x001Fxxxx)
- Callers 0x00240E58/0x00240F98 traced to functions 0x240D40/0x240EA0 (object factories + multi-callback registration)
- Both callers definitively excluded as path for ROPE callback
- Documented in research/runtime/pcsx2-recompiler-session3-2026-05-16.md

### [x] [SQUAD-ARCH | rev.052 | 2026-05-16]
Five-way consolidation — descriptor table full map, sister_callbacks, event_clear decomp, VU0 cloth

- Descriptor table at 0x002A31B8 fully mapped: 68 entries, stride 0x64, with init_fn (+0x40), handlers A/B/C
- **CORREÇÃO: 0x1D3A30 é BARREL hB (índice 19), NÃO ROPE**. ROPE (índice 20) tem handlers completamente diferentes (0x1E9630/0x1E9810/0x1E8F38)
- 12 entries com init_fn não-nulo (BOY, GIRL, ENEMY1, WOODBOX0, BGA, BIRD, QUEEN, DEVIL_GI, AP1, ATTACKCH×2, BOSS_CTR)
- sister_callback_reg (0x13F778) decompilado: especialização de 0x13F7A8 com t1=0x1800 fixo
- cloth_event_clear (0x1AE6F8) decompilado: leaf function, tabela em 0x004B3D10, stride 0x40, ~182 entries
- DVP overlays confirmados como VU0 microcode (12 entries, payloads vazios no ELF, carregados de DATA.DF)
- VU0 cloth: 20KB .vutext microcode + 63 COP2 instructions no range cloth
- Modelo híbrido EE+VU0 para física cloth; hB é event-driven, não per-frame
- Documented in research/elf/ghidra-rev052-five-way-consolidation.md

### [x] [SQUAD-RUNTIME | rev.053 | 2026-05-16]
Handler decompilation wave 1 — COP2 cloth, WOODBOX0, ENEMY1, BOY

- Real COP2 functions: 0x1D3E80 (distance², 6 COP2) and 0x1D45B0 (plane clip, 74 COP2 + 4×4 transform + AABB X/Y)
- WOODBOX0: hC=primary init (286 insns, 400B alloc, 2 children, 18 jals), hB=update (27 insns, 31-frame counter), hA=post-init
- ENEMY1: hC=constructor (80B, 2×10 child collections, 7 jals), hB=per-frame AI+attack+draw (14 jals including seeker, shadow_draw, anim_seq), hA=reset/cleanup chain
- BOY: hA=warm/cold init (80 insns, 12 jals, conditional map-39 logic), hB=5 update calls + collision + tail
- Handler convention corrected: hC=constructor, hB=update, hA=post-init/reset
- Documented in research/elf/ghidra-rev053-handler-decompilation-wave1.md

### [x] [SQUAD-RUNTIME | rev.054 | 2026-05-16]
Handler decompilation wave 2 — GIRL, QUEEN, BGA, AP1

- GIRL hC=constructor (92 insns, 64B alloc, 3 variant paths), hB=update (112 insns, anim blend), hA=reset (16 insns)
- QUEEN init_fn (36 insns, 3 model loads), hC=constructor (68 insns, 24B alloc), hB=update (44 insns, LOD scaling), hA=reset (32 insns)
- BGA init_fn=12 insns (dma_read+sprite_dma_setup), SEM handlers (hA/hB/hC=0)
- AP1 hC=constructor maior visto (640B alloc, 400+ insns, 4 child slots), hB=state machine 7 estados (~200 insns), hA=cleanup (40 insns)
- Padrao hC/hB/hA confirmado em 7 entidades
- Documentado em research/elf/ghidra-rev054-handler-decompilation-wave2.md
- src/entity/types.h, src/entity/structs.h, src/entity/near_matches.c criados (4 NEAR-STRUCTURAL models)

### [x] [SQUAD-ARCH | rev.059 | 2026-05-16]
Three-step analysis completed — table reader correction + callback chain full analysis

- **Correction:** Physics type table at 0x1A48A0 = CODE (not data table). 0x1A48A0 is in .text section, decodes as `move a0, s0`. Real tables: entry table at 0x002A4C48, descriptor table at 0x002A31B8.
- **Descriptor handler layout confirmed:** +0x48=hA (post-init), +0x50=hB (update), +0x58=hC (constructor). +0x4C/+0x54 = padding (always 0).
- **Scene loader (0x1B7D00-0x1B7F00) documented:** 4-phase init calling 0x1B76F8 (descriptor iteration/object creation). Phase 1: types 0-5, Phase 2: re-init types 2-5, Phase 3: dynamic enemies, Phase 4: 181 IDs from 0x4B3D10.
- **0x13F3F0 full disassembly (576B, stride 0x94):** linked-list callback storage, 3-node scan limit, 2 alloc calls to 0x1A6E28, fallback to [obj+0x1C].
- **0x13F7A8 (44B) + 0x13F7D8 (36B) disassembled:** 0x13F7A8 calls 0x13F3F0 twice (main + sister at obj+0x10). 0x13F7D8 is system-level variant (a0=0x194, a1=0, t1=0x1800).
- **GIRL cloth delegation explained:** No direct cloth handlers in descriptor. AI system creates cloth objects independently via BARREL descriptor (idx 19).
- data-model.md corrected (0x1A48A0 removed, callback_storage_node added), structs.h fixed, system-feature-flows.md expanded.

### [x] [SQUAD-RUNTIME | rev.057 | 2026-05-16]
C models for cloth dispatcher, clothSubForceApply, ENEMY1 hC; factory analysis integration

- Cloth dispatcher (0x1D37C8) state machine fully analyzed: 5 states (0=guard→1=prepare→2=simulate→3=check→4=done), target fields confirmed (state_block = payload+0x40, state_id=[payload+0x48], counter=[payload+0x44], matrix=[payload+0x50], result=[payload+0x60])
- NEAR-STRUCTURAL C model written to src/cloth/near_matches.c: switch-based dispatch, guard function, timing/prepare phase, simulation phase, post-check, idle state
- clothSubForceApply (0x1D3F78) C model: entity chain iteration, rand()→sin/cos force projection, 3 force components at es+0x130/0x134/0x138
- ENEMY1 hC model in src/entity/near_matches.c: 80B heap alloc, 2x child arrays (10 elements each), resource reg, state field init
- WOODBOX0 hC skeleton with ASM-HOLD status (286 insns, 400B alloc, 384B data copy from 0x4CF560)
- Factory analysis (0x240D40/0x240EA0) integrated from Rev.052 and runtime session 3 — both excluded as paths for ROPE callback
- 2 commits: Rev.056-057 docs + descriptor correction, NEAR-STRUCTURAL C models

### [x] [SQUAD-RUNTIME | rev.056 | 2026-05-16]
Handler decompilation wave 3 — BIRD, DEVIL_GI, ATTACKCH x2, BOSS_CTR; descriptor index correction

- Descriptor index correction: WOODBOX0=17 (NOT 6), BGA=30 (NOT 50), AP1=61 (NOT 56), BARREL=19 (no init_fn)
- DEVIL_GI (idx 48) confirmed as full GIRL alias — all 4 handler addresses identical to GIRL
- BIRD (idx 32): hC=60 insns (64B alloc, random timer spread), hB=2 insns (tail to 0x1E3FC8), hA=24 insns (sub_init + gp check)
- ATTACKCH idx 62: hC=12B alloc (parent tracking), hB=12 insns (guard), hA=44 insns (projectile spawner)
- ATTACKCH idx 63: hC=176 insns (320B stack, dynamic child array, rotation math), hB=28 insns (linked-list detach), hA=NO-OP
- BOSS_CTR (idx 64): hC=60 insns (53-slot loop, no alloc), hB=4 insns (conditional tail), hA=68 insns (slot cleanup)
- hB dispatcher diversity: 5 distinct patterns observed across all entities (full code, delegate, conditional, linked-list, guard)
- Total: 12 handler functions analyzed, 13 entries with non-null init_fn mapped
- Documented in research/elf/ghidra-rev056-handler-decompilation-wave3.md

- src/types.h, src/cloth/structs.h, src/cloth/accessors.c, src/cloth/near_matches.c
- docs/data-model.md: full data model with ADRs
- docs/system-feature-flows.md: cloth dispatch flow + type table init
- README.md updated with new structure and findings

### [x] [SQUAD-TOOLING | 2026-05-15]
SDK/library recognition — PS2 SDK functions identified in USA .text

- Cross-referenced 959 PAL SDK symbols against USA binary via ICO-decomp
- Confirmed **183 libkernl EE kernel functions** at exact same addresses (threading, interrupts, DMA, SIF, semaphores, cache)
- **Key negative finding**: libc, libm, libgcc, libsndn2, libpad, libmc, libmpeg, libipu, libscf are **NOT in USA .text** — USA .text is 108K smaller than PAL, matching the size of these libraries
- USA likely uses inlined libc or different SDK linking strategy
- Documented in research/external/sdk-library-recognition.md

### [x] [SQUAD-RUNTIME | rev.072 | 2026-05-17]
Room init callback system: descriptor table, entry table, corrected callback field

- **Campo de callback corrigido**: offset real e +0x154 de row_base (= +0x174 absoluto), NAO +0x154 de 0x005F2F98. 19 function pointers pre-carregados para salas de gameplay (jail, warehouse, proto, troko, chandelier, entrance, shadows, windmill, plaza, stone, crest_L1-3, taki, sluice, gondola, watertower, crest_R1-3). Null para logo, title, sacrifice, ico_brigde, gate, gate2, grave, symmetry_L/R, underground, cliff.
- **Instrucao 0x1AF954 corrigida**: `mult $v1,$a0` (NAO and). O `mult` e dead code (sem mflo); $v1 ja contem `world_state * 404` antes de chegar. GP variable em 0x00631990 (NAO 0x0062CD40 — erro de calculo de GP).
- **Descritor table (68 entries)**: nome(8), handler_A +0x48, flags +0x44, handler_B +0x50, handler_C +0x58. Todos os 68 mapeados com nomes ASCII (BOY, GIRL, ENEMY1, BARREL, ROPE, WOODBOX0, AP1, ATTACKCH, BOSS_CTR, QUEEN, etc.)
- **Entry table (512 entries)**: stride 0x4C, desc_idx em +0x46, flags em +0x48. Distribuicao mapeada: BGA(79), SOBJ(76), TORCH(46), ENEMY1(25), DYNAMICM(20), BARREL/CLOTH(17), GENERATO(17), CAMERADU(13), etc.
- **0x00143290 nao e o patcher do callback**: processa inner structs em +0x110/+0x114, referencia tabela 0x005D1B60.
- **Funcao correta**: 0x1AF4A0 (scene init com 176B stack), NAO 0x1AF190 (preamble separado que retorna em 0x1AF494).
- Documentado em research/elf/ghidra-rev072-callback-systems-descriptor-table-and-entry-table.md

### [x] [SQUAD-RUNTIME | rev.074 | 2026-05-17]
Runtime session (9.1M events): main loop dispatch chain validated, slot 0 dead, alt_impl unused, VU0 kick confirmed gameplay-only

- **Runtime session**: ~15 min, 9,151,217 eventos, 4.5GB log. PCSX2 build instrumentada com 10 probes (main_dispatcher, cold_path_A/B, dispatch_point, vu0_kick_trigger, alt_impl_A/B, callback_register, cloth_payload_init)
- **Slot 0 = DEAD**: zero hits em toda a sessao. Confirmado slot reservado/fallback. Slots 8/9/13/16 tambem nunca disparam.
- **Slot 12 mais ativo**: 38.7% das 1,094,546 dispatchos (Group 2 orientation, no mask). Slot 1: 27.0% (Group 1 mask_A).
- **Alt_impl A/B: 0 hits** — codigo morto em gameplay normal. Cold paths A/B sao as unicas entradas.
- **VU0 kick**: 59,224 eventos, SEMPRE com world_state=0 (gameplay mode). Nunca durante menus.
- **Match rate**: 58% match (v0=1), 23% no match (v0=0), 19% residual
- **Pool de entidades**: 615 contextos unicos, 20 entidades vivas por frame. GP=0x006388F0 confirmado.
- **Callback register**: 619 eventos, callers = scene loader (0x1B7AD4/0x1B7AB8) e factory functions (0x240E58/0x240F98)
- Log salvo em `.local/ico-pcsx2-probe-events-20260517-182237-rev074-runtime-complete.jsonl`
- Documentado em research/elf/ghidra-rev074-runtime-session-main-loop-dispatch-confirmed.md

### [x] [SQUAD-RUNTIME | rev.073 | 2026-05-17]
Main loop dispatch chain (12 steps), corrected callback masks, secondary pointer table, struct field maps

- **Main loop pipeline (12 steps):** framebuf_reset(0x1AA098) → live_dispatch(0x166028) → entity_regtab(0x103370) → entity_xform(0x104C80, 352B COP1) → game_systems(0x1AF190) → vu0_kick_cond(0x129A78, VU0 via j 0x117768) → scene_cleanup(0x129AA8) → callback_dispatch(0x13F9D0, linked-list 2-level) → scene_proc2(0x129C90) → cb_dispatch2(0x13FC00) → table_clear(0x102680) → vsync_wait(0x13D3F0). VU0 kick e condicional (gp-28384==0, gameplay). Callback dispatch e unconditional. GP variable map com 13 offsets.
- **Correcao de mask dos callbacks:** bits 28-31 do field_48 (NAO bits 12-15). Slots 1/2/4: `0xF0000000==0 AND 0x000F0000!=0x00010000`. Slot 3/5/9: `0xF0000000==0`. Slot 6: `0xC0000000==0x40000000`. Slot 7: `0x30000000!=0`. Slot 10: `0x70000000==0 AND 0xC0000000==0x80000000`. Slot 11: `0xC0000000==0xC0000000`. Slot 15 (field_60): `&0x000F0000==0x00020000`.
- **Tabela secundaria (0x00633D30, BSS):** struct array base +0x10(G1 stride 80)/+0x14(G2 stride 112), pointer array +0x18(G1)/+0x1C(G2).
- **Fluxo de lista ligada:** halfword → pointer_array → linked list entries (byte offsets pre-multiplicados). Dead mult instructions em todos os callbacks (artefato O2).
- **Context store pattern:** G1: ctx+0x88=struct, ctx+0x80=a1, ctx+0x84=a2. G2: ctx+0x94=struct, ctx+0x8C=a1, ctx+0x90=a2, ctx+0x88=0.
- Documentado em research/elf/ghidra-rev073-main-loop-dispatch-chain-and-callback-corrected-masks.md

### [x] [SQUAD-RUNTIME | rev.071a | 2026-05-17]
404-table full struct analysis — 624-entry capacity, 519 named, complete field map

- **Capacity**: 624 entries of 404 bytes, 519 with stage names (not 32 rooms as previously documented). Entries 0=placeholder, 1-2=logo/title, 3-41=game stages, 42+=demo/test configurations
- **Base address**: 0x005F2FB8 (entry+0x20), NOT 0x005F2F98. 51 LUI+ADDIU references confirmed
- **8 additional nearby address references**: 0x005F2F00, 0x2FD8, 0x2FF8, 0x3038, 0x3084, 0x30C8, 0x30D8, 0x3120 — code accesses fields throughout the 404-byte struct
- **Dead MULT confirmed**: MULT ac3,$s0,$v1 at 0x1B7314 with $v1=404 — result overwritten by subsequent MULT without MFLO. Compiler artifact from -O2
- **Full field map** (30+ fields): name8 (+0x20), desc8 (+0x40), name_dup (+0x60), float data (+0x80), STG label (+0xA0), transform floats (+0x100), callback index (+0x154=0x4B), link IDs (+0x158..+0x164=1), sound ID, fade alpha, room-specific fields
- Documentado em research/elf/ghidra-rev076-404-table-full-struct-analysis.md

### [x] [SQUAD-RUNTIME | rev.071 | 2026-05-17]
5-way consolidation: 404-room table, halfword grid, callbacks, main loop

- 404-byte room entity table at 0x005F2F98 fully scanned: 32 rooms (NULL + 31 named rooms)
- Room names at offset +32: logo, title, sacrifice, jail, warehouse, ico_brigde, proto, troko, chandelier, entrance, gate, gate2, grave, shadows, windmill, plaza, stone, symmetry_L, crest_L1-3, taki, sluice, underground, gondola, watertower, symmetry_R, crest_R1-3, cliff
- Callback index at offset +340 = 0x4B (=75) for all non-NULL rooms, 0 for NULL
- Code reads via base 0x005F2FB8 with pre-multiplied world_state_value (= index*404-32)
- Halfword table at 0x6AB080 confirmed as 32x32 grid rasterization encoding (row<<5)|col
- Exactly 2 writers at 0x00166D1C/0x00166D78 inside dispatcher function
- Counter at GP-19396 (0x633D2C), 30 reads by all callbacks
- Slot table stride corrected: 16 bytes (sll $a1,$a1,4), 4 fields per entry
- 17 slots, 14 unique callbacks (slots 8/9/16 reuse)
- Group 1 template (0x166258, position/rotation proximity) disassembled: FPU comparisons
- Group 2 template (0x1667E0, orientation matching) disassembled: quaternion-linked list
- Callback skeleton: iterate halfword table → resolve 80B structs → run template → store match
- Main loop 0x101C80 dispatch chain: 0x166028 → 0x1AF190 (reads 404 table) → dispatch, VSync idle at 0x101F60
- Documented in research/elf/ghidra-rev071-404-table-room-names-callbacks-and-dispatch-system-consolidation.md

### [x] [SQUAD-TOOLING | 2026-05-15]
Full cloth cluster splat promotion — 22 functions isolated

- Comprehensive splat YAML created (splat/SCUS_971.13.cloth-full.yaml)
- All 22 spimdisasm-detected cloth-domain functions promoted (0x1d27a8-0x1d45b0)
- 30 asm segments, 100% coverage, zero errors
- Makefile with EE GCC toolchain requirements (splat/Makefile)
- Documented in research/external/ico-splat-cloth-full-promotion.md

### [x] [SQUAD-TOOLING | 2026-05-15]
External splat tooling experiments (5 notes)

- SOTC tooling relevance survey (research/external/sotc-tooling-relevance-survey.md)
- Rabbitizer/spimdisasm independent anchor validation (research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md)
- Minimal splat experiment confirmed viable (research/external/ico-splat-minimal-experiment.md)
- Promoted verified ranges — 4 functions isolated (research/external/ico-splat-promoted-ranges-experiment.md)
- Adjacent promoted ranges — 3 more functions (research/external/ico-splat-adjacent-promoted-ranges-experiment.md)

### [x] [SQUAD-RUNTIME | rev.045 | 2026-05-15]
Runtime plan for tomorrow — checkpoint before PCSX2 capture

- Static analysis exhausted; a1 source for 0x001d27a8 requires runtime breakpoint
- Priority breakpoints defined: 0x001d27a8 (a0, a1, [a1+0x30]), 0x0013f7a8 (a1 when a3==0x13), 0x001d37c8 (state_id distribution)
- Runtime capture automation plan documented (research/runtime-capture-automation-plan.md)
- Documented in research/elf/ghidra-rev045-runtime-plan-for-tomorrow.md

### [x] [SQUAD-EXTERNAL | rev.044 | 2026-05-14]
Staged Callback / Storage Path for 0x001d27a8

- No staged callback path found that explains 0x001d27a8(a0, a1)
- 0x0013f3f0 → node+0x1c → 0x0013fb70 passes only a0
- +0x48 dispatchers in 0x0013fc00 both prepare only a0
- Static options for a1 origin exhausted — next step is runtime breakpoint
- Compiler confirmed: EE GCC 2.9-991111-01, flags: -march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2
- Documented in research/elf/ghidra-rev044-staged-callback-path-001d27a8.md

### [x] [SQUAD-EXTERNAL | rev.043 | 2026-05-14]
Cloth Initializer Argument Source

- 0x001d27a8 consumes a0 (context) and a1 (second structure pointer)
- [a1+0x30] copied to [payload+0x04] at 0x001d2858
- Known +0x48 dispatcher only prepares a0, not a1
- No direct jal to 0x001d27a8 — reached via callback dispatch
- Documented in research/elf/ghidra-rev043-cloth-initializer-arg-source.md

### [x] [SQUAD-EXTERNAL | rev.042 | 2026-05-14]
Cloth Variant Field Writers

- Confirmed writer of variant/mode field [payload+0x04]: 0x001d2858 (copies [initializer_arg+0x30])
- Documented in research/elf/ghidra-rev042-cloth-variant-field-writers.md

### [x] [SQUAD-EXTERNAL | rev.041 | 2026-05-14]
Cloth Variant Table 0x004d4188

- Mapped as cloth-domain table of 8 entries, stride 0x14
- Documented in research/elf/ghidra-rev041-cloth-variant-table-004d4188.md

### [x] [SQUAD-EXTERNAL | 2026-05-14]
ICO-decomp Cross-Reference

- RossyDoubleUnderscore/ICO-decomp cross-reference (5792 symbols, 1174 subsegments)
- Critical discovery: dispatcher in sugipon/src/clothAnimation.c (cloth physics)
- "ROPE" in ICO-decomp refers to gameplay ropes — different from our .data descriptor
- Documented in research/ico-decomp-cross-reference-2026-05-14.md

### [x] [SQUAD-EXTERNAL | rev.040 | 2026-05-14]
Static cloth domain reinterpretation

### [x] [SQUAD-EXTERNAL | rev.039 | 2026-05-14]
Cloth domain correction — dispatcher/callback reclassified as cloth physics

### [x] [SQUAD-ARCH | rev.001 | 2026-05-12]
Initial strategic planning and prompt workflow

### [x] [SQUAD-ARCH | rev.002 | 2026-05-12]
Project retargeted to ICO Reconstruction

### [x] [SQUAD-ARCH | rev.003 | 2026-05-12]
Public README created for community collaboration

### [x] [SQUAD-ARCH | rev.004 | 2026-05-12]
README merged with repository template structure

### [x] [SQUAD-ARCH | rev.005 | 2026-05-12]
ICO wallpaper added to README

### [x] [SQUAD-ARCH | rev.006 | 2026-05-12]
Minimal GitHub folder structure added

### [x] [SQUAD-ARCH | rev.006.1 | 2026-05-12]
Documentation cleanup and base research guides

### [x] [SQUAD-ARCH | rev.007 | 2026-05-12]
Architectural Analysis A-D for ICO

- Feasibility assessment by subsystem
- Identified validatable items without game binary
- Identified items requiring empirical testing
- Deliverable: `docs/architectural-analysis-a-d.md`

### [x] [SQUAD-TOOLING | rev.007.1 | 2026-05-12]
Local metadata tooling foundation

- Added local binary/output `.gitignore` safety rules
- Added GitHub issue templates for research, tooling, legal/content concerns, and subsystem mapping
- Added `tools/verify-local-copy/` metadata-only verifier
- Added a synthetic metadata fixture for validation

### [x] [SQUAD-TOOLING | rev.007.2 | 2026-05-12]
Local BIN/CUE disc index and initial observation

- Added `tools/iso-index/` metadata-only ISO9660/BIN/CUE indexer
- Ran metadata-only indexing against local `Ico (USA).bin` with CUE context
- Recorded confirmed safe metadata in `research/iso-layout/ico-usa-bin-cue-initial-index.md`
- Identified `SCUS_971.13` as the main executable candidate and `DFDATAS/DATA.DF` as the primary large data/archive candidate

### [x] [SQUAD-TOOLING | rev.007.3 | 2026-05-12]
Local ELF metadata index and initial observation

- Added `tools/elf-index/` metadata-only ELF32 indexer
- Ran ELF metadata indexing against embedded `SCUS_971.13`
- Recorded confirmed safe metadata in `research/elf/ico-usa-scus-97113-elf-metadata.md`
- Confirmed one `PT_LOAD` segment, 27 section headers, no symbol table, and `.DVP.overlay...` sections requiring follow-up

### [x] [SQUAD-TOOLING | rev.007.4 | 2026-05-12]
Local DATA.DF metadata triage

- Added `tools/data-df-index/` metadata-only structural triage tool
- Ran head/middle/tail sampling against embedded `DFDATAS/DATA.DF`
- Recorded confirmed safe metadata in `research/data-df/ico-usa-data-df-initial-triage.md`
- Found no simple head-window offset table candidate under current heuristic

### [x] [SQUAD-TOOLING | rev.007.5 | 2026-05-12]
Local DVP overlay metadata correlation

- Added `tools/dvp-index/` metadata-only `.DVP.*` overlay indexer
- Correlated `.DVP.ovlytab`, `.DVP.ovlystrtab`, and `.DVP.overlay...` metadata with ELF load range and `DATA.DF` size context
- Recorded confirmed safe metadata in `research/dvp/ico-usa-dvp-overlay-metadata.md`
- Identified 12 overlay table entries and 12 overlay string-table entries

### [x] [SQUAD-TOOLING | rev.007.6 | 2026-05-12]
Targeted DATA.DF scans around DVP tokens

- Extended `tools/data-df-index/` with repeatable `--target-offset` scans
- Ran targeted windows around DVP numeric tokens
- Recorded confirmed safe metadata in `research/data-df/ico-usa-data-df-dvp-targeted-scan.md`
- Found no simple local offset table or fixed-record candidates around tested DVP tokens

### [x] [SQUAD-TOOLING | rev.007.7 | 2026-05-12]
Executable reference scan for DATA.DF and DVP tokens

- Added `tools/exe-ref-index/` metadata-only exact reference scanner
- Scanned embedded `SCUS_971.13` for `DATA.DF`, `DFDATAS`, DVP terms, and DVP numeric constants
- Recorded confirmed safe metadata in `research/exe-refs/ico-usa-scus-97113-data-df-dvp-references.md`
- Confirmed direct `DATA.DF` and `DFDATAS` string references in executable data sections

### [x] [SQUAD-TOOLING | rev.007.8 | 2026-05-12]
MIPS split-immediate pattern scan for DATA.DF and DFDATAS addresses

- Added `tools/mips-immediate-scanner/` metadata-only MIPS split-immediate pattern scanner
- Scanned embedded `SCUS_971.13` for lui/addiu patterns referencing known virtual addresses
- Recorded confirmed safe metadata in `research/exe-refs/ico-usa-scus-97113-mips-immediate-patterns.md`
- Found 8 pattern matches for addresses 0x00556a10, 0x00556a20, 0x006127e8, and 0x00633b68
- DATA.DF at 0x00556a28 had no split-immediate patterns found in .text section

### [x] [SQUAD-TOOLING | rev.007.9 | 2026-05-12]
ELF symbol table analysis (stripped executable)

- Added `tools/elf-symbol-scan/` metadata-only ELF symbol table scanner
- Scanned embedded `SCUS_971.13` for .symtab, .dynsym, and dynamic linking structures
- Recorded confirmed metadata in `research/elf/ico-usa-scus-97113-symbol-table-analysis.md`
- Confirmed executable is completely stripped: no .symtab, no .dynsym, no PT_DYNAMIC

### [x] [SQUAD-TOOLING | rev.008 | 2026-05-12]
MIPS function prologue scan

- Added `tools/mips-prologue-scan/` metadata-only MIPS function prologue scanner
- Scanned embedded `SCUS_971.13` for addiu $sp patterns
- Recorded confirmed metadata in `research/elf/ico-usa-scus-97113-mips-prologue-scan.md`
- Found 3,991 function prologues with 71 unique stack sizes in .text section

### [x] [SQUAD-TOOLING | rev.009 | 2026-05-12]
Function reference correlation

- Added `tools/function-ref-correlator/` metadata-only function reference correlator
- Correlated prologue and immediate scan data to identify 7 functions with DATA.DF/DFDATAS references
- Recorded confirmed metadata in `research/elf/ico-usa-scus-97113-function-reference-correlation.md`
- Identified function addresses: 0x001321c8, 0x00132630, 0x00132ff0, 0x00185ca8, 0x0019fb34, 0x001a0a38, 0x0023d468

### [x] [SQUAD-TOOLING | rev.010 | 2026-05-12]
Call graph analysis

- Added `tools/mips-call-graph/` metadata-only MIPS call graph analyzer
- Scanned for jal/jalr instructions targeting the 7 known functions
- Found 15 calls from 13 unique caller functions
- Recorded confirmed metadata in `research/elf/ico-usa-scus-97113-call-graph-analysis.md`

---

## Revision Signatures

| Revision | Date | Squad | Summary |
|----------|------|-------|---------|
| rev.096 | 2026-05-19 | SQUAD-RUNTIME | Offline analysis of Rev.095 capture: main caller still dominates, second caller absent in this session, fast path remains inferred |
| rev.094 | 2026-05-19 | SQUAD-RUNTIME | Halfword runtime session: second caller at 0x0016828C observed, traces up to 26 cells, fast path 0x00166DFC still needs direct probe |
| rev.093b | 2026-05-19 | SQUAD-RUNTIME | Halfword entry probe validated 0x00166BB0 as active hot-path rasterizer before callback dispatch |
| rev.093 | 2026-05-19 | SQUAD-ARCH | Three investigations resolved: mask_set=ShockRequestBox_RequestCancel, dispatch table=compile-time .data, halfword writer still unresolved at this revision; later superseded by Rev.093b/Rev.094 |
| rev.069 | 2026-05-17 | SQUAD-RUNTIME | VU0 ring-buffer packet builder, kick stub, halfword table writers, alternate constants |
| rev.070 | 2026-05-17 | SQUAD-RUNTIME | Callers of 0x166028 (main loop, scene init, entry iter), 404-byte entity table, debug rodata table |
| rev.074 | 2026-05-17 | SQUAD-RUNTIME | Runtime session (9.1M events): slot 0 dead, slot 12 most active, alt_impl unused, VU0 kick gameplay-only, 58% match rate, 615 contexts/20 live entities, GP=0x6388F0 verified |
| rev.075 | 2026-05-17 | SQUAD-RUNTIME | Init_fn identification (0x1C3760=cloth_sys, 0x1F2370=cloth_tramp 5-mode, 0x17D128=env_effect), callback dispatch 0x13F9D0 (two-phase 8-bit mask + typed IDs 0x13-0x1B), cb_dispatch2 0x13FC00 (0x281AB0 table), ASM-HANDLER full analysis (BOY/GIRL/ENEMY1/WOODBOX0/AP1 — 15 functions) |
| rev.077 | 2026-05-17 | SQUAD-ARCH | Final static analysis: 8-step scene loader (kanban.c, GP=0x27A7A8). Descriptor table 0x2A31B8 (68 entity types, full structure). BARREL uses 0x1D3A30 (ROPE gap RESOLVED). Entry table 0x2A4C48 (512 spawns). Slot0 callback 0x168DA8 (no filter). VU0 kick 0x117768 = linked-list queue (NOT VU0). 1032 GP offsets mapped. Debug table 0x613E00 (47 debug entries, 0x168650=CollisionOldProc). Resource check 0x17B230 (bitmap). Wait/yield 0x203AA0 (syscall50). HUD pool 0x4Dxxxx debug display. GP data map consolidated. |
| rev.079 | 2026-05-18 | SQUAD-RUNTIME | Runtime session at windmill (14M events): slot 1 most active (45.7% vs slot 12 at 37.1%), 12 callbacks fully mapped, mask_set 0 gameplay hits (confirmed in 2nd session), slot 0 dead (2nd session), halfword writers 0 hits, alt_selection 0 hits, GP=0x6388F0 reconfirmed |
| rev.084 | 2026-05-18 | SQUAD-RUNTIME | Extended runtime session (43.8M events, ~122 min): entrance → windmill → cutscene → 3rd+ area. Slot 5 fired first time (triplet guard). All rare probes ZERO across 3rd independent session. 1,913 unique entity contexts. Cutscene period: 100% slot 12 (Group 1 suspended). Zone fingerprints confirmed: slot ratio shifts per area. Slot 0 root cause found (static analysis: 0 `addiu a1,0+JALR` sites). Total coverage: 66.9M events across 3 sessions. |
| rev.085 | 2026-05-18 | SQUAD-RUNTIME | Death validation: user jumped off cliff. mask_set = 0 hits even during death. Death zone = 100% slot 12 (Group 2 only), identical to cutscene. Confirms mask_set is I/O system (ShockRequestBox_RequestCancel), not gameplay death callback. |
| rev.086 | 2026-05-18 | SQUAD-ARCH | Final static analysis: descriptor +0x60 = behavior_fn (NOT vtable). Group A=0x202A60 (main chars), Group B=0x23D660 (props). Env effect table = 395 entries × 0x30 type-to-type mapping (NOT spatial zones). cb_routine4 +0x5C = no-op stubs (never called). VBlank counter 0x274EC0 = IOP-driven via SIF (no .text writer). GIRL=DEVIL_GI confirmed. |
| rev.091g | 2026-05-18 | SQUAD-TOOLING | boy_hC 76.64% via li expansion + ori→addiu normalizer + constant fixes (model addresses, tag, line). Zero regressions. |
| rev.091f | 2026-05-18 | SQUAD-TOOLING | boy_hB near-exact (97.06%) via li.s pseudo-op expansion and GP-relative resolution in scorer normalizer. Zero regressions, 8 exact matches intact. |
| rev.091h | 2026-05-18 | SQUAD-TOOLING | enemy1_hC 57.77% (+8.74%) via scene_obj elimination. extract_function_body bug fix (* prefix filtering). No regressions. |
| rev.076 | 2026-05-17 | SQUAD-ARCH | Post-runtime consolidation: 28 init_fn classified (6 groups). 17-slot table fully mapped. mask_set only uses bit 0. 404-byte = stage config. Halfword table = spatial hash. VU0 "kick" = COP2 macro utility. Two independent entity systems. ICO-decomp cross-ref. |
| rev.073 | 2026-05-17 | SQUAD-RUNTIME | Main loop dispatch chain (12 steps), corrected callback masks (bits 28-31), secondary pointer table (0x00633D30), struct field maps (80B/112B), linked-list flow with pre-multiplied offsets |
| rev.072 | 2026-05-17 | SQUAD-RUNTIME | Room init callbacks corrigidos: 19 function pointers reais (offset +0x174 absoluto), tabela de descritores (68 entries), tabela de entries (512 entries), instrucao 0x1AF954 = mult (dead code), 0x00143290 processa inner structs (nao callback) |
| rev.071 | 2026-05-17 | SQUAD-RUNTIME | 5-way consolidation: 404-room table (32 rooms, callback idx 0x4B), halfword grid rasterization (32x32), slot table stride 0x10 (17 entries, 14 callbacks), Group1/2 templates disassembled, main loop 0x101C80 dispatch chain documented |
| rev.091i | 2026-05-18 | SQUAD-TOOLING | Infrastructure fixes: include paths for extract_function_body, `ico_u8` typedef, `--whole-file` for all functions, score_all.py simplified. Compiler flags exploration across 7 flag sets × 38 functions (best per function documented in .local/flag_exploration_results.json). Systematic offset analysis: 148 unique offsets mapped across 25 handler functions, backbone confirmed (+348=entity_state, +2048=work_area). fn_1CE5F8 confirmed 100% with --whole-file. Current scores: 12 perfect, 26 partial/LOW, 0 compile errors. |
| rev.001 | 2026-05-12 | SQUAD-ARCH | Initial strategic planning and prompt workflow |
| rev.002 | 2026-05-12 | SQUAD-ARCH | Project retargeted to ICO Reconstruction |
| rev.003 | 2026-05-12 | SQUAD-ARCH | Public README created for community collaboration |
| rev.004 | 2026-05-12 | SQUAD-ARCH | README merged with repository template structure |
| rev.005 | 2026-05-12 | SQUAD-ARCH | ICO wallpaper added to README |
| rev.006 | 2026-05-12 | SQUAD-ARCH | Minimal GitHub folder structure added |
| rev.006.1 | 2026-05-12 | SQUAD-ARCH | Documentation cleanup and base research guides |
| rev.007 | 2026-05-12 | SQUAD-ARCH | Architectural analysis A-D for ICO |
| rev.007.1 | 2026-05-12 | SQUAD-TOOLING | Local metadata tooling foundation |
| rev.007.2 | 2026-05-12 | SQUAD-TOOLING | Local BIN/CUE disc index and initial observation |
| rev.007.3 | 2026-05-12 | SQUAD-TOOLING | Local ELF metadata index and initial observation |
| rev.007.4 | 2026-05-12 | SQUAD-TOOLING | Local DATA.DF metadata triage |
| rev.007.5 | 2026-05-12 | SQUAD-TOOLING | Local DVP overlay metadata correlation |
| rev.007.6 | 2026-05-12 | SQUAD-TOOLING | Targeted DATA.DF scans around DVP tokens |
| rev.007.7 | 2026-05-12 | SQUAD-TOOLING | Executable reference scan for DATA.DF and DVP tokens |
| rev.007.8 | 2026-05-12 | SQUAD-TOOLING | MIPS split-immediate pattern scan for DATA.DF and DFDATAS addresses |
| rev.007.9 | 2026-05-12 | SQUAD-TOOLING | ELF symbol table analysis (stripped executable) |
| rev.008 | 2026-05-12 | SQUAD-TOOLING | MIPS function prologue scan |
| rev.009 | 2026-05-12 | SQUAD-TOOLING | Function reference correlation (7 functions identified) |
| rev.010 | 2026-05-12 | SQUAD-TOOLING | Call graph analysis (13 callers identified) |
| rev.011 | 2026-05-12 | SQUAD-RUNTIME | Environment setup for disassembly (Ghidra + PCSX2) |
| rev.012 | 2026-05-12 | SQUAD-RUNTIME | Function disassembly analysis |
| rev.013 | 2026-05-12 | SQUAD-RUNTIME | Library calls analysis (top 25 functions) |
| rev.014 | 2026-05-12 | SQUAD-RUNTIME | PS2 SDK function analysis (leaf functions) |
| rev.015 | 2026-05-12 | SQUAD-TOOLING | First PoC: string extraction (81 .gcm files found) |
| rev.016 | 2026-05-12 | SQUAD-RUNTIME | Video/rendering functions identified |
| rev.017 | 2026-05-12 | SQUAD-TOOLING | Second PoC: string modification (NULL.gcm -> NULL0000) |
| rev.018 | 2026-05-12 | SQUAD-TOOLING | Multiple string modifications tested (title.gcm, logo.gcm, sacrifice.gcm) |
| rev.019 | 2026-05-13 | SQUAD-RUNTIME | Static analysis - state resolver caller context (146 callers, 14 clusters) |
| rev.020 | 2026-05-13 | SQUAD-RUNTIME | UI string context and caller analysis |
| rev.021 | 2026-05-13 | SQUAD-RUNTIME | Continue menu string deception + vtable call graph deep dive |
| rev.022 | 2026-05-13 | SQUAD-RUNTIME | Dispatcher ground truth — jump table address corrected to 0x00618fb0 |
| rev.023 | 2026-05-13 | SQUAD-RUNTIME | Dispatcher table resolution — confirmed 5-state dispatch model |
| rev.024 | 2026-05-13 | SQUAD-RUNTIME | Internal state block semantics — 5 blocks analyzed |
| rev.025 | 2026-05-13 | SQUAD-RUNTIME | Runtime-confirmed caller context (0x001d3a30) |
| rev.026 | 2026-05-13 | SQUAD-RUNTIME | ROPE record table context and descriptor structure |
| rev.027 | 2026-05-13 | SQUAD-RUNTIME | ROPE state block initializer analysis |
| rev.028 | 2026-05-13 | SQUAD-RUNTIME | State block provider contract (0x0013a0f8) |
| rev.029 | 2026-05-13 | SQUAD-RUNTIME | State block provider deeper static analysis |
| rev.030 | 2026-05-13 | SQUAD-RUNTIME | Provider caller survey — allocator pattern |
| rev.031 | 2026-05-13 | SQUAD-RUNTIME | Record callback dispatchers |
| rev.032 | 2026-05-13 | SQUAD-RUNTIME | Static callback follow-through |
| rev.033 | 2026-05-13 | SQUAD-RUNTIME | Node callback dispatch chain — store in node+0x1c |
| rev.034 | 2026-05-13 | SQUAD-RUNTIME | Callback signature and record selection |
| rev.035 | 2026-05-13 | SQUAD-RUNTIME | Entry table and descriptor correction — ROPE index fixed to 0x14 |
| rev.036 | 2026-05-13 | SQUAD-RUNTIME | Registration path survey — 5 callsites of 0x0013f7a8 |
| rev.037 | 2026-05-13 | SQUAD-RUNTIME | Remaining callers and ROPE registration gap — static options exhausted |
| rev.038 | 2026-05-13 | SQUAD-EXTERNAL | decomp.me scratch generation + CCC debug symbol scan (none found) |
| 2026-05-14 | 2026-05-14 | SQUAD-EXTERNAL | ICO-decomp cross-reference: cloth physics, source tree mapping |
| rev.039 | 2026-05-14 | SQUAD-EXTERNAL | Cloth domain correction — dispatcher/callback reclassified as cloth physics |
| rev.040 | 2026-05-14 | SQUAD-EXTERNAL | Static cloth domain reinterpretation + auxiliary helper mapping |
| rev.041 | 2026-05-14 | SQUAD-EXTERNAL | Cloth variant table 0x004d4188: 8 entries stride 0x14 |
| rev.042 | 2026-05-14 | SQUAD-EXTERNAL | Cloth variant field writers: 0x001d2858 confirmed, 0x001d1ad8 candidate, 0x001d390c discarded |
| rev.043 | 2026-05-14 | SQUAD-EXTERNAL | Cloth initializer arg source: 0x001d27a8 needs a1, [a1+0x30] origin open |
| rev.044 | 2026-05-14 | SQUAD-EXTERNAL | Staged callback path: no static explanation for a1 |
| rev.045 | 2026-05-15 | SQUAD-RUNTIME | Runtime plan for tomorrow — checkpoint before PCSX2 capture |
| rev.046 | 2026-05-15 | SQUAD-RUNTIME | Runtime capture: a1 source resolved — a1 = sp, caller = 0x001B7A74 |
| rev.047 | 2026-05-15 | SQUAD-RUNTIME | Descriptor Callback Runtime Model — modelo consolidado pós-runtime |
| 2026-05-15 | 2026-05-15 | SQUAD-TOOLING | External splat tooling experiments (SOTC survey, Rabbitizer, 3 splat experiments) |
| 2026-05-15 | 2026-05-15 | SQUAD-TOOLING | Full cloth cluster splat promotion — 22 functions isolated, YAML in splat/ |
| 2026-05-15 | 2026-05-15 | SQUAD-TOOLING | SDK/library recognition — 183 libkernl functions confirmed; libc absent from USA .text |
| 2026-05-15 | 2026-05-15 | SQUAD-TOOLING | decomp.me scratches regenerated + ee-gcc 15.2.0 toolchain installed |
| rev.048 | 2026-05-16 | SQUAD-EXTERNAL | C scratch model synthesis — fixed taxonomy, ico_ptr32 rule, 9-function status matrix |
| rev.049 | 2026-05-16 | SQUAD-EXTERNAL | Physics object type table — ROPE in static table, not dynamic registry |
| rev.050 | 2026-05-16 | SQUAD-ARCH | Cloth system anatomy — cloth_payload_init decompiled, 0x1B76F8 identified, entry table fully mapped |
| 2026-05-16 | 2026-05-16 | SQUAD-ARCH | First C source files committed (struct model + accessors + near matches) |
| 2026-05-16 | 2026-05-16 | SQUAD-ARCH | docs/data-model.md + docs/system-feature-flows.md created |
| rev.051 | 2026-05-16 | SQUAD-RUNTIME | Runtime session 3: 0 hits at 0x1D3A30 across ~90 min / 1419 events — refutes per-frame model |
| rev.052 | 2026-05-16 | SQUAD-ARCH | Five-way consolidation: descriptor map (68 entries), sister_callbacks, event_clear decomp, VU0 cloth physics |
| rev.053 | 2026-05-16 | SQUAD-RUNTIME | Handler decompilation wave 1: COP2 cloth (dist check + plane clip), WOODBOX0 init/update/post-init, ENEMY1 full lifecycle (19+ subfns), BOY warm/cold init paths |
| 2026-05-16 | 2026-05-16 | SQUAD-RUNTIME | 0x0024xxxx callers investigated: 0x240D40/0x240EA0 are object factories, excluded for ROPE callback |
| rev.054 | 2026-05-16 | SQUAD-RUNTIME | Handler decompilation wave 2: GIRL, QUEEN, BGA, AP1 — 12 funcoes disassembled, padrao hC/hB/hA confirmado em 7 entidades |
| rev.056 | 2026-05-16 | SQUAD-RUNTIME | Handler decompilation wave 3: BIRD, DEVIL_GI, ATTACKCH x2, BOSS_CTR — 12 handlers disassembled; descriptor index correction (WOODBOX0=17, BGA=30, AP1=61); hB dispatcher diversity confirmed (no unified dispatch); 13 entries with non-null init_fn |
| 2026-05-18 | 2026-05-18 | SQUAD-ARCH | BOY full handler decompilation: init_fn (0x153478), hC (0x1C1A98), hB (0x1C1DD8), hA (0x1C1F58), sub_1C1C48, sub_1C1EA8, sub_1C2098, sub_1C20A8, sub_1C2170 — all to NEAR-STRUCTURAL C in src/entity/boy.c. Updated docs/data-model.md and docs/system-feature-flows.md. |
| rev.057 | 2026-05-16 | SQUAD-RUNTIME | C models: cloth dispatcher (5-state FSM), clothSubForceApply (EE sin/cos force), ENEMY1 hC (80B constructor); WOODBOX0 hC ASM-HOLD; factory analysis integration |
| 2026-05-16 | 2026-05-16 | SQUAD-TOOLING | Compile test with ee-gcc 15.2.0: EXACT models structurally validated but NOT bit-identical (-mabi=eabi unsupported); modern toolchain confirms all offsets correct |
| 2026-05-16 | 2026-05-16 | SQUAD-TOOLING | Entity handler splat YAML (splat/SCUS_971.13.entity-handlers.yaml): 15 subsegments across 6 entities (BIRD, BOSS_CTR, ATTACKCH x2, WOODBOX0, ENEMY1) |
| 2026-05-16 | 2026-05-16 | SQUAD-ARCH | WOODBOX0 init data table at 0x4DF560 analyzed: 8 entries × 48B, model paths/params for crate parts; entry 4 contains object/sdf/st00a/model/0str16.p2o with float params |
| 2026-05-16 | 2026-05-16 | SQUAD-EXTERNAL | ICO-decomp thread.c cross-reference: thread.c decompiled C source found, struct IosThreadInfo layout (0x70 bytes), callback offset +0x1C maps to ThreadParam.entry |
| 2026-05-16 | 2026-05-16 | SQUAD-EXTERNAL | ICO-decomp fumi.h revealed: struct IosThreadInfo details (entry at +0x38, ThreadParam at +0x00), struct IosMsgQueue layout, memory partition constants |
| rev.059 | 2026-05-16 | SQUAD-ARCH | Table reader correction + callback chain full analysis: 0x1A48A0 is CODE not data (Rev.049 correction); descriptor handler layout fixed (+0x48=hA, +0x50=hB, +0x58=hC); scene loader 0x1B7D00 documented (4-phase init, calls 0x1B76F8); 0x13F3F0 (576B linked-list, stride 0x94) + 0x13F7A8 + 0x13F7D8 fully disassembled; GIRL cloth delegation explained (AI system creates cloth objects independently); data-model.md corrected |
| rev.060 | 2026-05-16 | SQUAD-RUNTIME | Consolidated hB analysis + callback pool correction: 0x1A6E28 is print/assert stub (NOT allocator, 32B save+return); 0x13F3F0 is POOL MANAGER (slot stride 0x94, gp-based, sorted doubly-linked per-entity list); 0x13D1B0 does heap_alloc(0x13A0F8) with t1 size/tag=173/"NodeCallback"; 0x13D3C8 jumps to kernel 0x100340 (queue insert); HOY hB (176 insns, 3 sub-fns: cloth+movement, collision/transform, head/weapon); ENEMY1 hB (142 insns, 3-state AI, counter÷10, 2-pass child sprite); GIRL hB (113 insns, BOY-similar + GIRL AI at 0x243AE8/0x243950); WOODBOX0 hC (286 insns, 400B alloc, 2 children); AP1 hC (367 insns, 6 heap_allocs, 4 children); runtime plan prepared; call graphs for 5 entities extracted |
| ~~rev.060~~ | 2026-05-16 | ~~SQUAD-TOOLING~~ | ~~Compiler correction: ee-gcc 2.9-991111-01 IS available on decomp.me via game-specific presets (not generic PS2). Backlog line 83 corrected. Test plan created at research/external/decompme-ee-gcc-991111-test-plan.md~~ **(subsumed into Pending Rev.060)** |
| rev.062 | 2026-05-16 | SQUAD-ARCH | GP-relative data map (11,547 accesses, 2,131 offsets): GP=0x006388F0 confirmed, 853 .sdata vars mapped, 40 entity type tags decoded, cloth VU0 function pointer in .sdata, world state block, seeker data cluster |
| rev.063 | 2026-05-16 | SQUAD-RUNTIME | VU0 cloth compute and writer functions: 0x001D9020 resolved (part of 0x1D8E40, 295 insns, VU0), 2 writers of gp[-18868] (0x1DFBC8 scene init + 0x1E00F8 entity init), cloth struct cluster gp[-18892..-18844] (48B, 12 slots), both writers outside clothAnimation.c range |
| rev.064 | 2026-05-16 | SQUAD-RUNTIME | Live scene init dispatch at 0x00166E10: cold path split (3 entry points), 400B stack, array iteration at 0x006AAC00, callback dispatch via [context+0x0C]; gp-25904 default state (0/0xFFFFFFFF); Rev.059 refuted (0x1B76F8/0x1B7D00 are DEAD CODE) |
| rev.088 | 2026-05-18 | SQUAD-ARCH | BARREL/ROPE handler decompilation: init_fn (0x166028), hA (0x1D2540/0x1D2550), hC BARREL (0x1D27A8), hC ROPE (0x1D3B28), cb_routine2 (0x1D3A30, ex-"ROPE callback"), fn_1D3BF0, fn_1D3D40, fn_1D3DD8 — 12 functions to NEAR-STRUCTURAL C in src/entity/barrel.c. Source files confirmed: src/item.c (hC assertions), src/fieldCollision.c (init_fn). Updated docs/data-model.md, docs/system-feature-flows.md, README.md. |
| rev.090 | 2026-05-18 | SQUAD-TOOLING | ee-gcc 2.9-991111-01 downloaded from decomp.me GitHub (github.com/decompme/compilers) and installed locally at toolchain/ee-gcc2.9-991111-01/bin/ee-gcc. Local compilation pipeline confirmed working with flags `-mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2`. Scoring tool created at tools/ee_gcc_compile.py. fn_1CE5F8 decompilation improved: frame 0x30 (was 0x40), delay slot nops inserted via asm barriers at 3 points (beq + 2 jals), 37 insn count matches target, all control-flow nops match. Ready for decomp.me v14 submission. |
| rev.091 | 2026-05-18 | SQUAD-TOOLING | Fixed 28 compile errors across all entity/cloth .c files for ee-gcc 2.9 (C89 mode). Changes: added u64 typedef (types.h); replaced C99 compound literals in enemy1.c (5 occurrences); fixed void→correct return types for sub_1C8478, sub_109F10, sub_105278, sub_103D50, sub_10D180 (boy.c) and sub_1C05A8 (barrel.c); fixed C89 declaration ordering in cloth/near_matches.c (3 blocks) and entity/near_matches.c (3 functions); fixed sub_1F2148/sub_12A618 return types. Fixed --size arg to accept hex in ee_gcc_compile.py. Applied sub_105F20 3rd arg and sub_1D4B40 2-arg signature. Score_all.py now runs all 37 functions with zero compile errors (was 28 errors). decompme_submit.py: normalized register names (ABI→numeric), hex→decimal immediates, jr $31→j $31. |
| rev.091i | 2026-05-18 | SQUAD-TOOLING | Include path fix (-I flags in compile_c_to_asm), ico_u8 typedef added to types.h, score_all.py simplified (always --whole-file), fn_1CE5F8 confirmed 100% with whole-file. Compiler flags exploration (7 flag sets, 38 functions): G0_O2 best for most; fn_1D3BF0 reaches 50.62% with -fno-schedule-insns; barrel_init 17.14% with -Os. Systematic offset analysis: 148 unique offsets across 25 handler functions; backbone confirmed (0x15C=entity_state, 0x800=work_area). 12 perfect, 26 partial/LOW, 0 compile errors. |

---

## Squad Definitions

| Squad | Focus Area |
|-------|------------|
| SQUAD-ARCH | Architecture, documentation, project structure |
| SQUAD-RUNTIME | Runtime environment, extraction, disassembly |
| SQUAD-TOOLING | Scripts, parsers, validation tools |
| SQUAD-GAMEPLAY | Gameplay systems, actors, events |
| SQUAD-QA | Quality assurance, validation, testing |

---

## Operating Rules

1. When starting a task: move to "In Progress"
2. When completing: mark as `[x]` and move to "Completed"
3. When identifying new work: add to "Pending"
4. All completed items remain in "Completed" section
5. Update `docs/architecture-log.md` when documenting features

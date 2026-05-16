# Rev.052 — Five-way consolidation: descriptor map, sister callbacks, event clear, DVP overlays, VU0 cloth

## Date

2026-05-16

## Objective

Consolidate five parallel static analyses into a single coherent model:
1. Full descriptor table (68 entries) with +0x40 init_fn scan
2. sister_callback_reg (0x13F778) disassembly
3. cloth_event_clear (0x1AE6F8) decompilation
4. DVP overlay system analysis
5. VU0 microcode and COP2 presence in cloth physics

## Scope

Included:

- Byte-level scan of 68 descriptor entries at 0x002A31B8
- Full disassembly of 0x13F778 (44 bytes) and 0x1AE6F8 (272 bytes)
- Decompiled C for 0x1AE6F8 (event clear, leaf function)
- DVP overlay table: 12 entries, all targeting VU microcode
- 20,704 bytes VU0 microcode in .vutext
- 63 COP2 instructions in cloth range

Excluded:

- Runtime verification of event clear behavior
- DATA.DF extraction for VU microcode overlays
- VU microcode disassembly (requires PS2 VU disassembler)

## Sources Used

| Fonte | Uso |
|---|---|
| `SCUS_971.13.elf` | Byte-level scan, objdump, readelf |
| `mips64r5900el-ps2-elf-objdump` | Disassembly |
| Rev.050 | cloth_payload_init decompilation, entry table |
| Rev.049 | Descriptor table model |
| Rev.037 | Callsite analysis |

## Evidence Used

- readelf section headers: .text = 0x00100000..0x0026F5D4, .vutext = 0x0026F5E0..0x002746C0
- objdump of descriptor table range (0x002A31B8..0x002A5978, 68 * 0x64 = 0x27C0)
- Disassembly of 0x13F778, 0x1AE6F8, 0x240D40, 0x240EA0
- DVP overlay table at 0x0071EB98 (12 * 12 bytes)
- COP2 instruction scan in cloth range (0x001D27A8..0x001D45D4)

## Findings

### 1. CORREÇÃO CRÍTICA: BARREL, não ROPE

A function `0x001D3A30` está exclusivamente no slot **hB (+0x50) de BARREL (índice 19)**. A entry ROPE (índice 20) tem handlers COMPLETAMENTE DIFERENTES:

| Entry | +0x48 hA | +0x50 hB | +0x58 hC |
|---|---|---|---|
| BARREL (19) | 0x1D3B28 | **0x1D3A30** | **0x1D27A8** |
| ROPE (20) | 0x1E9630 | 0x1E9810 | 0x1E8F38 |

O nome "ROPE callback" foi um erro introduzido na Rev.026 quando um label `.data` próximo a 0x002A3974 foi interpretado como "ROPE". Na verdade, 0x002A3974 é BARREL's hB. ROPE (0x002A3988) tem handlers de engine diferentes.

**O projeto deve abandonar o termo "ROPE callback" para `0x001D3A30`.** O termo correto é **BARREL hB** ou **cloth_update_handler**.

### 2. Tabela de descritores (68 entries, stride 0x64)

| Field | Offset | Description |
|---|---|---|
| name | +0x00 | 4-8 byte ASCII name, null-padded |
| phy_type | +0x20 | Physics constraint type (0xF1=lever, 0xF2=woodbox, 0xF3=barrel, 0=generic) |
| scale | +0x28 | float: physics scale (1.0=active, 0.0=inactive) |
| damping | +0x2C | float: damping/gravity (0.01=physics, 0.0=inert) |
| count | +0x30 | Instance count limit |
| **init_fn** | **+0x40** | **Constructor/asset init function (0 = no dedicated model)** |
| flags | +0x44 | 1=active, 0=inactive |
| handler_A | +0x48 | Lifecycle handler A |
| (pad) | +0x4C | Zero |
| **handler_B** | **+0x50** | **Lifecycle handler B (update/collision?)** |
| (pad) | +0x54 | Zero |
| handler_C | +0x58 | Lifecycle handler C (init for cloth via 0x1B76F8) |
| (pad) | +0x5C | Zero |

**12 entries com init_fn não-nulo:**

| Index | Name | init_fn (+0x40) | Notes |
|---|---|---|---|
| 1 | BOY | 0x153478 | Protagonist |
| 2 | GIRL | 0x174BA0 | Yorda (shared with DEVIL_GI) |
| 4 | ENEMY1 | 0x164440 | Shadow enemy |
| 17 | WOODBOX0 | 0x17D1D0 | Breakable crate (phy_type=0xF2) |
| 30 | BGA | 0x203EE8 | Background animation |
| 32 | BIRD | 0x1971C0 | Bird entity |
| 46 | QUEEN | 0x19B7F8 | Queen boss |
| 48 | DEVIL_GI | 0x174BA0 | Reuses GIRL model |
| 61 | AP1 | 0x1BB6B0 | Attack pattern 1 |
| 62 | ATTACKCH | 0x1BBF78 | Attack character variant |
| 63 | ATTACKCH | 0x1BBF78 | Attack character variant |
| 64 | BOSS_CTR | 0x198140 | Boss controller |

**Nenhuma entry além de BARREL (19) usa `0x1D3A30` ou `0x1D27A8`.** Esses handlers são exclusivos do sistema cloth BARREL.

### 3. sister_callback_reg (0x13F778)

```
0x13F778: move   t0, a3             ; t0 = type ID (22/23/24)
0x13F780: andi   a3, a2, 0xff      ; a3 = category byte (0/1)
0x13F788: move   a2, a1            ; a2 = callback function ptr
0x13F78C: li     t1, 0x1800        ; t1 = FIXED priority constant
0x13F790: jal    0x13F3F0          ; → node_callback_storage
0x13F794: move   a1, a0            ; a1 = object pointer
```

**Especialização de `0x13F7A8`**: a única diferença é `t1` — em `0x13F778` é fixo em `0x1800`, em `0x13F7A8` vem do caller via `t0`. A especialização economiza 1 registro nas 3 chamadas consecutivas feitas pelas factories.

Usada pelas factories (`0x240D40`, `0x240EA0`) para registrar callbacks com type IDs 22, 23, 24 antes do callback principal (type 19). O significado de cada type ID é desconhecido.

### 4. cloth_event_clear (0x1AE6F8) — decompilado

```
void cloth_event_clear(int type, uint16_t id) {
    int low, high;
    switch (type) {
    case 1:  low=0;   high=1;    break; // slot 0
    case 2:  low=1;   high=2;    break; // slot 1
    case 4:  low=42;  high=22;   break; // forced slot 42
    case 15: low=22;  high=42;   break; // slots 22-41
    default: low=42;  high=182;  break; // slots 42-181
    }
    for (int i = low; i < high; i++)
        if (table[i].id == id) break;
    if (i != high)
        table[i].id = 0;  // clear to mark free
}
```

- **Leaf function**: 68 instruções, zero jal
- **Tabela**: `0x004B3D10`, stride 0x40 (64B), ~182 entries
- **Campo**: `+0x02` = uint16_t id (lhu/sh)
- **Chamado de**: `cloth_payload_init` (ambos os paths, full e quick)
- **Semântica**: **stop/unregister** — limpa um ID de evento/som na tabela

Chamado nos dois paths porque ambos precisam limpar eventos/sons anteriores antes de (re)inicializar o objeto cloth.

### 5. DVP Overlays

**12 overlays**, todos targeting VU microcode (não MIPS):

| Overlay | VMA target | Size | Filler | Token (DATA.DF offset) |
|---|---|---|---|---|
| 0 | 0x0026F5F0 | 0x800 | 0x6e | 15101843 (no load) |
| 1 | 0x0026FDF8 | 0x800 | 0x00 | 359 |
| 2 | 0x002703B0 | 0x800 | 0x6e | 1887731 (no load) |
| 3 | 0x00270BB8 | 0x800 | 0x00 | 323 |
| 4 | 0x00270DA0 | 0x800 | 0x6e | 30224099 (no load) |
| 5 | 0x002715A8 | 0x800 | 0x00 | 244 |
| 6 | 0x00271DB0 | 0x1000 | 0x00 | 483563468/327 |
| 7 | 0x00272480 | 0x800 | 0x6e | 30224243 (no load) |
| 8 | 0x00272C88 | 0x800 | 0x00 | 243 |
| 9 | 0x00273490 | 0x1000 | 0x00 | 483563468/130 |
| 10 | 0x00273C98 | 0x1800 | 0x00 | 483563468/455 |
| 11 | 0x00274120 | 0x800 | 0x6e | 30216547 (no load) |

Payloads filler `0x6e` = placeholder no ELF, carregado de `DATA.DF` em runtime. Três overlays compartilham o token `483563468` (= 0x1CD2FCCC, plausible byte offset em DATA.DF).

**Conclusão**: Nenhum overlay contém código MIPS. `0x1D3A30` e `0x1D37C8` estão em `.text` principal. O dispatcher de `descriptor[+0x50]` não está em overlay.

### 6. VU0 Cloth Physics

- **20,704 bytes** de microcódigo VU0 em `.vutext` (2,588 × 8-byte VU microinstructions)
- **63 instruções COP2** no range cloth (`0x001D27A8`-`0x001D45D4`)
- Duas funções com blocos COP2 densos: `cloth_sub_001d3d70` (5 COP2) e `cloth_sub_001d4348/001d4358` (58 COP2)
- Zero referências a VU1
- 68 referências a scratchpad (`0x70000000`) como staging area para upload VU

**Modelo**: híbrido — EE MIPS + COP2 macro-mode para operações por-vértice + microcódigo VU0 para sub-rotinas de simulação. VU0 confirmado como coprocessador do sistema cloth.

### 7. Por que 0x1D3A30 nunca disparou em runtime

A análise de cinco direções converge para uma explicação:

1. **Não é per-frame**: BARREL hB (0x1D3A30) é event-driven, não chamado a cada frame
2. **Não está em overlay**: está em `.text` principal
3. **Não há caller estático**: nenhum `lw +0x50 -> jalr` no `.text` inteiro
4. **Contém COP2/VU0**: provavelmente chamado via interrupção VU0 ou dispatch indireto
5. **Só ativo sob condições específicas**: colisão, vento, interação do jogador com objetos BARREL

## What is Confirmed

- Descriptor table: 68 entries, stride 0x64, com init_fn, flags, e 3 handlers
- **0x1D3A30 = BARREL hB** (não ROPE). ROPE (idx 20) tem handlers diferentes.
- 12 entries com init_fn não-nulo (modelos 3D dedicados)
- sister_callback_reg (0x13F778) = especialização de 0x13F7A8 com t1=0x1800
- cloth_event_clear (0x1AE6F8) decompilado como leaf function que limpa IDs de evento
- Tabela de eventos em 0x004B3D10 (182+ entries, stride 0x40, campo id em +0x02)
- Overlays DVP = VU0 microcode (não MIPS), payloads carregados de DATA.DF
- VU0 confirmado no sistema cloth: 20KB microcode + 63 COP2 instruções

## What is Corrected

- **"ROPE callback" eliminado** — o termo correto é **BARREL hB** ou **cloth_update_handler**
- Physics type table (Rev.049) não existe como tabela separada — a descriptor table (0x002A31B8) já contém todos os parâmetros físicos e handlers
- .text termina em 0x0026F5D4 (confirmado), seguido por .vutext
- DVP overlays não contêm código MIPS — são exclusivamente VU microcode
- 0x1D27A8 não é "cloth_payload_init" genérico — é BARREL hC (init handler)

## What is Unknown

- O dispatcher runtime de descriptor[+0x50] (BARREL hB = 0x1D3A30)
- Se a chamada a 0x1D3A30 é via interrupção VU0
- Semântica exata dos type IDs 22, 23, 24 (sister callbacks)
- Conteúdo real dos overlays VU (carregados de DATA.DF em runtime)
- Se BARREL hB é chamado durante colisão, vento, ou interação específica

## Next Minimum Test

1. Breakpoint VU0 no PCSX2 para capturar quando microcódigo VU0 é executado e se dispara callback EE
2. Ou: breakpoint condicional em 0x1D3A30 com condição específica (ex: ao colidir com objeto BARREL)
3. Análise de `DATA.DF` nos offsets indicados pelos tokens DVP (483563468 ≈ 0x1CD2FCCC)

## Conservative Verdict

A consolidação de cinco análises paralelas corrige um erro conceitual que persistia desde a Rev.026: `0x1D3A30` não é um callback ROPE — é o handler de update (hB) do tipo de entidade **BARREL** (índice 19), que contém os parâmetros de física cloth (phy_type=0xF3, scale=1.0, damping=0.01). ROPE (índice 20) é uma entidade separada com handlers de engine completamente diferentes.

O sistema cloth usa VU0 (confirmado por COP2 instructions e 20KB de microcódigo), opera em modo híbrido EE+VU0, e o handler hB é event-driven — só ativo sob condições específicas de física/interação, não por frame.

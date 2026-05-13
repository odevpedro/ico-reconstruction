# rev.035 — Entry Table and Descriptor Correction

## Data

2026-05-13

## Objetivo

Executar a rodada "1-2-3-4-5" após Rev.034:

1. procurar quem escreve `+0x46` na tabela de entradas de stride `0x4c`;
2. mapear o layout inicial da tabela em `0x002a4c48`;
3. gerar uma tabela de índices de descriptors a partir de `0x002a31b8`;
4. comparar esses índices com os valores usados pelos callers de `0x001b76f8`;
5. corrigir a interpretação de Rev.034 onde necessário.

## Escopo

Incluído:

- busca estática por loads/stores em `+0x46`;
- dumps estruturais de `0x002a31b8` e `0x002a4c48`;
- índices de descriptor por label;
- entradas iniciais da tabela `0x4c`;
- correção da relação entre `ROPE`, índice e slots de callback.

Excluído:

- gameplay;
- runtime adicional;
- prova de entradas carregadas por dados de sala;
- renomeação definitiva de subsistemas.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev026-rope-record-table-context.md` | layout antigo baseado em record start |
| `research/elf/ghidra-rev034-callback-signature-and-record-selection.md` | hipótese a corrigir |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |

## Correção principal

Rev.034 tratou o record `ROPE` como índice `0x13` a partir da base `0x002a31b8`.

Isso está incorreto quando a tabela é alinhada pelo label.

A base:

```txt
0x002a31b8
```

aponta para o label do descriptor `NULL`, não para o mesmo "record start" usado em Rev.026.

Na convenção alinhada pelo label:

| Índice | Descriptor label VA | Label |
|---:|---|---|
| `0x13` | `0x002a3924` | `BARREL` |
| `0x14` | `0x002a3988` | `ROPE` |
| `0x15` | `0x002a39ec` | `CHAIN` |
| `0x16` | `0x002a3a50` | `FLEVER` |
| `0x17` | `0x002a3ab4` | `FLEVER_TRISTATE` |

Rev.026 used the record-start convention:

```txt
ROPE record start = 0x002a3934
ROPE label        = 0x002a3988
```

So:

```txt
0x002a3988 - 0x002a31b8 = 0x7d0
0x7d0 / 0x64 = 0x14
```

Corrected statement:

```txt
[entry +0x46] == 0x14 selects descriptor label ROPE.
```

not:

```txt
[entry +0x46] == 0x13
```

## Slot-offset correction

Rev.026 identified `ROPE +0x40 = 0x001d3a30` using the record-start convention:

```txt
record_start = 0x002a3934
record_start +0x40 = 0x002a3974 = 0x001d3a30
```

In the descriptor-label convention:

```txt
descriptor_label = 0x002a3988
descriptor_label -0x14 = 0x002a3974 = 0x001d3a30
```

Therefore, when `0x001b76f8` executes:

```asm
lw a1,+0x40(s7)
```

and `s7` is descriptor-label aligned, that is **not** the same slot as Rev.026's `record_start +0x40`.

This means the specific chain:

```txt
ROPE record_start +0x40 -> node +0x1c
```

is not proven by `0x001b7ac0`.

## 1. Writes to `+0x46`

The static scan found reads from `+0x46`, but no simple byte stores to `+0x46`:

```txt
lbu ...,+0x46(...)
```

Observed readers include:

| Address | Instruction |
|---|---|
| `0x001b774c` | `lbu a0,+0x46(s4)` |
| `0x001b79bc` | `lbu a1,+0x46(s4)` |
| `0x001b7abc` | `lbu v1,+0x46(s4)` |
| `0x001b7ad4` | `lbu v1,+0x46(s4)` |
| `0x001b7bc4` | `lbu a2,+0x46(v1)` |

Interpretation:

- `+0x46` appears to be read as an entry type/descriptor index;
- a direct `sb ..., +0x46(...)` writer was not found in this pass;
- the field may be static `.data`, bulk-copied, generated from loaded layout data, or written by a pattern not captured by the simple scan.

## 2. Entry table `0x002a4c48`

The table at `0x002a4c48` uses apparent stride:

```txt
0x4c
```

Fields observed in the first entries:

| Offset | Observed use |
|---:|---|
| `+0x24` | optional callback candidate passed to `0x0013f7a8` before descriptor fallback |
| `+0x2c` | numeric field, often small or `0x5eb` |
| `+0x30` | numeric field, often `0x15` |
| `+0x44` | halfword read by nearby functions |
| `+0x46` | descriptor/type index |
| `+0x47` | subtype/flag-like byte |
| `+0x48` | flag/mask-like word, often `0x0011ffff` |

Selected entries:

| Entry | VA | `+0x24` | `+0x46` | Label |
|---:|---|---:|---:|---|
| `0x00` | `0x002a4c48` | `0x00000000` | `0x1a` | `NONE` |
| `0x01` | `0x002a4c94` | `0x00000000` | `0x3c` | `KYOMI` |
| `0x03` | `0x002a4d2c` | `0x0017d128` | `0x1a` | `NONE` |
| `0x0c` | `0x002a4fd8` | `0x00000000` | `0x13` | `BARREL` |
| `0x0d` | `0x002a5024` | `0x00000000` | `0x13` | `BARREL` |
| `0x10` | `0x002a5108` | `0x00000000` | `0x02` | `GIRL` |
| `0x3b` | `0x002a5dcc` | `0x0020d940` | `0x1c` | `CAMERADUMMY` |
| `0x3f` | `0x002a5efc` | `0x0020dac8` | `0x1e` | `BGA` |

## 3. Descriptor indices

Selected descriptor indices from base `0x002a31b8`, stride `0x64`:

| Index | VA | Label | `label -0x14` | `label -0x1c` | `label +0x40` |
|---:|---|---|---:|---:|---:|
| `0x01` | `0x002a321c` | `BOY` | `0x00000000` | `0x00000000` | `0x00153478` |
| `0x02` | `0x002a3280` | `GIRL` | `0x001c1dd8` | `0x001c1f58` | `0x00174ba0` |
| `0x13` | `0x002a3924` | `BARREL` | `0x001e9950` | `0x001ea030` | `0x00000000` |
| `0x14` | `0x002a3988` | `ROPE` | `0x001d3a30` | `0x001d3b28` | `0x00000000` |
| `0x15` | `0x002a39ec` | `CHAIN` | `0x001e9810` | `0x001e9630` | `0x00000000` |
| `0x16` | `0x002a3a50` | `FLEVER` | `0x0018ecc8` | `0x0018f640` | `0x00000000` |
| `0x17` | `0x002a3ab4` | `FLEVER_TRISTATE` | `0x001bc1a8` | `0x001bc438` | `0x00000000` |

This table reconciles the two offset conventions:

```txt
Rev.026 record_start +0x40 == descriptor_label -0x14
Rev.026 record_start +0x38 == descriptor_label -0x1c
```

## 4. Comparison with callers of `0x001b76f8`

Rev.034 found three direct callers:

| Callsite | Pattern |
|---|---|
| `0x001b7d90` | loop over `a1 = 2..5` |
| `0x001b7df8` | loop over dynamic range |
| `0x001b7e6c` | loop using `lhu a1,+0x2(s0)` from table `0x004b3d10` |

These callers provide an entry index to `0x001b76f8`.

Inside `0x001b76f8`, the selected entry provides `+0x46`, which selects a descriptor label:

```txt
entry_index -> entry at 0x002a4c48 + entry_index * 0x4c
entry +0x46 -> descriptor index
descriptor = 0x002a31b8 + descriptor_index * 0x64
```

The first static entries include `BARREL` at descriptor index `0x13`, but no `ROPE` entry was found in a scan of entries `0x000..0x1ff`.

## 5. Reassessment of the `ROPE` chain

Confirmed:

```txt
ROPE descriptor label = 0x002a3988
ROPE callback at descriptor_label -0x14 = 0x001d3a30
0x001d3a30 executed in runtime and can call 0x001d37c8
0x0013fb70 calls node +0x1c callbacks with a0 = object/context
```

Not confirmed:

```txt
0x001b76f8 registers ROPE descriptor_label -0x14 into node +0x1c
```

Reason:

```txt
0x001b7ac0 loads descriptor_label +0x40, not descriptor_label -0x14.
```

Therefore, the current safe model is:

```txt
0x001d3a30 is a ROPE descriptor callback at descriptor_label -0x14.
0x0013fb70 is a real node +0x1c callback dispatcher.
But the exact static registration path from ROPE -0x14 to node +0x1c remains unresolved.
```

## README impact

The README should be corrected to avoid saying that `ROPE +0x40` is already linked to `node +0x1c`.

Safe public wording:

- confirmed dispatcher `0x001d37c8`;
- confirmed `ROPE` descriptor callback `0x001d3a30`;
- confirmed node callback dispatcher `0x0013fb70`;
- unresolved exact registration path from `ROPE` callback to that dispatcher.

## O que fica confirmado

1. Descriptor labels are aligned at `0x002a31b8 + index * 0x64`.
2. `ROPE` is descriptor index `0x14`, not `0x13`, in that convention.
3. `BARREL` is descriptor index `0x13`.
4. Rev.026's `record_start +0x40` corresponds to `descriptor_label -0x14`.
5. `0x001b7ac0` loads `descriptor_label +0x40`, not `descriptor_label -0x14`.
6. A simple static scan found no entry with `+0x46 == 0x14` in entries `0x000..0x1ff`.
7. A simple static scan found no direct `sb ..., +0x46(...)` writer.

## O que fica provável

1. `+0x46` is a descriptor/type index for `0x4c`-stride entries.
2. The entry table can select descriptor labels such as `BARREL`, `GIRL`, `BGA`, `SOBJ`, and others.
3. The previous `ROPE` registration hypothesis needs a different path than `0x001b7ac0`.

## O que fica possível

1. `ROPE` entries may be loaded dynamically from room/layout data rather than present in the static initial entries.
2. Another function may register `descriptor_label -0x14` callbacks into nodes.
3. `entry +0x24` may override descriptor fallback callbacks for some entries.

## O que permanece desconhecido

1. The exact static registration path for `0x001d3a30`.
2. Whether runtime ever uses `entry +0x46 == 0x14`.
3. Who writes or loads entry table fields when room data changes.
4. Whether `descriptor_label -0x14` has a global dispatcher distinct from `descriptor_label +0x40`.

## O que é descartado

1. `ROPE` as descriptor index `0x13`; that index is `BARREL`.
2. Treating `0x001b7ac0` as a proven registration of `ROPE +0x40` from Rev.026.
3. Treating `descriptor_label +0x40` and `record_start +0x40` as the same slot.

## Próximo teste mínimo

Sem gameplay:

1. Search for code paths that load `descriptor_label -0x14` or equivalent `record_start +0x40`.
2. Search for references to `0x001d3a30` through descriptor-relative scans, not only direct words.
3. Map functions around `0x00177304`, `0x0018202c`, `0x00190e88`, and related `+0x46` readers.
4. Investigate whether room/layout load code fills the `0x002a4c48` table dynamically.

## Veredito conservador

Rev.035 corrects the strongest overreach in Rev.034. The record/descriptor table is real, the `+0x46` descriptor index is real, and `0x0013fb70` remains a real node callback dispatcher. However, the specific path from `ROPE`'s confirmed callback `0x001d3a30` to `node +0x1c` is not proven by `0x001b7ac0`. The project should keep `0x001d3a30` as runtime-confirmed and structurally tied to `ROPE`, while treating its exact indirect dispatcher as still unresolved.

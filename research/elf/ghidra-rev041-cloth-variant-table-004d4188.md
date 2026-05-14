# rev.041 — Cloth Variant Table at 0x004d4188

## Data

2026-05-14

## Objetivo

Mapear estaticamente a tabela `0x004d4188`, identificada em Rev.040 como
estrutura consultada por `0x001d2bf0`, e determinar seu tamanho, campos
observados e relação com o cluster cloth-domain.

## Escopo

Incluído:

- tabela `0x004d4188`;
- referências estáticas à base `0x004d4188`;
- uso dos campos da tabela em `0x001d2bf0`;
- uso adicional em `0x001d3bf0`;
- delimitação entre a tabela e dados/strings adjacentes.

Excluído:

- runtime/emulador;
- `DATA.DF`;
- `.gcm`;
- extração ou interpretação de assets;
- nomes fortes para os IDs armazenados na tabela;
- matching PAL/USA definitivo.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev040-static-cloth-domain-reinterpretation.md` | ponto de partida: tabela consultada em `0x001d2bf0` |
| `research/ico-decomp-cross-reference-2026-05-14.md` | domínio `sugipon/src/clothAnimation` |
| `/tmp/ICO-decomp/config/symbol_addrs.txt` | símbolos PAL vizinhos |
| `/tmp/ICO-decomp/config/ICO-PAL.yaml` | source-file range de `clothAnimation` |
| `.local/extracted/SCUS_971.13.elf` | bytes locais USA |
| Capstone 5.0.7 | disassembly local |

## Evidência usada

Esta revisão usa:

- bytes da região `0x004d4188`;
- disassembly local dos ranges `0x001d3020..0x001d3120`,
  `0x001d3290..0x001d3360`, e `0x001d3be0..0x001d3ca0`;
- varredura estática por split-immediates que constroem `0x004d4188`.

Esta revisão não usa execução runtime.

## Delimitação da tabela

O primeiro dump amplo sugeria uma tabela longa de records `0x14`, mas isso é
incorreto. A região após `0x004d4228` já contém outro bloco de dados com
strings/caminhos.

A tabela relevante para o código analisado é:

```txt
base  = 0x004d4188
stride = 0x14
count = 8 entries
end   = 0x004d4228 exclusive
```

`0x004d4228` não deve ser tratado como continuação dessa tabela. Ele começa
um bloco de dados adjacente.

## Referências estáticas à base

Foram encontradas quatro construções estáticas da base `0x004d4188`:

| VA | Contexto | Uso observado |
|---:|---|---|
| `0x001d3030` / `0x001d303c` | dentro de `0x001d2bf0` | lê entry `+0x00` |
| `0x001d30d4` / `0x001d30d8` | dentro de `0x001d2bf0` | lê entry `+0x10` e testa bit 0 |
| `0x001d32b0` / `0x001d32bc` | dentro de `0x001d2bf0` | lê entry `+0x04`, depois entry `+0x10` |
| `0x001d3c40` / `0x001d3c4c` | dentro de `0x001d3bf0` | lê entry `+0x08` |

Isso coloca a tabela no cluster cloth-domain. Não foi observada referência
equivalente fora desse cluster nesta varredura pontual.

## Estrutura observada

O índice vem de:

```asm
lw    a1, 0x04(state_block)
mult  a1, 0x14
base  0x004d4188
```

Interpretação estrutural:

```c
struct cloth_variant_entry_candidate {
    int field_00;
    int field_04;
    int field_08;
    int field_0c;
    int flags_or_field_10;
}; // size 0x14
```

O valor `0x32f` aparece como sentinela em campos de ID. O código compara
campos carregados contra `0x32f` antes de executar chamadas auxiliares.

## Conteúdo da tabela

| Index | VA | `+00` | `+04` | `+08` | `+0c` | `+10 raw` | `+10 & 1` |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 0 | `0x004d4188` | sentinel `0x32f` | sentinel `0x32f` | `0x1ba` | sentinel `0x32f` | `0x00000000` | 0 |
| 1 | `0x004d419c` | sentinel `0x32f` | sentinel `0x32f` | sentinel `0x32f` | sentinel `0x32f` | `0x00000000` | 0 |
| 2 | `0x004d41b0` | `0x1bc` | `0x1bb` | `0x1bb` | `0x1bb` | `0x00000001` | 1 |
| 3 | `0x004d41c4` | `0x1be` | `0x1bd` | `0x1bd` | `0x1bd` | `0x00000001` | 1 |
| 4 | `0x004d41d8` | `0x1b8` | `0x1b8` | `0x1b8` | `0x1b8` | `0x00000001` | 1 |
| 5 | `0x004d41ec` | `0x1ac` | `0x1ac` | `0x1ac` | `0x1ac` | `0x00000001` | 1 |
| 6 | `0x004d4200` | sentinel `0x32f` | sentinel `0x32f` | `0x1b8` | sentinel `0x32f` | `0x00000000` | 0 |
| 7 | `0x004d4214` | `0x000` | `0x210` | `0x211` | `0x217` | `0x00000218` | 0 |

## Field usage

### Field `+0x00`

Used in `0x001d2bf0`:

```asm
0x001d3028: lw     a1,0x04(s6)       ; index = [state_block + 0x04]
0x001d302c: addiu  a2,zero,0x14
0x001d303c: addiu  v1,v1,0x4188      ; base = 0x004d4188
0x001d3044: lw     s3,0x00(v0)       ; entry +0x00
0x001d3048: beq    s3,0x32f,0x001d30d4
...
0x001d30ac: move   a0,s3
0x001d30b8: jal    0x001ebcd0
```

If `field_00 == 0x32f`, the call using that ID is skipped.

### Field `+0x04`

Used later in `0x001d2bf0`:

```asm
0x001d32a8: lw     a1,0x04(s1)       ; index = [state_block + 0x04]
0x001d32bc: addiu  s5,v1,0x4188      ; base = 0x004d4188
0x001d32c4: lw     s3,0x04(v0)       ; entry +0x04
0x001d32c8: beq    s3,0x32f,0x001d3330
...
0x001d331c: move   a0,s3
0x001d3324: jal    0x001ebc10
```

If `field_04 == 0x32f`, this second call is skipped.

### Field `+0x08`

Used in `0x001d3bf0`:

```asm
0x001d3c38: lw     v1,0x04(s0)       ; index = [state_block + 0x04]
0x001d3c48: mult   v1,0x14
0x001d3c4c: addiu  v0,v0,0x4188      ; base = 0x004d4188
0x001d3c5c: lw     s0,0x08(v0)       ; entry +0x08
0x001d3c60: beq    s0,0x32f,0x001d3cc8
```

This confirms that `field_08` is part of the same variant table and is used
outside `0x001d2bf0`, still inside the nearby cloth/chain cluster.

### Field `+0x0c`

No direct field-specific use was confirmed in this pass.

Because entries `2..5` mirror `+0x04`, `+0x08`, and `+0x0c`, the field likely
belongs to the same ID set, but its caller remains unknown.

### Field `+0x10`

Used as a flag source in `0x001d2bf0`:

```asm
0x001d30e8: lw     v0,0x10(v1)
0x001d30ec: andi   v0,v0,1
0x001d30f0: beqz   v0,0x001d3108
0x001d30f8: jal    0x001d12a8        ; a1 = 0x27 when bit set
...
0x001d3108: jal    0x001d12a8        ; a1 = 0x25 when bit clear
```

And later:

```asm
0x001d3338: lw     v0,0x10(v1)
0x001d333c: andi   v0,v0,1
0x001d3340: beqz   v0,0x001d3358
0x001d3348: jal    0x001d12a8        ; a1 = 0x2a when bit set
```

The low bit of `field_10` controls which internal event/signal wrapper path is
taken. Entries `2..5` have low bit set; entries `0`, `1`, `6`, and `7` do not.

## Relationship to state_block +0x04

Both `0x001d2bf0` and `0x001d3bf0` index the table with:

```txt
variant_index = [state_block + 0x04]
entry_ptr     = 0x004d4188 + variant_index * 0x14
```

This is the same field that `0x001d3a30` tests as a gate before calling
`0x001d37c8`:

```asm
0x001d3af4: lw   v1,0x04(a0)
0x001d3af8: xori v1,v1,1
0x001d3afc: bne  v1,zero,0x001d3b10
```

Therefore `[state_block + 0x04]` has at least two confirmed roles:

1. it is compared to `1` as a gate before dispatcher entry;
2. it is used as an index into `0x004d4188`.

This does **not** prove it is only a boolean. It is safer to describe it as a
mode/variant field whose value `1` specifically enables dispatcher entry in
`0x001d3a30`.

## Relationship to adjacent data

`0x004d4228` begins adjacent data. The bytes include readable paths such as:

```txt
object/sdf/boy/model/boymodel.p2c
object/sdf/boy/model/shdwmodel.p2s
object/sdf/boy/model/skelton.skb
NULL
```

These strings are used here only to delimit the end of the table at
`0x004d4228`. This revision does not analyze asset formats or asset contents.

## What Is Confirmed

1. `0x004d4188` is referenced statically inside `0x001d2bf0` and `0x001d3bf0`.
2. The relevant table has stride `0x14`.
3. The table contains 8 entries from `0x004d4188` through `0x004d4227`.
4. `0x004d4228` starts adjacent data and should not be treated as another
   `0x14` entry of this table.
5. `field_00`, `field_04`, and `field_08` are loaded as IDs and compared
   against sentinel `0x32f`.
6. `field_10 & 1` controls calls to `0x001d12a8` with IDs `0x25`, `0x27`,
   and `0x2a`.
7. The table index comes from `[state_block + 0x04]`.

## What Is Probable

1. `0x004d4188` is a cloth/chain variant table.
2. `field_00`, `field_04`, `field_08`, and probably `field_0c` are IDs for
   subresources, animation/motion slots, geometry helpers, or event targets.
3. `0x32f` is a sentinel meaning "no entry" or "skip this call".
4. The low bit of `field_10` is a variant flag that selects internal signal
   behavior.
5. `[state_block + 0x04]` is better named a variant/mode field than a boolean
   flag.

## What Is Possible

1. `field_0c` is used by a related function not covered by this pass.
2. Entry `7` may be a special variant because `field_10` contains `0x218`
   rather than a small boolean value, though the code currently only tests its
   low bit.
3. The IDs may correspond to motion, model, chain, or cloth resource indices,
   but that requires more cross-reference evidence.

## What Remains Unknown

1. The original name of the table.
2. The exact meaning of each ID column.
3. The full valid range of `[state_block + 0x04]` in runtime.
4. Who writes every possible value of `[state_block + 0x04]`.
5. Whether field `+0x0c` is used by another path.

## What Is Discarded

1. Treating the region after `0x004d4228` as more entries of the same table.
2. Treating `[state_block + 0x04]` as a simple boolean. It is used as a table
   index.
3. Treating `field_10` as only a boolean integer. The code tests bit 0, but
   entry `7` stores `0x218`, so other bits may carry meaning.
4. Assigning asset or gameplay names to the table entries from string adjacency
   alone.

## Next Minimum Test

Sem emulador:

1. Search for uses of `field_0c` by identifying other code paths that build
   `0x004d4188` and load offset `0x0c`, or by expanding the local cloth cluster
   disassembly.
2. Map writers of `[state_block + 0x04]` beyond `0x001d27a8`, especially paths
   that copy `[arg + 0x30]` into that field.
3. Cross-check whether the same 8-entry table exists in PAL if the PAL ELF
   becomes available locally.

Com emulador:

1. Capture `[state_block + 0x04]` at entry to `0x001d2bf0`, `0x001d3bf0`, and
   `0x001d3a30`.
2. Confirm which table indices appear in normal gameplay/load.

## Conservative Verdict

`0x004d4188` is a compact 8-entry, `0x14`-stride variant table used by the
cloth-domain cluster. It is indexed by `[state_block + 0x04]`, which means that
field should no longer be described as a simple gate/boolean. It is at least a
variant or mode field, with value `1` having a special role in `0x001d3a30` as
the condition for calling the dispatcher.

This improves the static model without resolving the callback registration gap.

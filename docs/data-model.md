# Data Model — ICO Reconstruction

> Documento vivo do modelo de dados reverso. Atualizado sempre que uma entidade for criada, alterada ou removida.
> **Ultima atualizacao:** 2026-05-17 (Rev.073 — Mask dos callbacks da slot table corrigida para bits 28-31 com valores corretos; tabela secundaria de ponteiros mapeada em 0x00633D30 (BSS); modelo de dados da halfword table expandido com fluxo de lista ligada; struct 80B Group 1 e struct 112B Group 2 documentadas)

---

## Indice

- [Visao Geral](#visao-geral)
- [Diagrama ER](#diagrama-er)
- [Entidades](#entidades)
- [Dominio de Valores](#dominio-de-valores)
- [Decisoes de Modelagem](#decisoes-de-modelagem)
- [Anexo: GP-Relative Global Data Map](#anexo-gp-relative-global-data-map)

---

## Visao Geral

Modelo de dados do sistema de objetos físicos do ICO, focado no subsistema cloth (corda/rope) e no pipeline de init de cena. O núcleo é composto por: hierarquia de 3 structs cloth (context → entity → payload), tabela de descritores (68 entries, stride 0x64), tabela de entries de sala (512 entries, stride 0x4C), a **tabela de slots** em 0x00282690 (17 entries, stride 0x10, 14 callbacks únicos) iterada pelo dispatcher 0x00166E10, e a **lista runtime de ponteiros** em 0x006AAC80. **Correção Rev.064:** O modelo anterior de scene loader (0x1B76F8/0x1B7D00) foi refutado — ambos são DEAD CODE. **Correção Rev.066-067:** O array de descritores previamente documentado como 0x006AAC00 é na verdade uma lista de ponteiros (stride 4) em **0x006AAC80**. A tabela de slots em 0x00282690 (não 0x006AAC00) fornece os callbacks e flags de configuração para cada slot 0-16.

- **Origem:** Engenharia reversa do ELF EE (MIPS R5900, ee-gcc 2.9-991111-01)
- **Nível de confiança:** EXACT ou NEAR-STRUCTURAL para todos os campos documentados (verificados por compilação C contra assembly original)
- **Nomenclatura:** Neutra (field_00, field_40, etc.) até que evidência adicional confirme semântica de gameplay

---

## Diagrama ER

```mermaid
erDiagram
    ENTRY_TABLE ||--|| DESCRIPTOR_RECORD : "+0x46 seleciona indice"
    SCENE_LOADER ||--|{ ENTRY_TABLE : "itera 0x1B76F8"
    CLOTH_CONTEXT ||--|| CLOTH_ENTITY : "entity em +0x15C"
    CLOTH_ENTITY ||--|| CLOTH_PAYLOAD : "payload em +0x800"
    CLOTH_PAYLOAD ||--|| STATE_BLOCK : "state_id em +0x48"
    CALLBACK_STORAGE_NODE ||--|| CLOTH_ENTITY : "callback em +0x1C"
    CALLBACK_REGISTRATION ||--|| CALLBACK_STORAGE_NODE : "cria nodes (stride 0x94)"
```

---

## Entidades

### scene_init_context

Contexto de inicialização de cena/entidade, passado como `a0` para o dispatcher `0x00166E10` (e seus cold paths `0x00167230`/`0x00167258`). Operado pela função principal de init de cena (400B stack, itera lista em 0x006AAC80).

**Offset base:** passado como argumento (`$a0`), salvo em `$s1`

**Nota Rev.066-067:** Os campos `desc_slot_0` a `callback_fn` (+0x00 a +0x0C) foram originalmente descritos como vindos do "desc array em 0x006AAC00", mas a análise Rev.066-067 mostra que o dispatcher carrega esses valores da **slot table** em `0x00282690` (indexada por `a1 * 0x10`). O campo `callback_fn` (+0x0C) é o `slot_table[N].callback`. A descrição abaixo mantém os offsets originais do contexto mas corrige a fonte: os dados vêm da slot table, não da runtime_ptr_list em 0x006AAC80.

| Campo | Offset | Tipo | Acesso | Descricao |
|-------|--------|------|--------|-----------|
| `desc_slot_0` | +0x000 | ico_ptr32 | lw | Slot 0 da slot table em 0x00282690 |
| `desc_slot_1` | +0x004 | ico_ptr32 | lw | Slot 1 |
| `desc_slot_2` | +0x008 | ico_ptr32 | lw | Slot 2 |
| `callback_fn` | +0x00C | ico_ptr32 | lw | **Callback dispatch** — JALR em 0x00167020(a0=s1, a1=s2, a2=s0), vindo de slot_table[N]+0x0C |
| `stream_16` | +0x010 | ico_ptr32 | — | Ponteiro para stream secundário (s1+16) |
| `stream_32` | +0x020 | ico_ptr32 | — | Transform/init stream (s1+32, s6) |
| `float_2C` | +0x02C | float | swc1 | Float (setado em 0x00166FE4, 0x00167030) |
| `check_ptr_74` | +0x074 | ico_ptr32 | lw | Ptr de matching (comparado com entity s2) |
| `match_idx_78` | +0x078 | ico_s32 | lw | Índice de matching (comparado com s0 loop) |
| `gate_ptr_7C` | +0x07C | ico_ptr32 | lw | Gate null-check para init |
| `sub_entity` | +0x080 | ico_ptr32 | lw | **Sub-entidade** (init = 0 via cold path, gp-25904 low) |
| `sub_idx` | +0x084 | ico_s32 | lw | **Índice sub** (init = -1 via cold path, gp-25904 high) |
| `data_ptr_88` | +0x088 | ico_ptr32 | lw | Ptr (+76 lido para dados) |
| `sub_ptr_8C` | +0x08C | ico_ptr32 | lw | Sub-struct +348 (path alternativo) |
| `sub_idx_90` | +0x090 | ico_s32 | lw | Índice (path alternativo) |
| `data_ptr_94` | +0x094 | ico_ptr32 | lw | Ptr (+96 dereferenciado) |
| `written_98` | +0x098 | ico_ptr32 | sw | Valor escrito de [+148+96] |
| `matrix_A0` | +0x0A0 | float[?] | — | Início matriz transform (0x002438B8) |
| `float_AC` | +0x0AC | float | swc1 | Float na matriz [s0+12] |
| `flag_B0` | +0x0B0 | ico_s32 | sw | Limpado pelo cold path 1 (0x00167234) |

**Evidencia:** Disassembly completo de 0x00166E10 (Rev.064), cold paths 0x00167230/0x00167258, confirmado por 14+6 JALRs via GP. Slot table confirmada via byte-level leitura (Rev.066-067).

---

### ~~desc_array_0x006AAC00~~ CORRIGIDO

**O label `desc_array_0x006AAC00` está INCORRETO (corrigido Rev.066-067).**  
Não há referência estática direta a `0x006AAC00` no código. A referência real vai para `0x006AAC80`.

### runtime_ptr_list_0x006AAC80

Lista de ponteiros de runtime para entidades/contextos. Iterada pelo dispatcher `0x00166E10` usando contagem em `gp-25896`.

**Tabela:** `0x006AAC80` (secao `.bss`)
**Iterador:** `0x00166E10` (função principal de dispatch)
**Count:** `gp-25896` = `0x006323C8` (resetado por 0x00166028, incrementado por entry)
**Max:** `< 0x100` (verificado por `slti` em 0x001660C8)
**Elemento:** 4 bytes (`index << 2`)
**Populado por:** `0x00166028` (build_runtime_pointer_list)

O dispatcher lê a lista assim:

```asm
0x00166EB8: lui      $a1, 0x6b
0x00166EC8: lw       $s2, -0x5380($a1)   ; s2 = *(0x006AAC80)
```

A leitura é de um ponteiro (primeira entry), não de um descritor completo. Cada entry é um ponteiro de 32-bit para uma estrutura de entidade/contexto.

**Callers de 0x00166028** (construtor da lista):
| Caller | Contexto |
|---|---|
| `0x00101ee8` | Cadeia principal de init: JAL 0x1AA098 → JAL 0x166028 → JAL 0x103370... |
| `0x001af974` | Init de entidade: carrega fn ptr de `[base+0x154]`, JALR, depois JAL 0x166028 |
| `0x001b7b50` | Init de subsistema: última chamada antes do epílogo |

Também alcançado via 14 wrappers GP-slot (gp-25856, slots 0-11).

**Relação com slot_table_0x00282690:** A lista runtime em 0x006AAC80 contém ponteiros para as entidades/contextos que serão processados pelo dispatcher. A slot table fornece a configuração de **como** processar (qual callback, qual grupo, com/sem guarda). As entradas da lista são itens de dados, não descritores de configuração.

**Relação com descriptor_table (0x002A31B8):** Desconhecida. A runtime_ptr_list em 0x006AAC80 contém ponteiros de entidades já criadas. A descriptor_table em 0x002A31B8 contém descritores de tipo (68 entries, stride 0x64) usados pelo sistema de criação de objetos.

---

### VU0 ring buffer (0x004C7710)

Buffer circular para pacotes VIF/VU0 usado pelo transform orchestrator 0x1D4A58. Cada entrada tem 16 bytes (8 data + 8 type tag).

**Tabela:** `0x004C7710` (BSS/runtime)

| Campo | Offset | Tipo | Descricao |
|-------|--------|------|-----------|
| `base` | +0x00 | u32 | Base/limit do buffer (usado para wrap-around) |
| `head` | +0x10 | ico_ptr32 | **Head pointer** — posicao atual de escrita (avanca 16 por push) |

**Formato do pacote (5 entradas por invocacao de 0x1D43F8):**

| Entry | Type tag | Data | Descricao |
|:-----:|:--------:|------|-----------|
| 1 | 0 | `*(gp-0x54C4)` | Header global (scene/entity context id) |
| 2 | 1 | 4 campos byte de struct B | Atributos de vertice/RGBA |
| 3 | 5 | 3 campos de struct A (matrix) | Vetor transform, upper 32 bits = 0xFFFFFFFF se `t0=-1` |
| 4 | 1 | 4 campos byte de struct D | Atributos de vertice/RGBA |
| 5 | 5 | 3 campos de struct C (matrix) | Vetor transform (tail-call), terminator se `t0=-1` |

**Push function:** `0x111918` — escreve sd(a1), sd(a0) no head, avança 16.
**Consumidor:** Codigo runtime em `0x3800C` (VIF uploader, fora do ELF).
**Acesso GP:** Apenas `lw -0x54C4($gp)` para o header value.

---

### VU0 kick stub (0x117C40)

Inline asm para upload VIF (VU0) com terminação via `J 0x3800C`.

| Endereco | Patrao | Tipo |
|:--------:|--------|:---:|
| `0x117C40` | 4× LUI (0xE74B/0xE64B/0xE54B/0xE44B) + ANDI + J | Full stub |
| `0x117C60` | 4× LUI + ANDI + J | Full stub (duplicate) |
| `0x117CE0` | 1× LUI + J | Truncated variant |

**VIF command pattern (4 registradores VF):**
- VF3 (`$v1`, LUI 0xE74B)
- VF27 (`$k1`, LUI 0xE64B)
- VF19 (`$s3`, LUI 0xE54B)
- VF11 (`$t3`, LUI 0xE44B)

**SQC2 block emparelhado em `0x117C80`/`0x1181EC`:** sqc2 $vf3/$vf11/$vf19/$vf27.

---

### Halfword table (0x006AB080, BSS)

Tabela runtime-populada de `uint16`, iterada por todos os 14 callbacks da slot table.
Populada por exatamente 2 escritores na funcao iteradora `0x00166C80` (dentro do mesmo range do dispatcher `0x00166E10`).

**Tabela:** `0x006AB080` (secao `.bss`)
**Index counter:** `gp-19396` = `0x00633D2C`
**Valor escrito:** `(a2 << 5) + t0` codifica coordenadas de grid 32x32
**Significado:** Cada halfword = `(row << 5) | col` onde row ∈ [0,31], col ∈ [0,31]

**Processo de populacao:** A funcao `0x00166C80` traca uma linha/ray atraves de um grid 32x32 e registra todas as celulas intersectadas como halfwords. Ambos a2 (row) e t0 (col) sao bounds-checked contra [0,31].

**Escritores:**

| Endereco | Contexto |
|:--------:|----------|
| `0x00166D1C` | Path A da funcao iteradora |
| `0x00166D78` | Path B (duplicata) |

**Contador = 30 leituras** no range 0x168xxx (todos os 14 callbacks consomem o contador).

### Consumo pelos callbacks (fluxo de lista ligada)

Todos os 14 callbacks compartilham este fluxo:

```txt
1. halfword = table[loop_counter]           # 16-bit index (0-1023)
2. ptr = pointer_array[halfword]            # from secondary_table+0x18 (G1) or +0x1C (G2)
3. For each entry in linked list at ptr:
   a. entry_val = *ptr (16-bit signed; <0 = chain end)
   b. struct_ptr = struct_array_base + entry_val  # entry_val is pre-multiplied by stride!
   c. Apply mask to struct_ptr->field_48 (G1) or field_60 (G2 slot 15)
   d. If passes: jal template (0x166258 G1, 0x1667E0 G2)
   e. ptr += 2 (next entry in list)
```

**Group 1 (w0=1, slots 0-11):** struct stride 0x50 (80 bytes), template 0x166258 (position/rotation proximity).
**Group 2 (w0=0, slots 12-16):** struct stride 0x70 (112 bytes), template 0x1667E0 (orientation matching).

Os valores de entry na lista ligada sao **byte offsets pre-multiplicados** pelo stride — nao indices crus. O `mult $v0, 0x50`/`mult $v0, 0x70` em cada callback e dead code (artefato de compilador O2).

### secondary_table_0x00633D30

Tabela secundaria de ponteiros populada em runtime, usada por todos os 14 callbacks para resolver halfword indices em structs.

**Endereco:** `0x00633D30` (.sbss+0x130, GP-0x4BC0)
**Tipo:** BSS — setada em runtime pelo dispatcher init (0x166E10)
**Carregada por:** `lw $a1, -0x4BC0($gp)` em todos os callbacks

A tabela tem duas metades (Group 1 e Group 2):

| Offset | Group 1 (w0=1) | Group 2 (w0=0) |
|--------|---------------|----------------|
| +0x10  | struct array base (stride 0x50=80) | — |
| +0x14  | — | struct array base (stride 0x70=112) |
| +0x18  | pointer array (indexed by halfword) | — |
| +0x1C  | — | pointer array (indexed by halfword) |

### 80-byte struct (Group 1)

| Offset | Size | Field |
|--------|------|-------|
| +0x00  | 2    | Linked list entry (signed byte offset; <0 = chain end) |
| +0x02  | 2    | Unknown |
| +0x08  | 4    | Index/link |
| +0x10  | 16   | Position data (consumed by Group1 template 0x166258) |
| +0x40  | 4    | Template data |
| +0x44  | 4    | Template data |
| +0x48  | 4    | **Flags** — tested by slot-specific mask (all G1 slots except 0) |
| +0x74  | 4    | Entity guard 1 (used by slots 4/5 with triplet guard) |
| +0x78  | 4    | Entity guard 2 |
| +0x7C  | 4    | Entity guard 3 |

### 112-byte struct (Group 2)

| Offset | Size | Field |
|--------|------|-------|
| +0x00  | 2    | Linked list entry |
| +0x60  | 4    | **Flags** — tested by slot 15 mask (`& 0x000F0000 == 0x00020000`) |

---

### room_entity_table_0x005F2F98

Tabela de entidades/contextos por sala/zona (world_state). Stride 404 bytes, 32 rows em `.data`.

**Base:** `0x005F2F98` (secao `.data`)
**Stride:** 0x194 (404 bytes)
**Rows:** 32 (indices 0-31)
**Header:** 32 bytes (0x00-0x1F), conteudo especifico da tabela
**Nome da sala:** offset +0x20 (row_base), 32 bytes ASCII null-padded
**Callback pointer:** offset +0x174 absoluto (= row_base + 0x154), u32 — function pointer pre-carregado (19 salas de gameplay tem ponteiros nao-nulos)

**Code read pattern (0x001AF948-0x001AF970):**
```
v1 = [0x0062CD40]               ; GP-28512, pre-multiplicado: world_state * 404
a0 = 404                        ; constant only for dead mult
a1 = 0x005F0000                 ; table high
mult v1, a0                     ; DEAD CODE — result unused
v0 = 0x005F2FB8                 ; row base (header + 0x20)
v0 = v0 + v1                    ; row_base + world_state * 404
v0 = *(v0 + 340)                ; load callback from row_base + 0x154
if v0 != 0: jalr v0             ; call room init callback
```

| Campo | Offset (rel row_base) | Offset (absoluto) | Tipo | Descricao |
|-------|----------------------|-------------------|------|-----------|
| `name` | +0x00 | +0x20 | char[32] | Nome ASCII da sala (ex: "logo", "sacrifice", "jail") |
| `field_00` | — | +0x00 | u32 | Flag/tipo (row 0=1, salas=0) |
| `field_04` | — | +0x04 | u32 | (row 0=0x0001001A, salas=2) |
| `field_08` | — | +0x08 | u32 | (sempre 1) |
| `view_dist` | — | +0x0C | float | Distancia de visao/fog (row 0=~0, salas=1500.0) |
| `...` | — | +0x10..+0x16F | — | (diversos campos) |
| `debug_type_id` | +0x134 | +0x154 | u32 | Valor 0x4B (75) para todas as salas (provavel debug/type ID) |
| `callback_ptr` | **+0x154** | **+0x174** | **ico_ptr32** | **Room init function pointer (19 non-null, ver tabela abaixo)** |
| `flag_158` | +0x158 | +0x178 | u32 | 1 |
| `flag_15C` | +0x15C | +0x17C | u32 | 1 |
| `param_160` | +0x160 | +0x180 | u32 | 1 |
| `param_164` | +0x164 | +0x184 | u32 | 1 |
| `param_168` | +0x168 | +0x188 | u32 | 0x56 (86) |
| `count_16C` | +0x16C | +0x18C | u32 | Contador (1 row 0, 2 rows 1-31) |
| `pad_170` | +0x170 | +0x190 | u32 | 0 |
| `float_178` | +0x178 | +0x198 | float | 0.5f |
| `float_17C` | +0x17C | +0x19C | float | 1.0f |
| `mode_180` | +0x180 | +0x1A0 | u32 | 2 |
| `room_specific_184` | +0x184 | +0x1A4 | u32 | Especifico por sala (ex: logo=0x94, NULL=0x2B9) |
| `room_specific_188` | +0x188 | +0x1A8 | u32 | 0 |
| `room_specific_18C` | +0x18C | +0x1AC | u32 | 0 |
| `room_specific_190` | +0x190 | +0x1B0 | u32 | Especifico por sala (ex: logo=0x7F, NULL=0x2B9) |

**CORRECAO Rev.072:** O campo `callback_ptr` (row_base + 0x154, absoluto 0x005F2F98 + 0x174) contem **function pointers reais**, nao indices. 19 salas de gameplay tem ponteiros pre-carregados no ELF. O valor 0x4B encontrado anteriormente estava em offset **+0x134** de row_base (= 0x005F2F98 + 0x154) e e um debug/type ID, nao o callback. O `mult` em 0x1AF954 e dead code (sem `mflo`), e a multiplicacao e feita upstream — a GP var `0x0062CD40` ja contem `world_state * 404` em runtime.

**Room init callbacks (19 non-null):**

| Room index | Room name    | Callback pointer |
|------------|--------------|------------------|
| 4  | jail         | 0x00231AC8 |
| 5  | warehouse    | 0x00234AB0 |
| 7  | proto        | 0x0020FA98 |
| 8  | troko        | 0x0022A838 |
| 9  | chandelier   | 0x00228198 |
| 10 | entrance     | 0x00239750 |
| 14 | shadows      | 0x00210D78 |
| 15 | windmill     | 0x0022B878 |
| 16 | plaza        | 0x00213B88 |
| 17 | stone        | 0x0022BFE8 |
| 19 | crest_L1     | 0x0021A4D0 |
| 20 | crest_L2     | 0x0021A6B8 |
| 21 | crest_L3     | 0x0021A980 |
| 22 | taki         | 0x00211780 |
| 23 | sluice       | 0x00225F68 |
| 25 | gondola      | 0x00237B78 |
| 26 | watertower   | 0x0022D8F8 |
| 28 | crest_R1     | 0x0021F828 |
| 29 | crest_R2     | 0x0021FA30 |
| 30 | crest_R3     | 0x0021FD20 |

Null callbacks: logo (title screen), sacrifice (intro?), ico_brigde, gate, gate2, grave, symmetry_L, symmetry_R, underground, cliff — provavelmente salas sem gameplay ou carregamento por overlay.

### Alternate implementation constants (0x55F260)

Blocos de 4 QWORDs em `.data` usados pelas implementações alternativas `0x169F80`/`0x16A058`.

**Alternate A (0x55F280, usado por 0x169F80):**

| VA | i32 lo | i32 hi | Significado provavel |
|:---|---:|---:|:---|
| 0x55F280 | 442 | 4 | count, base index |
| 0x55F288 | 1431024 | 2 | bone/object ID, subtype |
| 0x55F290 | 0 | 1431024 | zero, bone/object ID |
| 0x55F298 | 0 | 0 | zero sentinel |

**Alternate B (0x55F260, usado por 0x16A058):** 3 QWORDs zero + 1 com hi=388.

---

### gp-25904 default state

---

### gp-25904 default state

Valor de 8 bytes em `0x006323C0` usado como estado padrão para campos de entidade.

```c
// gp-25904 (0x006323C0):
uint8_t default_state[8] = { 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };
// = 0xFFFFFFFF00000000
```

| Metade (32-bit) | Valor | Uso |
|-----------------|-------|-----|
| Lower (offset 0) | 0x00000000 | [context+0x080] = null pointer (sub_entity init) |
| Upper (offset 4) | 0xFFFFFFFF | [context+0x084] = -1 (sub_idx init, "disabled" flag) |

**Carregado por:** Cold paths 0x00167230/0x00167258 via LDL/LDR unaligned (gp-25904).
**Armazenado em:** [context+128..135] via SDL/SDR unaligned store.

---

### slot_table_0x00282690

Tabela de configuração de 17 slots (indices 0-16) para o dispatcher `0x00166E10`. Cada slot tem stride 0x10 e define o callback e flags de processamento.

**Tabela:** `0x00282690` (secao `.data`)
**Stride:** 0x10 (16 bytes por slot, confirmado via `sll $a1, $a1, 4` em 0x00166E1C)
**Indexado por:** `a1 * 0x10` (slot index passado pelas wrappers)
**Acessado por:** dispatcher `0x00166E10` em `0x00166E24`
**Callbacks unicos:** 14 (em 17 slots; slots 8/9/16 reusam callbacks de slots 1/3/12)

| Campo | Offset | Tipo | Descricao |
|-------|--------|------|-----------|
| `w0` | +0x00 | ico_s32 | **Grupo**: 0=Group2 (orientacao), 1=Group1 (posicao/rotacao) |
| `w1` | +0x04 | ico_s32 | **Guarda**: 1=habilita triplet guard (`[ctx+0x74/78/7C]`) |
| `w2` | +0x08 | ico_s32 | **Flag extra**: armazenado em `sp+0xCC`, passado aos callbacks |
| `callback` | +0x0C | ico_ptr32 | **Função callback** (JALR em 0x00167020) — 14 funções únicas, 2 templates |

**Slots mapeados (Rev.067):**

| Slot | VA | w0 | w1 | w2 | Callback | Callback role (mask bits 28-31) |
|-----:|----:|:-:|:-:|:-:|--------:|---------------|
| 0 | 0x282690 | 1 | 0 | 0 | 0x168DA8 | Group1, sem mascara |
| 1 | 0x2826A0 | 1 | 0 | 0 | 0x168ED0 | G1, `0xF0000000==0 AND 0x000F0000!=0x00010000` |
| 2 | 0x2826B0 | 1 | 0 | 0 | 0x1692F0 | G1, mask idem, **a2=1** |
| 3 | 0x2826C0 | 1 | 0 | 0 | 0x169440 | G1, `0xF0000000==0` |
| 4 | 0x2826D0 | 1 | 1 | 0 | 0x169020 | G1, mask slot 1 + **triplet guard** |
| 5 | 0x2826E0 | 1 | 1 | 0 | 0x169190 | G1, mask slot 3 + **triplet guard** |
| 6 | 0x2826F0 | 1 | 0 | 0 | 0x1696C0 | G1, `0xC0000000==0x40000000` |
| 7 | 0x282700 | 1 | 0 | 0 | 0x169580 | G1, `0x30000000!=0` |
| 8 | 0x282710 | 1 | 0 | 1 | 0x168ED0 | (reusa slot 1) |
| 9 | 0x282720 | 1 | 0 | 1 | 0x169440 | (reusa slot 3) |
| 10 | 0x282730 | 1 | 0 | 0 | 0x169800 | G1, `0x70000000==0 AND 0xC0000000==0x80000000` |
| 11 | 0x282740 | 1 | 0 | 0 | 0x169968 | G1, `0xC0000000==0xC0000000` |
| 12 | 0x282750 | 0 | 0 | 0 | 0x169AA8 | Group2, sem mascara |
| 13 | 0x282760 | 0 | 1 | 0 | 0x169BD0 | Group2, **triplet guard** |
| 14 | 0x282770 | 0 | 0 | 0 | 0x169E58 | Group2, **a2=1** |
| 15 | 0x282780 | 0 | 0 | 0 | 0x169D18 | G2, mask `[+0x60] & 0x000F0000 == 0x00020000` |
| 16 | 0x282790 | 0 | 0 | 1 | 0x169AA8 | (reusa slot 12) |

**Funções chamadas pelos callbacks:**

| Funcao | Stack | Proposito |
|--------|-------|-----------|
| `0x166258` | 0xE0 + `$f20-$f24` | Setup de posicao/rotacao: matrizes e float ops em `+0x20`, `+0x40`, `+0x4C` |
| `0x1667E0` | 0x30, sem FPU | Check de orientacao: dot products, comparacao float contra zero |
| `0x243B60` | — | Matrix library, provavelmente `ApplyMatrix` PS2 SDK |

**Tabela de halfwords em 0x006AB080 (BSS):** Todos os 14 callbacks iteram esta tabela populada em runtime (uint16 entries). Ela mapeia IDs de objeto para indices em uma tabela de ponteiros. O bound do loop e o tamanho de elemento (`0x50` ou `0x70`) sao controlados por `w0`.

---

### cloth_context

Contexto de frame passado como `a0` para funções cloth.

**Offset base:** passado como argumento (registrador `$a0`)

| Campo | Offset | Tipo | Acesso | Descricao |
|-------|--------|------|--------|-----------|
| `pad_15C` | +0x000 | u8[0x15C] | — | Padding ate entity |
| `entity` | +0x15C | ico_ptr32 | lw | Ponteiro para cloth_entity |
| `entity_alt` | +0x160 | ico_ptr32 | lw | Segundo ponteiro entity (em 0x1D3D40) |
| `extra` | +0x16C | ico_ptr32 | lw | Ponteiro auxiliar (pode ser NULL) |
| `field_08` | +0x08 | ico_s32 | lw | Lido por cloth_payload_init em [s5+0x08] (provavel posicao X) |
| `field_0C` | +0x0C | ico_s32 | lw | Lido por cloth_payload_init em [s5+0x0C] (provavel posicao Z) |

**Evidencia:** 3 EXACT + 3 NEAR-STRUCTURAL matches (Rev.048), confirmado no disassembly de 0x1D27A8 (Rev.050)

---

### cloth_entity

Entidade que contém o payload cloth. Parte de uma entidade maior.

**Offset base:** apontado por `cloth_context.entity`

| Campo | Offset | Tipo | Acesso | Descricao |
|-------|--------|------|--------|-----------|
| `pad_000` | +0x000 | u8[0x78] | — | Inicio da entidade |
| `field_78` | +0x078 | ico_s32 | sw | Zerado por cloth_payload_init |
| `pad_07C` | +0x07C | u8[0x784] | — | Ate o payload |
| `payload` | +0x800 | ico_ptr32 | lw,sw | Ponteiro para cloth_payload |
| `field_814` | +0x814 | ico_s32 | sw | Escrito em 0x1B76F8 a partir de `entry[+0x30]` |

---

### cloth_payload

Dados de instância de um objeto cloth/rope. Inicializado por `func_0x001D27A8`.

**Offset base:** apontado por `cloth_entity.payload`

| Campo | Offset | Tipo | Acesso | Descricao |
|-------|--------|------|--------|-----------|
| `field_00` | +0x00 | ico_u32 | lw | Bandeira (testada < 1 para inatividade) |
| `variant_id` | +0x04 | ico_s32 | lw | 0 (area A) ou 1 (area B) — controla init path em 0x1D27A8 |
| `flag_08` | +0x08 | ico_u64 | ld | Bandeira 64-bit. zero = ativo |
| `pad_10` | +0x10 | u8[0x30] | — | Gap nao mapeado (dados de pose no runtime) |
| `field_40` | +0x40 | ico_s32 | lw | Preenchido com retorno de 0x1B7FE8 quando variant == 1 |
| `field_44` | +0x44 | ico_s32 | sw/lw | Timer/counter (decrementado em state 1 do dispatcher) |
| `state_id` | +0x48 | ico_u32 | lw | 0-4, indexa jump table do dispatcher 0x1D37C8 |
| `matrix_ptr` | +0x50 | ico_ptr32 | sw | Ponteiro para matriz de transform (state 2) |
| `result_ptr` | +0x60 | ico_ptr32 | sw | Ponteiro para resultado allocado (state 2 via 0x12A618) |
| `field_64` | +0x64 | ico_u32 | sw | Flag de fase (1=state 0 ativo, 0=state 2 limpa, 4=state 3 completa) |

**Evidencia:** 3 EXACT matches, runtime confirmacao de `[a1+0x30]` como area ID, `[a1+0x58]` como slot de callback vazio. `field_40` e `field_44` confirmados no disassembly de 0x1D27A8 (Rev.050).

---

### ~~physics_type_entry~~ REMOVIDO

**A tabela de tipos físicos anteriormente documentada em 0x001A48A0 foi REMOVIDA (Rev.059).** O endereço 0x1A48A0 está dentro da seção `.text` (0x100000–0x26F5D4) e decodifica como código MIPS, não como tabela de dados.

As tabelas reais que cumprem o papel da antiga "physics type table" são:

| Tabela | VA | Seção | Stride | Entries |
|--------|-----|-------|--------|---------|
| Entry table | 0x002A4C48 | .data | 0x4C (76) | 512 |
| Descriptor table | 0x002A31B8 | .data | 0x64 (100) | 68 |

Os nomes de tipo (WOODBOX01, ROTOBJEC, BARREL, ROPE, etc.) fazem parte do campo `name` (+0x00) do descritor, não de uma tabela separada.

---

### descriptor_record

Entrada na tabela de descritores (descriptor table). Tabela em `0x002A31B8`, stride 0x64.

**Tabela:** `0x002A31B8` (secao `.data`)
**Stride:** 0x64 (100 bytes)

| Campo | Offset | Tipo | Descricao |
|-------|--------|------|-----------|
| `name` | +0x00 | char[4-8] | Nome ASCII, 4-8 bytes null-padded |
| `phy_type` | +0x20 | u8 | Tipo de constraint fisico (0xF1=lever, 0xF2=woodbox, 0xF3=barrel, 0=generic) |
| `scale` | +0x28 | float | Escala fisica (1.0=active, 0.0=inactive) |
| `damping` | +0x2C | float | Damping/gravity (0.01=physics, 0.0=inert) |
| `count` | +0x30 | u32 | Limite de instancias |
| `init_fn` | +0x40 | ico_ptr32 | **Constructor/asset init** (0 = sem modelo dedicado) |
| `flags` | +0x44 | u32 | 1=active, 0=inactive |
| **hA** | **+0x48** | **ico_ptr32** | **Post-init/Reset handler** |
| (pad) | +0x4C | u32 | Zero |
| **hB** | **+0x50** | **ico_ptr32** | **Update per-frame handler** |
| (pad) | +0x54 | u32 | Zero |
| **hC** | **+0x58** | **ico_ptr32** | **Constructor/Init handler** (chamado por 0x1B76F8) |
| (pad) | +0x5C | u32 | Zero |

**Convencao de handlers (confirmada em 7 entidades, Rev.053-054):**

| Slot | Papel | Detalhes |
|------|-------|----------|
| **hC (+0x58)** | Constructor | Aloca heap via 0x13A0F8, init estado interno. Chamado pelo iterador 0x1B76F8 |
| **hB (+0x50)** | Update per-frame | Despachado em runtime. Contem colisao, AI, animacao, desenho |
| **hA (+0x48)** | Post-init / Reset | Chamado condicionalmente. Cleanup, reset de estado |
| **init_fn (+0x40)** | Asset init | Carrega modelos 3D, configura DMA. Chamado pelo sistema de assets |

**Indices conhecidos (Rev.056 — scan completo de 68 entries, indices corrigidos):**

| Indice | Label | init_fn (+0x40) | hA (+0x48) | hB (+0x50) | hC (+0x58) | Alloc hC | Status |
|--------|-------|-----------------|-------------|-------------|-------------|----------|--------|
| 1 | BOY | 0x153478 | 0x1C1F58 | 0x1C1DD8 | 0x1C1A98 | ? | ASM |
| 2 | GIRL | 0x174BA0 | 0x1D1A98 | 0x1D17F8 | 0x1D1668 | 64B | ASM |
| 4 | ENEMY1 | 0x164440 | 0x1CE690 | 0x1CE3C0 | 0x1CE220 | 80B | ASM |
| 17 | WOODBOX0 | 0x17D1D0 | 0x1C05D0 | 0x1C0538 | 0x1C00C0 | 400B | ASM |
| 19 | BARREL | 0 | 0x1D3B28 | 0x1D3A30 | 0x1D27A8 | ~40B | CLOTH |
| 20 | ROPE | 0 | 0x1E9630 | 0x1E9810 | 0x1E8F38 | — | OVERLAY |
| 30 | BGA | 0x203EE8 | 0 | 0 | 0 | 0 | ASM |
| 32 | BIRD | 0x1971C0 | 0x197080 | 0x197078 | 0x197240 | 64B | NEAR |
| 46 | QUEEN | 0x19B7F8 | 0x19A9A0 | 0x19A8F0 | 0x19A7E8 | 24B | NEAR |
| 48 | DEVIL_GI | 0x174BA0 | 0x1D1A98 | 0x1D17F8 | 0x1D1668 | 64B | ALIAS |
| 61 | AP1 | 0x1BB6B0 | 0x1BA530 | 0x1BA330 | 0x1B8720 | 640B | ASM |
| 62 | ATTACKCH | 0x1BBF78 | 0x1BBEC8 | 0x1BBEA0 | 0x1BBE50 | 12B | NEAR |
| 63 | ATTACKCH | 0x1BBF78 | 0x1BBE48 | 0x1BBDD8 | 0x1BBB20 | dyn*8 | ASM |
| 64 | BOSS_CTR | 0x198140 | 0x198000 | 0x197FC8 | 0x198218 | 0 | ASM |

**13 entradas com init_fn nao-nulo:** BOY, GIRL, ENEMY1, WOODBOX0, BGA, BIRD, QUEEN, DEVIL_GI, AP1, ATTACKCH x2, BOSS_CTR
**10 enderecos unicos init_fn:** GIRL/DEVIL_GI compartilham 0x174BA0, ATTACKCH 62/63 compartilham 0x1BBF78

**Observacao:** BARREL (idx 19) e ROPE (idx 20) sao descritores diferentes, mas na tabela de tipos fisicos (0x001A48A0) ambos aparecem com os mesmos handlers. Rev.056 corrigiu indices de varios descritores: WOODBOX0=17 (era 6), BGA=30 (era 50), AP1=61 (era 56).

---

### entry_table

Tabela de entries de objetos por sala/zona. Tabela em `0x002A4C48`, stride 0x4C.

**Tabela:** `0x002A4C48` (secao `.data`)
**Stride:** 0x4C (76 bytes)

| Campo | Offset | Tipo | Descricao |
|-------|--------|------|-----------|
| `pos_x` | +0x00 | float | Posicao X (quase sempre 1.0f padrao) |
| `pos_y` | +0x04 | float | Posicao Y |
| `pos_z` | +0x08 | float | Posicao Z |
| `rot` | +0x0C | float | Rotacao/angulo |
| `scale_a` | +0x10 | float | Escala/parametro A |
| `scale_b` | +0x14 | float | Escala/parametro B |
| `unk_18` | +0x18 | float | Transform field |
| `unk_1C` | +0x1C | float | Transform field |
| `unk_20` | +0x20 | float | Transform field |
| `callback_override` | +0x24 | ico_ptr32 | Callback override (0 = usa descriptor) |
| `unk_28` | +0x28 | ico_s32 | Desconhecido |
| `param_2C` | +0x2C | ico_s32 | Parametro numerico (0x5eb, 0x15, etc) |
| `param_30` | +0x30 | ico_s32 | Parametro numerico (copiado para [entity+0x814]) |
| `unk_34` | +0x34 | ico_s32 | Desconhecido |
| `unk_38` | +0x38 | ico_s32 | Desconhecido |
| `unk_3C` | +0x3C | ico_s32 | Desconhecido |
| `flags_hi` | +0x40 | u16 | Halfword de flags/shift |
| `unk_42` | +0x42 | u16 | Desconhecido |
| `unk_44` | +0x44 | u16 | Geralmente 0 |
| `descriptor_idx` | +0x46 | u8 | **Indice do descriptor** (mais importante!) |
| `subtype` | +0x47 | u8 | Byte de subtipo (mascara 0x1f) |
| `flags` | +0x48 | ico_u32 | Flags/mask (0x0011FFFF padrao) |

**Indices mapeados:**

| +0x46 | Label | Entries encontradas |
|---|---|---|
| 0x01 | BOY | 2 |
| 0x02 | GIRL | 2 |
| 0x07 | idx_07 | ~60 (objetos pequenos) |
| 0x0A | idx_0A | ~40 (objetos comuns) |
| 0x0E | idx_0E | ~15 |
| 0x13 | **BARREL** | **24+ (objetos cloth)** |
| 0x15 | CHAIN | 5 |
| 0x16 | FLEVER | 4 |
| 0x1C | CAMERADUMMY | ~30 |
| 0x1E | BGA | ~80 (background animations) |
| 0x2C | idx_2C | ~20 |

---

### cloth_state_block

Sub-bloco dentro de cloth_payload (offset +0x40). Contém estado local do dispatcher de 5 estados.

**Offset base:** `payload + 0x40` (referenciado como `s1` no dispatcher)

| Campo | Offset absoluto | Offset relativo a s1 | Tipo | Acesso | Descricao |
|-------|-----------------|----------------------|------|--------|-----------|
| `field_40` | payload+0x40 | +0x00 | ico_ptr32 | lw | Passado para guard function 0x1F2148 |
| `counter` | payload+0x44 | +0x04 | ico_s32 | sw/lw | Timer decrementado em state 1; quando 0 → state 2 |
| `state_id` | payload+0x48 | +0x08 | ico_u32 | lw | 0-4, seleciona handler no dispatcher 0x1D37C8 |
| `matrix` | payload+0x50 | +0x10 | ico_ptr32 | — | Workspace de matriz (state 2) |
| `result` | payload+0x60 | +0x20 | ico_ptr32 | sw | Result struct allocado (state 2, via 0x12A618) |

**State machine (Rev.057):**

```
State Flow: 0 → 1 → 2 → 3 → 4 (terminal). External code resets state to 0.

State 0 (GUARD, 0x1D3818):
  - Chama 0x1F2148(field_40, ctx)
  - Se retornar != 0: setup_1, setup_2, flag=1, state=1
  - Se retornar 0: exit (state fica 0)

State 1 (PREPARE/TIMING, 0x1D3844):
  - Copia quaternion seed de 0x4C4750 para stack
  - Calculo com dados de 0x274EC0/0x274EC4 (timing params)
  - ee_sqrt, ee_atan2, ee_cos para compute force
  - Copia quaternion resultante para [entity_state+0xA0]
  - counter--; se 0: state=2

State 2 (SIMULATE, 0x1D391C):
  - Limpa [payload+0x64]
  - CD functions (0x12ABE0/0x12AC28/0x12ADE8/0x12A618)
  - Matrix init + entity callback (0x181BF8)
  - cloth_phase_2 (0x1D12D8)
  - Alloca result struct, init quat, copy data de 0x276140
  - state=3

State 3 (CHECK, 0x1D39E0):
  - Chama 0x12A7F8(result_ptr)
  - Se pass: state=4, [payload+0]=1, [ctx+0x16C]=0
  - Se fail: retry (state fica 3)

State 4 (IDLE):
  - No-op. Epilogo direto
```

---

### callback_storage_node

Node de lista ligada usado pelo sistema de registro de callbacks. Gerenciado por `0x13F3F0` (576 bytes, stride 0x94).

**Offset base:** apontado por `entity_or_context + 0x10` (head da lista)

| Campo | Offset | Tamanho | Tipo | Descricao |
|-------|--------|---------|------|-----------|
| `next` | +0x00 | 8 | ico_ptr64 | Proximo node na lista ligada (ld/sd) |
| `data` | +0x08 | 4 | ico_ptr32 | Ponteiro para dados do callback |
| `type` | +0x0C | 4 | ico_u32 | Tipo do callback (ex: 0x13 = cloth) |
| `body` | +0x10 | 0x84 | u8[132] | Corpo do node (allocado separadamente) |

**Stride total:** 0x94 (148 bytes)

**Behavior (Rev.059):**
- `0x13F7A8(a0=obj, a1=data, a2=type, a3=&spill)`: wrapper que chama 0x13F3F0 duas vezes
  - Primeira: a0 = obj (callback na lista principal)
  - Segunda: a0 = obj+0x10 (sister callback storage)
- `0x13F7D8()`: variante sem contexto, a0=0x194, a1=0, t1=0x1800
- `0x13F3F0()`: busca linear (max 3 nodes). Se não encontra: alloc via 0x1A6E28
- Fallback: se alloc falha, armazena em `[obj+0x1C]` diretamente (mesmo offset de ThreadParam.entry no EE SDK)
- Runtime confirmou 1249 chamadas, todas com a3=0x13, 10 data pointers distintos (Rev.051)

---

| Propriedade | Valor |
|-------------|-------|
| `typedef` | `int` |
| Uso | Ponteiros internos de 32-bit armazenados em structs |
| Evidencia | 8/8 testes C confirmam lw/sw com int |
| Nao se aplica a | Ponteiros de funcao (DescriptorRecord callbacks) |

---

## Dominio de Valores

### state_id (cloth_payload +0x48)

| Valor | Handler | Fase | Descricao |
|-------|---------|------|-----------|
| 0 | 0x001D3818 | GUARD | Check condition. Se pass: setup, state→1 |
| 1 | 0x001D3844 | PREPARE/TIMING | Quaternion seed, counter--. Quando 0: state→2 |
| 2 | 0x001D391C | SIMULATE | Matrix, CD functions, result alloc, entity callback. state→3 |
| 3 | 0x001D39E0 | CHECK | Verifica resultado. Se ok: state→4, cleanup. Se nao: retry |
| 4 | 0x001D3A10 | IDLE/DONE | No-op. Algo externo reseta state para 0 |
| >=5 | — | INVALIDO | Bounds check → executa state 0 guard |

### variant_id (cloth_payload +0x04)

| Valor | Area observada no runtime |
|-------|---------------------------|
| 0 | Entrance, Y=-175 (area A) |
| 1 | Lower level, Y=-1245 (area B) |

---

## Decisoes de Modelagem

### ADR-DM-001 — `ico_ptr32` como `int`

| Campo | Detalhe |
|-------|---------|
| **Status** | Aceita |
| **Data** | 2026-05-16 |
| **Contexto** | O binario EE usa lw/sw (32-bit) para carregar enderecos internos de structs. Se usassemos ponteiros C (`void*`, `uintptr_t`), o GCC geraria ld (64-bit), divergindo do codigo original |
| **Decisao** | `typedef int ico_ptr32` e usar casts explicitos |
| **Alternativas** | `uint32_t` (geraria lwu, nao usado pelo ICO); `void*` (gera ld incorreto) |
| **Consequencias** | Necessario cast em toda leitura/escrita; compilador ee-gcc 2.9 consegue usar ulh/usw com int |

### ADR-DM-002 — Nomenclatura neutra para campos

| Campo | Detalhe |
|-------|---------|
| **Status** | Aceita |
| **Data** | 2026-05-16 |
| **Contexto** | Nomes como "state_id", "flag_08" e "field_40" descrevem comportamento binario, nao semantica de gameplay |
| **Decisao** | Manter nomes neutros ate que evidencia de gameplay confirme semantica |
| **Alternativas** | Nomes especulativos ("is_active", "cloth_type", "animation_state") |
| **Consequencias** | Documento e codigo fonte menos legiveis para leigos, mas evitam conclusao falsa |

---

## Anexo: GP-Relative Global Data Map

**GP = 0x006388F0** (confirmado do boot code em `0x100034-0x10005C`)

**Seccoes acessiveis via GP (`$28`):**

| Seccao | VA | Tamanho | GP offset range | Tipo |
|--------|-----|---------|-----------------|------|
| `.lit4` | 0x00630900 | 0xFD0 | -32752 a -28708 | Float constant pool (LWC1/SWC1, read-only) |
| `.sdata` | 0x00631900 | 0x22C6 | -28596 a -18960 | Initialized small data (853 vars, 8.669 acc) |
| `.sbss` | 0x00633C00 | 0x3F4 | -18960 a -18588 | Uninitialized small data (248 vars, 1.831 acc) |

**Variaveis globais mapeadas:**

| GP offset | VA | Secao | Acessos | Escritores | Provavel uso |
|-----------|-----|-------|---------|------------|-------------|
| -28172 | 0x00631AE4 | .sdata+0x1E4 | 1.260 | descriptor_iteration | `girl_obj_ptr` — ponteiro para entidade GIRL |
| -28168 | 0x00631AE8 | .sdata+0x1E8 | 986 | descriptor_iteration | `other_obj_ptr` — ponteiro para segunda entidade (BOY?) |
| -28164 | 0x00631AEC | .sdata+0x1EC | 113 | scene_loader_approx | `obj_ptr_2` |
| -18868 | 0x00633F3C | .sbss+0x33C | 434 | ? | Cloth vertex/state pointer (runtime) |
| -28544 | 0x00631970 | .sdata+0x070 | 160 | — | `state_field_2` |
| -28512 | 0x00631990 | .sdata+0x090 | 139 | descriptor_iteration | `world_state` — mundo atual * 404 (pre-multiplicado) |
| -23604 | 0x00632CBC | .sdata+0x13BC | 150 | print_stub_disabled | `texture_flag` |
| -26388 | 0x006321DC | .sdata+0x8DC | 62 | callback_system_reg | `callback_system_state` |
| -21424 | 0x00633540 | .sdata+0x1C40 | 52 | seeker_update | `seeker_state` |
| -19528 | 0x00633CA8 | .sbss+0x0A8 | 4 | node_callback_storage | `pool_base` |
| -19524 | 0x00633CAC | .sbss+0x0AC | 4 | node_callback_storage | `pool_capacity` |

**Clusters de structs:**

**Object Pointer Array** (gp[-28176..-28152], 6 ponteiros × 4B):
- -28176: `obj_ptr_0` (2 acc)
- -28172: `girl_obj_ptr` **HOT** (1.260 acc)
- -28168: `other_obj_ptr` **HOT** (986 acc)
- -28164: `obj_ptr_2` (113 acc)
- -28160: `obj_ptr_3` (8 acc)
- -28156: `obj_ptr_4` (3 acc)

**Callback/Event System** (.sbss+0x0A0..+0x0D0):
- -19536: `event_flag` (17 acc)
- -19528: `pool_base` (4 acc, writer: node_callback_storage)
- -19524: `pool_capacity` (4 acc, writer: node_callback_storage)
- -19520: `event_feedback_state` (9 acc, writer: event_feedback)

**Cloth System Struct Cluster** (.sbss+0x324..+0x364, 48 bytes, 12 slots, 2 writers):

| Offset | GP offset | Writer (scene 0x1DFBC8) | Writer (entity 0x1E00F8) | Reader (0x1D8E40) | Acesso total |
|---|---|---|---|---|---|
| `.sbss+0x324` | -18892 | sim | sim | — | — |
| `.sbss+0x328` | -18888 | sim | — | — | — |
| `.sbss+0x32C` | -18884 | — | sim | — | — |
| `.sbss+0x330` | -18880 | sim | — | — | — |
| `.sbss+0x334` | -18876 | sim | — | — | — |
| `.sbss+0x338` | -18872 | sim | sim | — | — |
| `.sbss+0x33C` | **-18868** | **sim** | **sim** | **7×** | **434 (MAIS ACESSADO)** |
| `.sbss+0x340` | **-18864** | **sim** | **sim** | **7×** | **117** |
| `.sbss+0x344` | -18860 | — | sim | — | — |
| `.sbss+0x348` | -18856 | sim | — | — | — |
| `.sbss+0x350` | -18848 | — | — | — | — |
| `.sbss+0x354` | -18844 | sim | — | — | — |

**Writers:** 0x1DFBC8 (scene init, 12 GP vars) + 0x1E00F8 (entity init, 8 GP vars). Ambos estão FORA do range clothAnimation.c (0x1D27A8-0x1D45B0).

**Function 0x1D8E40** (VU0 cloth render/compute, 295 insns, 656B stack, 20 JALs): acessa gp[-18868] e gp[-18864] em 2 blocos de VU0 kernel. Fora do range clothAnimation.c. Ver `research/elf/ghidra-rev063-vu0-cloth-compute-and-writer-functions.md`.

**Init values nao-zero em .sdata:** 40 entity type tags (OBJ, BGA, ENE, ICO, CAM, ACT, etc.), format strings `"%s : %s"` / `"%s : %d"`, texture format names (PSMCT32, PSMT8, etc.), GS texture params (SELTEX, SCRL-U, SHINE, etc.), funcao pointer `0x001D9020` (cloth VU0 SIMD), e 0xFFFFFFFF sentinelas.

**Init values no cluster callback** (.sdata+0x8E8..0x908):
| Offset | Valor | Significado |
|--------|-------|-------------|
| gp-26376 | 0x00000001 | count/ID |
| gp-26368 | 0x00000030 | 48 (size?) |
| gp-26360 | 0x000A7325 | packed config |
| gp-26352 | 0x00005010 | packed config |
| gp-26348 | **0x001D9020** | **Function pointer (VU0 cloth SIMD)** |
| gp-26344 | 0x0000000A | 10 (count?) |

> Fonte: `research/elf/ghidra-rev062-gp-relative-data-map.md`

---

### ADR-DM-003 — Entries da tabela de tipos tem stride fixo 0x64

| Campo | Detalhe |
|-------|---------|
| **Status** | Aceita |
| **Data** | 2026-05-16 |
| **Contexto** | Espacamento entre ROTOBJEC (0x1A48A0), BARREL (0x1A4904), ROPE (0x1A4968) e CHAIN (0x1A49CC) e exatamente 0x64 |
| **Decisao** | stride = 0x64 para todas as 31 entries |
| **Alternativas** | Entries com tamanhos variaveis (rejeitado pois quebraria indexacao por ID) |
| **Consequencias** | A funcao iteradora deve fazer `entry = base + index * 0x64` |

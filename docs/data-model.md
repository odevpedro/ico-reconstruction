# Data Model — ICO Reconstruction

> Documento vivo do modelo de dados reverso. Atualizado sempre que uma entidade for criada, alterada ou removida.
> **Ultima atualizacao:** 2026-05-16 (Rev.050 — added descriptor_entry, descriptor_record, entry_table; expanded cloth_payload, cloth_context)

---

## Indice

- [Visao Geral](#visao-geral)
- [Diagrama ER](#diagrama-er)
- [Entidades](#entidades)
- [Dominio de Valores](#dominio-de-valores)
- [Decisoes de Modelagem](#decisoes-de-modelagem)

---

## Visao Geral

Modelo de dados do sistema de objetos físicos do ICO, focado no subsistema cloth (corda/rope). O núcleo é uma hierarquia de 3 structs (context → entity → payload) mais uma tabela de tipos físicos (31 entries, stride 0x64).

- **Origem:** Engenharia reversa do ELF EE (MIPS R5900, ee-gcc 2.9-991111-01)
- **Nível de confiança:** EXACT ou NEAR-STRUCTURAL para todos os campos documentados (verificados por compilação C contra assembly original)
- **Nomenclatura:** Neutra (field_00, field_40, etc.) até que evidência adicional confirme semântica de gameplay

---

## Diagrama ER

```mermaid
erDiagram
    ENTRY_TABLE ||--|| DESCRIPTOR_RECORD : "+0x46 seleciona indice"
    DESCRIPTOR_RECORD ||--|{ PHYSICS_TYPE_TABLE : "handlers compartilhados"
    CLOTH_CONTEXT ||--|| CLOTH_ENTITY : "entity em +0x15C"
    CLOTH_ENTITY ||--|| CLOTH_PAYLOAD : "payload em +0x800"
    CLOTH_PAYLOAD ||--|| PHYSICS_TYPE_TABLE : "variant_id indexa entry"
    PHYSICS_TYPE_TABLE ||--|| CLOTH_PAYLOAD : "handler_c inicializa"
    CLOTH_PAYLOAD ||--|| STATE_BLOCK : "state_id em +0x48"
```

---

## Entidades

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
| `field_44` | +0x44 | ico_s32 | swc1 | Resultado do calculo timing/speed em 0x1D2978 |
| `state_id` | +0x48 | ico_u32 | lw | 0-4, indexa jump table do dispatcher 0x1D37C8 |

**Evidencia:** 3 EXACT matches, runtime confirmacao de `[a1+0x30]` como area ID, `[a1+0x58]` como slot de callback vazio. `field_40` e `field_44` confirmados no disassembly de 0x1D27A8 (Rev.050).

---

### physics_type_entry

Entrada na tabela de tipos de objeto físico. Tabela em `0x001A48A0`, stride 0x64, 31 entries.

**Tabela:** `0x001A48A0` (dentro da secao `.text`)
**Stride:** 0x64 (100 bytes)

| Campo | Offset | Tipo | Descricao |
|-------|--------|------|-----------|
| `count` | +0x00 | ico_u32 | Geralmente 1 |
| `handler_a` | +0x04 | ico_ptr32 | Primeiro handler (cleanup/post-dispatch) |
| `pad_08` | +0x08 | ico_ptr32 | Sempre null |
| `handler_b` | +0x0C | ico_ptr32 | Segundo handler (update/callback principal) |
| `pad_10` | +0x10 | ico_ptr32 | Sempre null |
| `handler_c` | +0x14 | ico_ptr32 | Terceiro handler (init/payload init) |
| `pad_18` | +0x18 | ico_u32 | Sempre null |
| `pad_1C` | +0x1C | ico_u32 | Sempre null |
| `name` | +0x20 | char[8] | Nome ASCII (ex: "ROPE\0\0\0\0") |

**Tipos conhecidos:** WOODBOX01, ROTOBJEC, BARREL, ROPE, CHAIN, FLEVER, WLEVER, NONE, CAMERADU, BIRD, GENERATO, CANDLE, MOBJ, CHANDELI, WORM, POOL, DARKVOLU, MCOLTEST, ROPEFIX, CAGE, DYNAMICM, QUEEN, QUEENDEM, CAGEFIX, CLOTHTES

**Observacao:** Os tipos ROPE e ROPEFIX sao duas variantes da mesma categoria — ROPE usa funcoes cloth (0x1Dxxxx), ROPEFIX usa funcoes overlay (0x1Exxxx).

---

### descriptor_record

Entrada na tabela de descritores (descriptor table). Tabela em `0x002A31B8`, stride 0x64.

**Tabela:** `0x002A31B8` (secao `.data`)
**Stride:** 0x64 (100 bytes)

| Campo | Offset | Tipo | Descricao |
|-------|--------|------|-----------|
| ... | +0x00..+0x3C | varios | Campos nao mapeados |
| `callback_aux` | +0x48 | ico_ptr32 | Handler auxiliar (ex: 0x1D3B28 para BARREL) |
| `callback_update` | +0x50 | ico_ptr32 | Handler update/frame (ex: 0x1D3A30 para BARREL) |
| `callback_init` | +0x58 | ico_ptr32 | Handler init/payload (ex: 0x1D27A8 para BARREL) |

**Indices conhecidos:**

| Indice | Label | VA | +0x40 | +0x48 | +0x50 | +0x58 |
|--------|-------|----|-------|-------|-------|-------|
| 0x01 | BOY | 0x002A321C | 0x00153478 | 0x00000000 | 0x00000000 | 0x00000000 |
| 0x02 | GIRL | 0x002A3280 | 0x00174BA0 | 0x001C1DD8 | 0x001C1F58 | 0x00000000 |
| 0x13 | BARREL | 0x002A3924 | 0x00000000 | 0x001D3B28 | 0x001D3A30 | 0x001D27A8 |
| 0x14 | ROPE | 0x002A3988 | 0x00000000 | 0x001D3B28 | 0x001D3A30 | 0x001D27A8 |
| 0x15 | CHAIN | 0x002A39EC | 0x00000000 | 0x1E9630 | 0x1E9810 | 0x1E8F38 |
| 0x16 | FLEVER | 0x002A3A50 | 0x00000000 | 0x18F640 | 0x18ECC8 | 0x18E5B0 |
| 0x17 | FLEVER_TRISTATE | 0x002A3AB4 | 0x00000000 | 0x1BC438 | 0x1BC130 | 0x1C09C8 |

**Observacao:** BARREL (0x13) e ROPE (0x14) compartilham EXATAMENTE os mesmos 3 handlers. A entry table usa BARREL, nunca ROPE.

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

### state_block

Bloco interno de estado do dispatcher `0x1D37C8`.

| Campo | Offset | Tipo | Acesso | Descricao |
|-------|--------|------|--------|-----------|
| `state_id` | +0x48 | ico_u32 | lw | 0-4, seleciona state block |

**State blocks:**

| ID | VA | Tamanho provavel |
|----|-----|-----------------|
| 0 | 0x001D3818 | ~44 bytes |
| 1 | 0x001D3844 | ~216 bytes |
| 2 | 0x001D391C | ~196 bytes |
| 3 | 0x001D39E0 | ~48 bytes |
| 4 | 0x001D3A10 | ~24 bytes |

---

### ico_ptr32 (tipo provisorio)

| Propriedade | Valor |
|-------------|-------|
| `typedef` | `int` |
| Uso | Ponteiros internos de 32-bit armazenados em structs |
| Evidencia | 8/8 testes C confirmam lw/sw com int |
| Nao se aplica a | Ponteiros de funcao (DescriptorRecord callbacks) |

---

## Dominio de Valores

### state_id (cloth_payload +0x48)

| Valor | Significado |
|-------|-------------|
| 0 | State block 0 (0x001D3818) no dispatcher |
| 1 | State block 1 (0x001D3844) |
| 2 | State block 2 (0x001D391C) |
| 3 | State block 3 (0x001D39E0) |
| 4 | State block 4 (0x001D3A10) |
| >=5 | Bounds check — cai no guard do dispatcher |

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

### ADR-DM-003 — Entries da tabela de tipos tem stride fixo 0x64

| Campo | Detalhe |
|-------|---------|
| **Status** | Aceita |
| **Data** | 2026-05-16 |
| **Contexto** | Espacamento entre ROTOBJEC (0x1A48A0), BARREL (0x1A4904), ROPE (0x1A4968) e CHAIN (0x1A49CC) e exatamente 0x64 |
| **Decisao** | stride = 0x64 para todas as 31 entries |
| **Alternativas** | Entries com tamanhos variaveis (rejeitado pois quebraria indexacao por ID) |
| **Consequencias** | A funcao iteradora deve fazer `entry = base + index * 0x64` |

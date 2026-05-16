# Cloth Struct Hypotheses

## Date

2026-05-16 (Updated with ico_ptr32 rule and exact match evidence)

## Resumo executivo

Hipóteses de estruturas de dados para o sistema cloth do ICO, baseadas em
offsets confirmados por 6 near-matches de accessors + análise runtime
Rev.046 + 3 retestes com `ico_ptr32` (Rev.048).

**Mudanças desta revisão**:
- `ico_ptr32` (typedef para `int`) é a regra provisória para campos que
  armazenam endereços internos do jogo (gera `lw`, não `ld`)
- `EntityContext.entity` e `Entity.payload` mudaram de `void*` para `ico_ptr32`
- O initializer struct (`a1`) continua como `StackInitializer` na stack

Cinco estruturas separadas:

1. **EntityContext** — contexto passado em `a0` (contém endereço da entidade)
2. **Entity** — entidade que contém payload cloth em +0x800
3. **ClothPayload** — dados de simulação (variant, state_id, flags)
4. **DescriptorRecord** — registro fixo na `.data` (contém slots de callback)
5. **StackInitializer** — estrutura temporária na stack (passada em `a1`)

## Evidência runtime (Rev.046)

Captura no PCXS2 debugger, breakpoint em `0x001D27A8`:

| Registrador | Valor | Significado |
|---|---|---|
| `a0` | `0x008320B4` | Entity context (heap) |
| `a1` | `0x00798E40` | Stack initializer (**a1 == sp**) |
| `sp` | `0x00798E40` | Stack pointer |
| `s0` | `0x002A3924` | Descriptor (`.data`) |
| `s3` | `0x008320B4` | Entity context (cópia) |
| `ra` | `0x001B7A88` | Caller |
| `[a1+0x30]` | `0` | Variant inicial |
| `[a1+0x58]` | `0` | (não é callback) |

## Estruturas

### 0. Regra provisória: `ico_ptr32`

```c
typedef int ico_ptr32;
```

`ico_ptr32` modela endereços internos do jogo armazenados como words de
32 bits. O tipo `int` (signed) foi escolhido porque gera `lw`, que é o
que o ICO usa. `unsigned int` geraria `lwu`. `void*` geraria `ld`.

**Confirmado por**: 3 retestes com exact near-match (0x1D3DB0, 0x1D3D40,
0x1D40A0) — todos geraram `lw` para entity e payload com este tipo.

**Cautela**: não é tipo final. Não aplicar a campos que são ponteiros de
sistema (callbacks em DescriptorRecord) sem verificar se o ICO usa `ld`
ou `lw` para carregá-los.

### 1. EntityContext (era "ClothContext")

Passado em `a0` para funções do cluster cloth. Contém endereço da
entidade e endereço auxiliar, ambos como `ico_ptr32`.

```c
// Tamanho: pelo menos 0x170 bytes
struct EntityContext {
    uint8_t  pad_000[0x15C];          // 348 bytes de uso interno
    ico_ptr32 entity;                  // +0x15C — endereço da Entity (32-bit)
    uint8_t  pad_160[0x16C - 0x160];  // 12 bytes
    ico_ptr32 extra_ptr;               // +0x16C — endereço auxiliar (32-bit)
};
```

**Offsets confirmados por**:
- `+0x15C`: func_001D3D70, 3D80, 3D98, 3DB0, 3D40, 40A0, dispatcher 37C8
- `+0x16C`: func_001D3D40, 3DD8, 4170

### 2. Entity (era "ClothEntity")

Entidade que contém o payload cloth em `+0x800`.

```c
// Tamanho: pelo menos 0x804 bytes
struct Entity {
    uint8_t  pad_000[0x800];          // 2048 bytes — estrutura maior de entidade
    ico_ptr32 payload;                 // +0x800 — endereço do ClothPayload (32-bit)
};
```

**Offsets confirmados por**:
- `+0x800`: func_001D3D70, 3D80, 3D98, 3DB0, 3D40, 40A0, dispatcher 37C8,
  payload_init 27A8, runtime Rev.046

### 3. ClothPayload

Payload de simulação. Instalado em `[entity + 0x800]` pela função
`0x001D27A8`. Referenciado como `state_block` no dispatcher (acesso ao
state_id via `+0x48`).

```c
// Tamanho: pelo menos 0x4C bytes (acessamos até +0x48)
struct ClothPayload {
    int       field_00;                // +0x00 — flag de atividade
    int       variant;                 // +0x04 — mode/variant (indexa 0x4d4188)
    uint64_t  flag_08;                 // +0x08 — flag 64-bit (se zero, variant válido)
    uint8_t   pad_10[0x40 - 0x10];    // 48 bytes
    int       state_id;                // +0x48 — state ID (0-4, usado pelo dispatcher)
    uint8_t   pad_4C[0x60 - 0x4C];    // 20 bytes
    void     *check_ptr;               // +0x60 — usado em func_0012A7F8 (state resolver)
    uint8_t   pad_64[0x74 - 0x64];    // 16 bytes
    int       arg_storage;             // +0x74 — armazena argumento (usado em 4170/3DD8)
};
```

**Offsets confirmados por**:
- `+0x00`: func_001D3D80 (testa zero)
- `+0x04`: func_001D3D70, payload_init 27A8, dispatcher 37C8
- `+0x08`: func_001D3D40, 3DD8, 4170 (flag 64-bit)
- `+0x48`: func_001D3D98, 3DB0, 3D40, 3DD8, 4170, dispatcher 37C8
- `+0x60`: func_001D40D8 (passado para func_0012A7F8)
- `+0x74`: func_001D4170, 3DD8 (escrita)

### 4. DescriptorRecord

Registro fixo na `.data`. Contém nome, parâmetros e slots de callback.
O runtime confirmou que `s0 = 0x002A3924` aponta para um destes.

```c
// Tamanho: ~0x60 bytes (observado)
struct DescriptorRecord {
    int       flags_or_id;             // +0x00
    char      name[12];               // +0x04 — nome ASCII (BARREL, CHAIN, etc.)
    uint8_t   pad_10[0x44 - 0x10];   // 52 bytes — parâmetros, floats, IDs
    void     *slot_48;                // +0x44 — callback +0x48 (aux)
    void     *slot_50;                // +0x4C — callback +0x50 (update)
    void     *slot_58;                // +0x54 — callback +0x58 (init) ★
    uint8_t   pad_5C[?];             // +0x5C — (zero no BARREL)
    // +0x60: possível segundo nome ou rótulo (ex: "ROPE" no BARREL)
};
```

**Offsets confirmados por** runtime (dump em `0x002A3924`):
- `+0x44`: `0x001D3B28` (cloth_aux)
- `+0x4C`: `0x001D3A30` (cloth_update_cb — o "ROPE callback")
- `+0x54`: `0x001D27A8` (cloth_payload_init — ★ confirmado via jalr)
- Nome "BARREL" em `+0x04`, label "ROPE" em `+0x60`

### 5. StackInitializer (era "InitArg")

Estrutura temporária na stack do caller, passada como `a1`. **Não é um
descritor global**. Confirmado: `a1 == sp` no entrypoint de `0x001D27A8`.

```c
// Tamanho: ~0x80 bytes (observado)
struct StackInitializer {
    float     pos_x;                   // +0x00 (410.5 neste hit)
    float     pos_y;                   // +0x04 (-175.0)
    float     pos_z;                   // +0x08 (1333.0)
    float     scale_or_weight;         // +0x0C (1.0)
    uint8_t   pad_10[0x10];           // +0x10 zeros
    float     blend[4];               // +0x20 (1.0, 1.0, 1.0, 1.0)
    int       variant;                 // +0x30 — copiado para payload.variant (0 neste hit)
    uint8_t   pad_34[0x0C];           // +0x34
    int       size_or_id;              // +0x40 (0x00EBE1D2 — repetido)
    // +0x40: segunda metade = cópia da primeira metade (pose alvo?)
};
```

**Offsets confirmados por** runtime (dump em `0x00798E40`):
- `+0x30`: `0` (variant)
- `+0x58`: `0` (NÃO contém callback — corrige Rev.044)
- Struct duplicada em `+0x40`: sugere "pose inicial → pose alvo"

## Mapa completo

```
Descritor (.data, s0=0x002A3924 "BARREL")
  +0x58 → 0x001D27A8 (callback init)
       │
       ├─ a0 = EntityContext (heap, s3=0x008320B4)
       │    +0x15C → Entity
       │               +0x800 → ClothPayload (alocado por 0x001D27A8)
       │
       ├─ a1 = StackInitializer (stack, sp=0x00798E40)
       │    +0x00: posição (410.5, -175.0, 1333.0)
       │    +0x20: blend/weights (1.0, 1.0, 1.0, 1.0)
       │    +0x30: variant = 0 (→ payload.variant)
       │    +0x40: cópia (pose alvo?)
       │
       └─ retorno = payload
            └─ [Entity + 0x800] = payload
```

## O que mudou com runtime e exact matches

### Mudanças runtime (Rev.046-047)

| Antes | Depois |
|---|---|
| `InitArg` como descriptor global | `StackInitializer` na stack |
| `[a1+0x58]` suspeito conter callback | `[a1+0x58] = 0` (não é callback) |
| `InitArg` só com variant | Struct completa com dados de transform |
| Descritor não observado | `DescriptorRecord` com nome e 3 slots |
| "ROPE" como descritor separado | "ROPE" é label dentro do BARREL |

### Mudanças de modelagem (Rev.048 — exact match retests)

| Antes | Depois | Evidência |
|---|---|---|
| `void *entity` em EntityContext | `ico_ptr32 entity` | 3 retestes: `int` gera `lw` (match), `void*` gera `ld` (mismatch) |
| `void *payload` em Entity | `ico_ptr32 payload` | mesmo padrão |
| Todos os ponteiros como `void*` | `ico_ptr32` para endereços, `void*` apenas para callbacks de sistema | `DescriptorRecord.slot_*` ainda não testados |

## Nomes provisórios vs ICO-decomp

| Nosso nome | ICO-decomp | Status |
|---|---|---|
| `ClothPayload.variant` | unnamed | Offset +0x04 confirmado |
| `ClothPayload.state_id` | unnamed | Offset +0x48 confirmado |
| `DescriptorRecord.slot_58` | unnamed | Runtime: contém 0x001D27A8 |
| `StackInitializer.variant` | unnamed | Runtime: = 0 neste hit |
| `Entity` | unnamed | Ponteiro em +0x15C |
| "BARREL" | unnamed | Observado em runtime |
| "ROPE" | unnamed | É label, não descritor |

## Riscos

1. **Apenas 1 hit runtime**: os valores podem mudar em outros contexts
2. **Struct duplicada**: pode ser coincidência deste hit específico
3. **Posição 410.5, -175.0, 1333.0**: parece posição de fase específica
4. **`extra_ptr`**: função exata desconhecida
5. **Nomes**: todos provisórios — semântica real depende de mais evidência

## Próximo passo

1. Breakpoint em `0x001B7A74` — entender quem chama o caller
2. Breakpoint em `0x0013F7A8` — ver registro do callback no slot +0x50
3. Mais hits de `0x001D27A8` — ver se variant muda
4. Consolidar C sources testados em `research/external/cloth-exact-match-c-sources.md`
5. Testar `0x001D4358` (pack color, 160B) — próximo candidato sem branches


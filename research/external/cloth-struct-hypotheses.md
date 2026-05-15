# Cloth Struct Hypotheses

## Date

2026-05-15

## Resumo executivo

Hipóteses de estruturas de dados para o sistema cloth do ICO, baseadas em
offsets confirmados por 6 near-matches de accessors e pela análise do
dispatcher/registro. Nomes e semântica são **provisórios** — a confirmação
final depende de runtime.

## Critérios de inclusão

- Offsets confirmados por **2 ou mais funções** no cluster cloth
- Offsets rastreados até o código-fonte do ICO-decomp (quando disponível)
- Nomes baseados em contexto, não em especulação de gameplay

## Estruturas

### 1. ClothContext

Provável estrutura passada em `a0` para funções do cluster cloth.

```c
// Tamanho: pelo menos 0x170 bytes (acessamos até +0x16C)
struct ClothContext {
    uint8_t  pad_000[0x15C];          // 348 bytes de uso interno
    void    *entity;                   // +0x15C — ponteiro para ClothEntity
    uint8_t  pad_160[0x16C - 0x160];  // 12 bytes
    void    *extra_ptr;                // +0x16C — ponteiro auxiliar (opcional, testado em 3D40)
};
```

**Offsets confirmados por**:
- `+0x15C`: func_001D3D70, 3D80, 3D98, 3DB0, 3D40, 40A0, dispatcher 37C8
- `+0x16C`: func_001D3D40, 3DD8, 4170

### 2. ClothEntity

Entidade que contém o payload cloth. O offset `+0x800` é grande o suficiente
para sugerir que esta estrutura é um nó de jogo/entidade mais amplo.

```c
// Tamanho: pelo menos 0x804 bytes
struct ClothEntity {
    uint8_t  pad_000[0x800];          // 2048 bytes — possivelmente uma entidade de jogo
    void    *payload;                  // +0x800 — ponteiro para ClothPayload
};
```

**Offsets confirmados por**:
- `+0x800`: func_001D3D70, 3D80, 3D98, 3DB0, 3D40, 40A0, dispatcher 37C8,
  payload_init 27A8

### 3. ClothPayload

Payload de simulação de cloth. Contém campos de estado, variante, flags.
Esta estrutura é armazenada em `[entity + 0x800]` e referenciada como
`state_block` no dispatcher (via `state_block + 0x40` para o campo state_id).

```c
// Tamanho: pelo menos 0x4C bytes (acessamos até +0x48)
struct ClothPayload {
    int       field_00;                // +0x00 — status/flag (testado como booleano)
    int       variant;                 // +0x04 — mode/variant (indexa tabela 0x4d4188)
    uint64_t  flag_08;                 // +0x08 — flag 64-bit (se zero, variant é válido?)
    uint8_t   pad_10[0x40 - 0x10];    // 48 bytes
    int       state_id;                // +0x40 (via state_block+0x40+0x08=+0x48)
                                       // Na prática: [state_block + 0x48]
                                       // Onde state_block = ClothPayload
    // Acima de +0x4C: fields acessados por update_callback e outros
};
```

**Offsets confirmados por**:
- `+0x00`: func_001D3D80 (testa se é zero)
- `+0x04`: func_001D3D70 (retorna), cloth_payload_init 27A8 (escreve), dispatcher 37C8 (usa como variant indireto)
- `+0x08`: func_001D3D40, 3DD8, 4170 (testa se é zero como flag)
- `+0x48`: func_001D3D98, 3DB0, 3D40, 3DD8, 4170, dispatcher 37C8 (state_id)

### 4. InitArg (provisório — depende de runtime)

Argumento recebido em `a1` por `cloth_payload_init_001d27a8`. A estrutura
exata **depende de runtime** para ser confirmada.

```c
// Hipotético — baseado em Rev.043
struct InitArg {
    uint8_t  pad_00[0x30];           // 48 bytes de uso interno
    int      variant_id;              // +0x30 — valor copiado para payload.variant
    // Existe campo em +0x58? Rev.044 mencionou possível field.
};
```

**Offsets**:
- `+0x30`: confirmado por Rev.043 (escrito em payload+0x04)
- `+0x58`: mencionado em Rev.044 como possível field de constructor — **não confirmado**

## Mapa de memória (cluster cloth)

```
ctx (a0)
  +0x15C → entity
             +0x800 → payload
                        +0x00  field_00    (flag de atividade)
                        +0x04  variant     (mode, indexa 0x4d4188)
                        +0x08  flag_08     (64-bit, trava variant?)
                        +0x10  ...
                        +0x48  state_id    (0-4, usado pelo dispatcher)
                        +0x4C  ...
  +0x16C → extra_ptr (opcional)
```

## Nomes provisórios vs ICO-decomp

| Nosso nome | ICO-decomp | Status |
|---|---|---|
| `ClothPayload.variant` | unnamed | Offset +0x04 confirmado |
| `ClothPayload.state_id` | unnamed | Offset +0x48 confirmado |
| `ClothPayload.flag_08` | unnamed | Offset +0x08 confirmado, lido como `ld` |
| `ClothEntity` | unnamed | Existe como conceito |
| `InitArg.variant_id` | unnamed | Offset +0x30, origem runtime |

Nenhum destes campos tem nome no ICO-decomp (5792 símbolos, 0 matches).

## Riscos

1. **Padding**: estruturas podem ter padding diferente do esperado
2. **Union**: campos podem ser unions, não fields separados
3. **Tipo `int`**: assumimos 32-bit, mas alguns podem ser 16-bit ou 64-bit
4. **Semântica**: nomes como `variant`, `state_id`, `flag_08` são descritivos,
   não funcionais
5. **`extra_ptr` em +0x16C**: só testado em 3 funções, pode ser outro campo

## Próximo passo

Runtime para confirmar `InitArg` e verificar se as hipóteses de struct
estão corretas. Até lá, estas estruturas devem ser tratadas como
**provisórias e sujeitas a correção**.

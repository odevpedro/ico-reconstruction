# First Scratch Candidate — `func_001D3D70`

## Date

2026-05-15

## Resumo executivo

Preparação para o primeiro scratch real de matching C contra o ICO,
usando `func_001D3D70` (16 bytes) como alvo. A função é um accessor
simples que navega por uma cadeia de ponteiros: carrega `[a0+0x15c]`,
depois `[v0+0x800]`, e retorna `[v1+0x04]`.

É o candidato ideal para testar o GCC 2.95.2 PS2 Linux com flags ICP
em um cenário de matching real.

## Por que esta função

| Critério | Atende? |
|---|---|
| Tamanho | **16 bytes** (4 instruções) |
| Jump table | ❌ Ausente |
| Switch | ❌ Ausente |
| Float complexo | ❌ Ausente |
| Chamada indireta | ❌ Ausente |
| VU/MMI | ❌ Ausente |
| Estrutura grande | ❌ Accessor trivial |
| Chamada de função | ❌ Leaf function |
| Total de branches | 0 |

É a função mais simples do cluster cloth, com zero branches e zero
chamadas. Perfeita para validar o pipeline de scratch sem ruído.

## Assembly observado

Fonte: `/tmp/ico-cloth-full/asm/cloth_sub_001d3d70.s`

```asm
glabel func_001D3D70
    /* D4D70 001D3D70 5C01828C */  lw         $v0, 0x15C($a0)
    /* D4D74 001D3D74 0008438C */  lw         $v1, 0x800($v0)
    /* D4D78 001D3D78 0800E003 */  jr         $ra
    /* D4D7C 001D3D7C 0400628C */   lw        $v0, 0x4($v1)
.size func_001D3D70, . - func_001D3D70
```

### Tradução register:

- `$a0` → argumento 1 (context/entity pointer)
- `$v0` → retorno / temporário
- `$v1` → temporário
- `$ra` → return address

### Interpretação:

```c
// Pseudocódigo:
int func_001D3D70(void *ctx) {
    struct entity *e = *(struct entity**)(ctx + 0x15C);
    struct payload *p = *(struct payload**)(e + 0x800);
    return p->field_04;  // field at offset 0x04
}
```

Ou mais provavelmente, considerando o domínio cloth:

```c
int cloth_get_variant(void *context) {
    // context + 0x15C = entity pointer (confirmado no dispatcher)
    // entity + 0x800 = cloth payload (confirmado em Rev.043)
    // payload + 0x04 = variant/mode field (confirmado em Rev.041-042)
    return *(int*)(*(void**)(*(void**)(context + 0x15C) + 0x800) + 0x04);
}
```

## Hipótese de assinatura

A função lê o campo `variant` (também chamado `mode`) do payload cloth.
Este campo indexa a tabela `0x004d4188` (8 entries, stride 0x14,
documentado na Rev.041).

Funções similares que acessam o mesmo campo:
- `0x001d2850` (`lw $v1, 0x30($s4)`) — escreve o variant no payload
- `0x001d37f0` (`lw $v1, 0x8($s1)`) — lê variant do dispatcher
  (mas via `state_block + 0x40 + 0x08`)

Nome sugerido (provisório): `cloth_get_variant`

## C mínimo proposto

```c
// Tentativa 1 — casting explícito
struct ee_entity {
    void *payload;  // offset 0x800 do entity (mas há campos antes)
};

struct cloth_payload {
    int unk00;
    int variant;    // offset 0x04
};

int cloth_get_variant(void *context) {
    void *entity = *(void**)((char*)context + 0x15C);
    struct cloth_payload *p = *(struct cloth_payload**)((char*)entity + 0x800);
    return p->variant;
}
```

```c
// Tentativa 2 — struct mais idiomática
typedef struct {
    int field_00;
    int field_04;
    int field_08;
    int field_0C;
    void *parent;
} ClothPayload;

typedef struct {
    int id;
    ClothPayload *payload;  // no offset 0x800
} ClothEntity;

int cloth_get_variant(void *context) {
    ClothEntity *ent;
    // load entity pointer from context + 0x15C
    __builtin_memcpy(&ent, (char*)context + 0x15C, sizeof(ent));
    // load payload pointer from entity + 0x800
    __builtin_memcpy(&ent, (char*)ent + 0x800, sizeof(ent));
    return ((ClothPayload*)ent)->field_04;
}
```

A Tentativa 1 é preferível por ser mais previsível — sem struct aninhada
que pode gerar código diferente do esperado.

## Flags candidatas

Usar ICP (config mais próxima do ICO conforme testes anteriores):

```
-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls
```

Também testar variante com `-mabi=eabi`:

```
-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -mabi=eabi -fno-pic -mno-abicalls
```

## Critérios de comparação

### Sequência de instruções

O assembly alvo é:

```asm
lw $v0, 0x15C($a0)    ; carrega entity pointer de context+0x15C
lw $v1, 0x800($v0)    ; carrega payload pointer de entity+0x800
jr $ra                 ; return
lw $v0, 0x4($v1)      ; delay slot: carrega variant field
```

### O que verificar

| Aspecto | Alvo (ICO) | Ideal (GCC) |
|---|---|---|
| Prologue | Nenhum (leaf, 0 stack) | Nenhum |
| Load 1 | `lw $v0, 0x15C($a0)` | load de (a0+0x15C) |
| Load 2 | `lw $v1, 0x800($v0)` | load de (v0+0x800) |
| Load 3 | `lw $v0, 0x4($v1)` | load de (v1+0x04) |
| Return | `jr $ra` | `j $31` ou `jr $ra` |
| Delay slot | Load 3 no slot do `jr` | Load no slot do `jr` |
| Stack | 0 bytes | 0 bytes |
| Registradores | `$v0`, `$v1` apenas | 2 temporários no máximo |

## O que seria match perfeito

```
// GCC output idêntico (apenas nomes de registradores)
lw $2, 0x15C($4)       # lw $v0, 0x15C($a0)
lw $3, 0x800($2)       # lw $v1, 0x800($v0)
jr $31                  # jr $ra
lw $2, 0x4($3)         # lw $v0, 0x4($v1)
```

Diferença aceitável: `$2` vs `$v0`, `$4` vs `$a0`, etc.
(são os mesmos registradores, apenas notação diferente).

## O que seria near-match aceitável

1. **Uso de `ld` em vez de `lw` para loads**: se o GCC tratar os ponteiros
   como 64-bit, pode gerar `ld` em vez de `lw`. Aceitável se a sequência
   e offsets forem os mesmos.

2. **Stack frame pequeno**: se o GCC gerar `dsubu $sp, $sp, N` / `daddu $sp`
   mesmo sem necessidade. Aceitável se N ≤ 32 e o resto do código for igual.

3. **Ordem diferente de loads**: se o GCC inverter a ordem (ex: load
   `0x800` antes de `0x15C`). Improvável para leaf function.

4. **`nop` no delay slot**: se o GCC não colocar o load no delay slot.
   Aceitável, perde apenas a otimização de delay slot.

## Riscos

1. **Struct padding**: se o C definir structs com padding diferente do
   esperado pelo código ICO, os offsets mudam. Mitigação: usar aritmética
   de ponteiros explícita (`(char*)ptr + offset`).

2. **Tipo do campo variant**: `int` (32-bit) vs `long` (64-bit). Se o GCC
   gerar `ld`/`sd` em vez de `lw`/`sw`. Mitigação: declarar como `int`.

3. **`__builtin_memcpy` vs cast direto**: o GCC pode tratar casts de
   ponteiro de forma diferente de `memcpy`. Mitigação: testar ambas.

4. **Otimização excessiva**: o GCC pode foldar a leitura se detectar que
   o valor não é usado (com `-O2`). Mitigação: retornar o valor.

5. **Delay slot preenchido com nop**: o GCC pode não conseguir agendar
   o load no slot do `jr`. Isso é aceitável mas reduz a qualidade do match.

## Próximo passo recomendado

1. ✅ **Análise da função concluída** (este documento)
2. 🔄 **Compilar no container Docker** com ICP e ICP+EABI
3. 🔄 **Comparar output com o assembly do ICO**
4. 🔄 **Ajustar o C se necessário** (tentar diferentes struct layouts)
5. 🔄 **Classificar o resultado**: match / near-match / mismatch
6. 🔄 **Se match/near-match: preparar scratch decomp.me**
7. 🔄 **Se mismatch: documentar e tentar próxima candidata**
   (ex: `func_001D3D80`, 18 bytes, outro accessor)

## Veredito

```txt
Probabilidade de match perfeito: MÉDIA (depende do tratamento de delay slot)
Probabilidade de near-match: ALTA
Dificuldade: MUITO BAIXA (4 instruções, leaf function)
Valor como primeiro scratch: ALTO (valida todo o pipeline)
Risco: BAIXO (função trivial, sem armadilhas conhecidas)
```

`func_001D3D70` é o alvo certo para o primeiro scratch. Se o GCC 2.95.2
PS2 Linux gerar código idêntico (exceto nomes de registradores), isso
valida o pipeline de matching C → asm → comparação. Se não gerar, a
diferença será pequena e instrutiva.

Em qualquer caso, o custo de tentar é mínimo (4 instruções), e o
aprendizado é máximo.

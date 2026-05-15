# State Resolver Notes — func_0013EB50 / func_0013EBE0

## Date

2026-05-15

## Resumo executivo

Análise conceitual do par de funções `func_0013EB50` / `func_0013EBE0`
que implementa um padrão de iteração de entidades filtrado por descriptor
ID. Este padrão bloqueia 3 funções do cluster cloth (`func_001D40D8`,
`func_001D3DD8`, `func_001D4170`).

## O padrão observado

### Chamada

Todas as funções que usam este padrão fazem:

```asm
addiu $a0, $zero, 0x13    ; a0 = 0x13 (ROPE descriptor ID)
jal   func_0013EB50        ; get first node
daddu $s0, $v0, $zero     ; s0 = first node (or null)

loop:
  lw   $v1, 0x15C($s0)     ; entity from context
  lw   $a0, 0x800($v1)     ; payload from entity
  ; ... testa condições do payload ...
  jal  func_0013EBE0($s0)  ; get next node
  bnel $s0, zero, loop     ; if not null, continue
```

### O que func_0013EB50 faz

```
Entrada:  a0 = descriptor ID (ex: 0x13 = ROPE)
Saída:    v0 = ponteiro para primeiro node/context
          v0 = 0 se não há nodes com este descriptor
```

Comportamento:
- Recebe um ID de descritor
- Retorna o primeiro node/context na lista que corresponde a este ID
- Ou null se não houver nenhum
- Não toma `a1` (ou toma um opcional)

### O que func_0013EBE0 faz

```
Entrada:  a0 = ponteiro para node atual (retornado por func_0013EB50)
Saída:    v0 = ponteiro para próximo node
          v0 = 0 se não há mais nodes
```

Comportamento:
- Recebe o node atual
- Retorna o próximo node na lista para o mesmo descriptor
- Iteração: `v0 = func_0013EBE0(v0)` até v0 == 0

### Em C

```c
// Padrão de uso (func_001D40D8 como exemplo):
void iterate_rope(struct ClothContext *start) {
    struct ClothContext *ctx = func_0013EB50(0x13); // get first ROPE
    while (ctx) {
        struct ClothEntity *ent = ctx->entity;
        struct ClothPayload *pl = ent->payload;
        if (pl->variant == 1 && pl->state_id == 3) {
            if (func_0012A7F8(pl + 0x60)) {
                func_001A6E28(&D_618FC8); // action!
            }
        }
        ctx = func_0013EBE0(ctx); // next
    }
}
```

## Funções que usam este padrão

| Função | Descriptor | Condições | Ação |
|---|---|---|---|
| func_001D40D8 | 0x13 (ROPE) | variant != 1, state_id == 3, + check | callback D_618FC8 |
| func_001D3DD8 | 0x13 (ROPE) | extra_ptr exist, flag_08==0, state_id < 1 | func_0019F530 + reset payload |
| func_001D4170 | 0x13 (ROPE) | extra_ptr exist, flag_08==0, state_id < 1 | func_0019F530 + set payload field |
| func_001D3E80 | 0x13 (ROPE) | (padrão similar) | (float-related) |
| func_001D3F78 | 0x13 (ROPE) | (padrão similar) | (float-related) |

Todas as 5 recebem `a0 = 0x13` antes de chamar func_0013EB50.
O descritor 0x13 é o ROPE (confirmado em Rev.035).

## O que mais chama func_0013EB50/EBE0?

```bash
grep -rn "0013EB50\|0013EBE0" /tmp/ico-cloth-full/asm/cloth_*.s
```

- Apenas cloth_sub functions — consistentes com o padrão.
- Nenhuma função fora do cluster cloth usa este padrão (no splat).

## Por que não tentar decompilar agora

1. As funções dependem de `func_0013EB50`/`func_0013EBE0` que não estão
   no cluster cloth — estão no código genérico de iteração de entidades
2. Sem entender o que elas retornam, o C gerado não vai compilar direito
3. As condições de matching dependem de flags + state_id + campos que
   já entendemos, mas o fluxo de iteração precisa ser modelado

## Como desbloquear

1. **Analisar func_0013EB50 estaticamente** — extrair do monolithic asm
   (`main_text_precloth_0013fd10_001d27a8.s`) e entender o que retorna
2. **Analisar func_0013EBE0 estaticamente** — mesma extração
3. **Se ambas forem simples**, adicionar protótipos C e tentar scratch
4. **Se forem complexas**, marcar como dependentes de runtime

## Próximo passo

Extrair func_0013EB50 e func_0013EBE0 do monolithic asm e analisar.
Se forem simples o suficiente, adicionar ao splat YAML como funções
promovidas e tentar scratch das 5 funções bloqueadas.

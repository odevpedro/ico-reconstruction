# rev.037 — Remaining Callers and the ROPE Registration Gap

## Data

2026-05-13

## Objetivo

Completar o mapeamento dos cinco callers de `0x0013f7a8` iniciado em Rev.036,
analisar as três funções restantes e emitir um veredito sobre o gap de registro
de `0x001d3a30`.

## Escopo

Incluído:

- análise completa das funções `0x00240d40`, `0x00240ea0` e `0x00201e70`;
- revisão dos dois callsites de `0x0013f7a8` dentro de `0x001b76f8`;
- mapeamento de callers de cada função;
- veredito conservador sobre o gap de registro do ROPE.

Excluído:

- gameplay;
- runtime adicional;
- análise das funções chamadas por `0x0013f778` e `0x0013f3f0`;
- prova de que `0x001d3a30` nunca é registrado.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev036-registration-path-survey.md` | os cinco callsites e zero-guard |
| `research/elf/ghidra-rev035-entry-table-and-descriptor-correction.md` | ROPE descriptor_label+0x40 == 0 |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções |

## Revisão dos dois callsites de `0x001b76f8`

Rev.036 documentou o zero-guard em `0x001b7ac4` mas não detalhou o callsite anterior.

Trecho completo:

```asm
0x001b7a90: lhu     v0,+0x40(s4)        -- entry[+0x40] halfword
0x001b7a94: beq     v0,zero,0x001b7aa0  -- skip t0 flag if halfword==0
0x001b7a98: addiu   t0,zero,+0x1800
0x001b7a9c: dsll    t0,v0,10            -- t0 = halfword << 10
0x001b7aa0: lw      a1,+0x24(s4)        -- a1 = entry[+0x24]
0x001b7aa4: beq     a1,zero,0x001b7ac0  -- if entry[+0x24]==0 skip to fallback
0x001b7aa8: daddu   a0,s3,zero
0x001b7aac: daddu   a2,zero,zero
0x001b7ab0: jal     0x0013f7a8          -- register entry[+0x24] as callback
0x001b7ab4: addiu   a3,zero,+0x13
0x001b7ab8: beq     zero,zero,0x001b7ad8
0x001b7abc: lbu     v1,+0x46(s4)
0x001b7ac0: lw      a1,+0x40(s7)        -- a1 = descriptor_label+0x40 (fallback)
0x001b7ac4: beq     a1,zero,0x001b7ad4  -- zero-guard: ROPE skipped here
0x001b7ac8: daddu   a2,zero,zero
0x001b7acc: jal     0x0013f7a8          -- register descriptor_label+0x40
0x001b7ad0: addiu   a3,zero,+0x13
```

Interpretação:

- Callsite `0x001b7ab0` registra `entry[+0x24]` quando não-zero. Isso é um override por entrada.
- Callsite `0x001b7acc` registra `descriptor_label+0x40` como fallback. ROPE tem esse slot zerado, logo SKIPPED.
- Para que ROPE passasse por `0x001b7ab0`, precisaria de uma entrada com `+0x46 == 0x14` e `+0x24 == 0x001d3a30`. Não há evidência estática de tal entrada.

## 1. Função `0x00240d40` — callsite `0x00240e50`

Prologue e inicialização:

```asm
0x00240d40: addiu   sp,sp,-0xa0
0x00240d44: daddu   v0,a0,zero          -- v0 = a0 (descriptor pointer)
0x00240d50: daddu   fp,a1,zero          -- fp = a1
0x00240d58: daddu   s7,a2,zero          -- s7 = a2
0x00240d60: daddu   s6,a3,zero          -- s6 = a3
0x00240d68: daddu   s5,zero,zero        -- s5 = 0 (default)
0x00240d7c: beq     t0,zero,0x00240d88  -- if caller t0==0, skip [v0+0x40]
0x00240d80: sd      s0,0x0(sp)
0x00240d84: lw      s5,+0x40(v0)        -- s5 = [descriptor+0x40] (only when t0!=0)
```

Callsite:

```asm
0x00240e3c: beq     s5,zero,0x00240e58  -- if s5==0, skip
0x00240e40: daddu   a1,s5,zero
0x00240e44: daddu   a0,s4,zero
0x00240e48: daddu   a2,zero,zero
0x00240e4c: addiu   a3,zero,+0x13
0x00240e50: jal     0x0013f7a8
```

Conclusão:

- `s5` = `[descriptor+0x40]` apenas quando o caller passa `t0 != 0`.
- Para ROPE: `descriptor_label+0x40 == 0x00000000` (Rev.035); `s5` seria 0 mesmo que `t0 != 0`.
- ROPE não pode ser registrado por este caminho.

## 2. Função `0x00240ea0` — callsite `0x00240f90`

Prologue:

```asm
0x00240ea0: addiu   sp,sp,-0x80
0x00240eb4: daddu   s0,a1,zero          -- s0 = a1
0x00240ebc: daddu   s6,t0,zero          -- s6 = caller t0 (5th arg)
```

Callsite (após verificações):

```asm
0x00240f7c: beq     s6,zero,0x00240f98  -- skip if s6==0
0x00240f80: daddu   a1,s6,zero          -- a1 = s6 = caller's t0
0x00240f90: jal     0x0013f7a8          -- register caller's t0 as callback
```

Callers conhecidos de `0x00240ea0`:

| Caller | t0 | Callback registrado |
|---|---|---|
| `0x001c3704` | `lui t0,0x001c; addiu t0,t0,+14176` = `0x001c3720` | `0x001c3720` |
| `0x001f22c4` | `lui t0,0x001f; addiu t0,t0,+9072` = `0x001f2390` | `0x001f2390` |

Conclusão:

- O callback é o `t0` passado pelo caller — um literal.
- Os dois callers conhecidos passam `0x001c3720` e `0x001f2390`.
- Nenhum passa `0x001d3a30`.
- ROPE não é registrado por este caminho.

## 3. Função `0x00201e70` — callsite `0x00201ed4`

Corpo completo relevante:

```asm
0x00201e70: addiu   sp,sp,-0x40
0x00201e74: addiu   v0,zero,+0x4c       -- stride = 0x4c
0x00201e80: daddu   s1,a2,zero          -- s1 = a2 (node slot ptr)
0x00201e88: daddu   s0,a0,zero          -- s0 = a0 (object)
0x00201e90: lui     a0,0x002a
0x00201e94: addiu   a0,a0,+0x4c48       -- a0 = 0x002a4c48 (entry table)
0x00201e98: lw      v1,+0x8(s0)         -- v1 = [object+8] (entry index)
0x00201e9c: mult    v1,v1,v0            -- v1 = index * 0x4c (R5900)
0x00201ea0: addu    v1,v1,a0            -- v1 = entry_ptr
0x00201ea4: lhu     v0,+0x40(v1)        -- v0 = entry[+0x40] halfword
0x00201ea8: dsll    t0,v0,10
0x00201eac: bne     t0,zero,0x00201ecc  -- branch if entry[+0x40] != 0
0x00201eb0: lw      s2,+0x0(s1)
0x00201eb4: daddu   a0,s0,zero
0x00201eb8: daddu   a2,zero,zero
0x00201ebc: jal     0x0013f778          -- register a1 (entry[+0x40]==0 variant)
0x00201ec0: addiu   a3,zero,+0x13
0x00201ec4: beq     zero,zero,0x00201ee0
0x00201ec8: sw      v0,+0x0(s1)
0x00201ecc: daddu   a0,s0,zero
0x00201ed0: daddu   a2,zero,zero
0x00201ed4: jal     0x0013f7a8          -- register a1 (entry[+0x40]!=0 variant)
0x00201ed8: addiu   a3,zero,+0x13
```

`a1` é o argumento `a1` original da função — não derivado da tabela de descritores.

Callers de `0x00201e70` e seus `a1`:

| Caller | `a1` na chamada |
|---|---|
| `0x00203080` | `s0` (valor de registrador salvo — proveniência não resolvida estaticamente) |
| `0x00203ea0` | `[s0+4]` (lido de estrutura de dados em runtime) |

Em nenhum caso `a1` é um literal identificável. Ambos dependem de dados de runtime.

Conclusão:

- `0x00201e70` é o único caminho estático restante onde `0x001d3a30` poderia ser registrado.
- A condição de entrada (`entry[+0x40] != 0`) é verificada contra a entrada correspondente ao objeto.
- O callback `a1` vem do caller — não há evidência estática de que algum caller passe `0x001d3a30`.

## Mapa consolidado dos cinco callsites de `0x0013f7a8`

| Callsite | Função | Fonte do callback `a1` | ROPE possível? |
|---|---|---|---|
| `0x001b7ab0` | `0x001b76f8` | `entry[+0x24]` (override por entrada) | Somente se existir entrada com `+0x46==0x14` e `+0x24==0x001d3a30` |
| `0x001b7acc` | `0x001b76f8` | `descriptor_label+0x40` (fallback) | Não: ROPE slot é 0 |
| `0x00240e50` | `0x00240d40` | `descriptor_label+0x40` (gate t0) | Não: ROPE slot é 0 |
| `0x00240f90` | `0x00240ea0` | `t0` do caller (literal) | Não: callers passam `0x001c3720` e `0x001f2390` |
| `0x00201ed4` | `0x00201e70` | `a1` do caller (runtime) | Não resolvido estaticamente |

## O que fica confirmado

1. `0x001b76f8` tem dois callsites distintos para `0x0013f7a8`: um via `entry[+0x24]` e outro via `descriptor_label+0x40`.
2. O fallback via `descriptor_label+0x40` é definitivamente pulado para ROPE (slot = 0).
3. `0x00240d40` não pode registrar ROPE porque `ROPE descriptor_label+0x40 == 0`.
4. `0x00240ea0` registra somente `0x001c3720` e `0x001f2390` pelos callers conhecidos.
5. `0x00201e70` usa o argumento `a1` do caller — proveniência runtime em ambos os callsites.

## O que fica provável

1. A registração de `0x001d3a30` ocorre em runtime, possivelmente via `entry[+0x24]` ou via `0x00201e70`, com o valor determinado por dados de sala/layout carregados dinamicamente.
2. A entrada ROPE na tabela `0x002a4c48` pode existir em runtime (não apenas nos dados estáticos) com `+0x24 == 0x001d3a30`.

## O que fica possível

1. `0x00201e70` é chamado em algum contexto que passa `a1 = 0x001d3a30`.
2. Um caller de `0x00240ea0` não mapeado passa `t0 = 0x001d3a30`.
3. Dados de sala preenchem `entry[+0x24]` com `0x001d3a30` para entradas ROPE geradas em runtime.

## O que permanece desconhecido

1. O valor de `a1` nos dois callers de `0x00201e70` em runtime.
2. Se algum caller de `0x00203080` ou `0x00203ea0` é ativado para objetos com descritor ROPE.
3. Se existem mais callers de `0x00240ea0` além dos dois mapeados.
4. O mecanismo exato pelo qual `0x001d3a30` entra em `node +0x1c` no runtime observado em Rev.025.

## O que é descartado

1. `0x001b7acc` como caminho de registração de ROPE (zero definitivo no slot).
2. `0x00240d40` como caminho de registração de ROPE (zero definitivo no slot).
3. `0x00240ea0` como caminho de registração de ROPE via callers `0x001c3704` e `0x001f22c4`.

## Próximo teste mínimo

Sem gameplay:

1. Mapear todos os callers de `0x00240ea0` (não só os dois encontrados) para verificar se algum passa `t0 = 0x001d3a30`.
2. Mapear os callers dos callers de `0x00203080` e `0x00203ea0` para rastrear a proveniência de `s0` e `[s0+4]`.
3. Se runtime for retomado: capturar `a1` e `a0` na entrada de `0x0013f7a8` quando `a3 == 0x13` para confirmar qual callback é realmente registrado no caso ROPE.
4. Verificar se dados de sala ou função de carga preenchem `entry[+0x24]` com ponteiros de callback.

## Veredito conservador

Rev.037 fecha o mapeamento estático dos cinco callsites de `0x0013f7a8`. Três caminhos são definitivamente excluídos para ROPE. O caminho via `entry[+0x24]` em `0x001b76f8` e o caminho via `0x00201e70` são candidatos vivos mas requerem runtime para confirmação. O gap entre o callback confirmado `0x001d3a30` e seu registro em `node +0x1c` permanece aberto — mas as opções estáticas foram esgotadas.

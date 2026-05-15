# Cloth Cluster Scratch Candidates

## Date

2026-05-15

## Resumo executivo

Classificação das 22 funções do cluster cloth (0x1d27a8–0x1d45b0) por
viabilidade de matching C usando o GCC 2.95.2 PS2 Linux com flags ICP.
O objetivo é orientar a ordem de scratches: começar pelo mais simples,
evitar armadilhas (jump table, runtime dependency), e construir confiança
no pipeline antes de atacar funções complexas.

## Critérios de classificação

| Categoria | Critério |
|---|---|
| `first_scratch_candidate` | Leaf function, sem jump table, sem float, ≤ 64B, sem dependência de runtime |
| `good_small_candidate` | Sem jump table, poucos branches, tamanho moderado (64-200B) |
| `possible_later` | Sem jump table, mas maior (200-600B) ou com float |
| `avoid_for_now_jump_table` | Contém jump table (GCC 2.95.2 não reproduz .word 4B) |
| `avoid_for_now_runtime_dependent` | Depende de runtime para entender argumentos/estruturas |
| `keep_as_asm` | Candidato pobre para matching C (tail call, gp-rel, etc.) |
| `needs_more_context` | Muito grande ou complexo para abordar sem mais contexto |

## Tabela das 22 funções

### Cluster cloth (22 funções em `splat/SCUS_971.13.cloth-full.yaml`)

| # | Nome | VA | Bytes | JT | DIR | IND | FLT | BR | LEAF | L/S | Classificação |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `cloth_sub_001d3d70` | 0x1d3d70 | 16 | N | N | N | N | N | Y | 3/0 | **first_scratch_candidate** |
| 2 | `cloth_sub_001d3d80` | 0x1d3d80 | 24 | N | N | N | N | N | Y | 3/0 | **first_scratch_candidate** |
| 3 | `cloth_sub_001d3d98` | 0x1d3d98 | 24 | N | N | N | N | N | Y | 3/0 | **first_scratch_candidate** |
| 4 | `cloth_sub_001d3db0` | 0x1d3db0 | 40 | N | N | N | N | Y | Y | 4/0 | **first_scratch_candidate** |
| 5 | `cloth_sub_001d40a0` | 0x1d40a0 | 56 | N | N | N | N | Y | Y | 4/0 | **first_scratch_candidate** |
| 6 | `cloth_sub_001d3d40` | 0x1d3d40 | 48 | N | N | N | N | Y | Y | 5/0 | **first_scratch_candidate** |
| 7 | `cloth_sub_001d4348` | 0x1d4348 | 16 | N | N | N | N | N | Y | 1/0 | **keep_as_asm** (tail call `j`, gp-rel) |
| 8 | `cloth_sub_001d40d8` | 0x1d40d8 | 152 | N | Y | N | N | Y | N | 9/4 | **good_small_candidate** |
| 9 | `cloth_sub_001d4358` | 0x1d4358 | 160 | N | Y | N | N | N | N | 13/3 | **good_small_candidate** |
| 10 | `cloth_sub_001d3dd8` | 0x1d3dd8 | 168 | N | Y | N | N | Y | N | 11/6 | **good_small_candidate** |
| 11 | `cloth_sub_001d4170` | 0x1d4170 | 184 | N | Y | N | N | Y | N | 12/7 | **good_small_candidate** |
| 12 | `cloth_aux_001d3b28` | 0x1d3b28 | 200 | N | Y | N | N | Y | N | 16/6 | **possible_later** |
| 13 | `cloth_sub_001d3e80` | 0x1d3e80 | 248 | N | Y | N | Y | Y | N | 12/8 | **possible_later** |
| 14 | `cloth_update_cb_001d3a30` | 0x1d3a30 | 248 | N | Y | N | N | Y | N | 20/8 | **possible_later** |
| 15 | `cloth_sub_001d4228` | 0x1d4228 | 288 | N | Y | N | N | Y | N | 25/5 | **possible_later** |
| 16 | `cloth_sub_001d3f78` | 0x1d3f78 | 296 | N | Y | N | Y | Y | N | 17/13 | **possible_later** |
| 17 | `cloth_sub_001d3bf0` | 0x1d3bf0 | 336 | N | Y | N | Y | Y | N | 17/11 | **possible_later** |
| 18 | `cloth_sub_001d43f8` | 0x1d43f8 | 440 | N | Y | N | N | Y | N | 38/6 | **possible_later** |
| 19 | `cloth_payload_init_001d27a8` | 0x1d27a8 | 528 | N | Y | N | Y | Y | N | 35/30 | **avoid_for_now_runtime_dependent** |
| 20 | `cloth_setup_001d29b8` | 0x1d29b8 | 568 | N | Y | N | Y | Y | N | 37/9 | **needs_more_context** |
| 21 | `cloth_dispatcher_001d37c8` | 0x1d37c8 | 616 | Y | Y | N | Y | Y | N | 23/22 | **avoid_for_now_jump_table** |
| 22 | `cloth_sim_001d2bf0` | 0x1d2bf0 | 3032 | N | Y | N | Y | Y | N | 145/67 | **needs_more_context** |

### Funções de callback (separadas do cluster, referência)

| Nome | VA | Bytes | Nota |
|---|---|---|---|
| `callback_register` | 0x13f7a8 | 48 | Wrapper bem entendido, sem jump table |
| `callback_storage` | 0x13f3f0 | 584 | Complexo, armazena callback em node |
| `cb48_dispatcher` | 0x13fc00 | 272 | **Tem indirect call (jalr)**, não tentar agora |

## Melhores primeiros alvos

### 1. `func_001D3D70` (16B) — ✅ PRIORIDADE MÁXIMA
```
lw $v0, 0x15C($a0)
lw $v1, 0x800($v0)
jr $ra
lw $v0, 0x4($v1)
```
Accessor puro: carrega entity→payload→variant. Sem branches, sem float,
sem chamadas. Ideal para validar o pipeline de scratch.

### 2. `func_001D3D80` (24B)
```
lw $v0, 0x15C($a0)
lw $v1, 0x800($v0)
lw $v0, 0x0($v1)
jr $ra
sltiu $v0, $v0, 1
```
Accessor com comparação: retorna `(payload->field_00 == 0)`. Leve variação
do padrão 001D3D70.

### 3. `func_001D3D98` (24B)
```
lw $v0, 0x15C($a0)
lw $v1, 0x800($v0)
lw $v0, 0x48($v1)
xori $v0, $v0, 2
jr $ra
sltiu $v0, $v0, 1
```
Accessor: retorna `(variant ^ 2) == 0` → `variant == 2`.

### 4. `func_001D3DB0` (40B)
```
lw $v0, 0x15C($a0)
lw $v1, 0x800($v0)
lw $v0, 0x48($v1)
sltiu $v0, $v0, 5  ; bounds check (state < 5?)
jr $ra
```
Accessor com bounds check: retorna `(variant < 5)`.

### 5. `func_001D3D40` (48B)
Accessor com condicional: testa contexto + payload, decide se variante
é válida.

### 6. `func_001D40A0` (56B)
Accessor com condicional.

## Funções a evitar por enquanto

### Por jump table
- `cloth_dispatcher_001d37c8` — usa jump table com `.word` (4B entries),
  GCC 2.95.2 sempre gera `.dword` (8B). Non-matching até encontrar
  solução para lowering.

### Por runtime dependency
- `cloth_payload_init_001d27a8` — depende de runtime para entender
  a origem do `a1` (gap principal do projeto).

### Por ser melhor manter como asm
- `cloth_sub_001d4348` — thunk/tail call com GP-relative addressing,
  melhor manter como asm.

### Por precisar de mais contexto
- `cloth_setup_001d29b8` (568B) — setup complexo, sem contexto de struct
- `cloth_sim_001d2bf0` (3032B) — maior função do cluster, contém
  possivelmente InitCloth4D/GetCloth4D como labels internos

## Funções runtime-dependent

Apenas uma função depende criticamente de runtime:

| Função | Motivo |
|---|---|
| `cloth_payload_init_001d27a8` | Origem do `a1` não resolvida estaticamente |

Depois que o runtime resolver `a1`, esta função se torna um alvo
`possible_later` ou `good_small_candidate`.

## Ordem recomendada de scratches

```
Ordem  Função             Tam  Justificativa
─────  ─────────────────  ───  ─────────────────────────────────────
1º     func_001D3D70      16B  Accessor puro, 4 instr, pipeline proof
2º     func_001D3D80      24B  Accessor com sltiu, variação do padrão
3º     func_001D3D98      24B  Accessor com xori, variação do padrão
4º     func_001D3DB0      40B  Accessor com bounds check
5º     func_001D3D40      48B  Accessor com condicional
6º     func_001D40A0      56B  Accessor com condicional
7º     func_001D40D8     152B  Primeira com chamada direta (jal)
8º     func_001D4358     160B  Sem branches, chamadas diretas
9-16   (demais)           —    Possível later ou após runtime
```

## Veredito

```
Melhor primeiro alvo:   func_001D3D70
Primeiros scratches:    6 funções (16-64B, leaf, sem jump table)
Segunda leva:           4 funções (152-200B, com jal, sem jump table)
Aguardar runtime:       1 função (cloth_payload_init)
Evitar por JT:          1 função (cloth_dispatcher)
Manter como asm:        1 função (cloth_sub_001d4348)
Precisa mais contexto:  2 funções (cloth_setup, cloth_sim)
```

A ordem proposta permite escalar a dificuldade gradualmente: começar com
accessors puros (4-6 instruções), depois adicionar branches, depois
chamadas de função, e só então atacar funções maiores. O dispatcher e
o cloth_payload_init ficam para depois — um por JT, outro por runtime.

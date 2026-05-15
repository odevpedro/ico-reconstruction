# Cloth Functions Hold List

## Date

2026-05-15

## Resumo executivo

Triagem definitiva das 22 funções do cluster cloth, classificando quais
devem ser priorizadas, quais devem ser evitadas agora, e por quê. O
objetivo é evitar retrabalho e manter o foco em funções com alta
probabilidade de near-match.

## Categorias

| Categoria | O que fazer |
|---|---|
| `accessor_validated` | Já testado, near-match confirmado. Usar como template |
| `good_candidate` | Próximo alvo — características similares aos validados |
| `needs_state_resolver` | Bloqueado por func_0013EB50/EBE0 — entender iterator primeiro |
| `needs_runtime` | Depende de runtime para entender argumentos/estruturas |
| `avoid_jump_table` | Jump table com .word — GCC não reproduz |
| `needs_context` | Muito grande ou complexo para abordar agora |
| `keep_asm` | Melhor manter como assembly (thunk, gp-rel) |

## Tabela das 22 funções

### ✅ Accessor validated (6)

| Função | Bytes | Status | Próximo passo |
|---|---|---|---|
| `func_001D3D70` | 16 | ✅ NEAR-MATCH | Template para novos scratches |
| `func_001D3D80` | 24 | ✅ NEAR-MATCH | Template |
| `func_001D3D98` | 24 | ✅ NEAR-MATCH | Template |
| `func_001D3DB0` | 40 | ✅ NEAR-MATCH | Template |
| `func_001D3D40` | 48 | ✅ NEAR-MATCH | Template |
| `func_001D40A0` | 56 | ✅ NEAR-MATCH | Template |

### 🔜 Good candidate (2)

| Função | Bytes | Motivo |
|---|---|---|
| `func_001D4358` | 160 | Sem branches, chamadas diretas para func_00111918 (pack color) |
| `func_001D3BF0` | 336 | Tem float e chamadas, mas sem jump table — pode ser próximo |

### ⏸ Needs state resolver (3)

| Função | Bytes | Chamadas |
|---|---|---|
| `func_001D40D8` | 152 | `func_0013EB50`, `func_0013EBE0`, `func_0012A7F8`, `func_001A6E28` |
| `func_001D3DD8` | 168 | `func_0013EB50`, `func_0013EBE0`, `func_0019F530` |
| `func_001D4170` | 184 | `func_0013EB50`, `func_0013EBE0`, `func_0019F530` |

Todas as 3 compartilham o padrão:
1. `a0 = 0x13` (ROPE descriptor ID)
2. `jal func_0013EB50` (state resolver — obtém primeiro node)
3. Loop: itera nodes, testa condições
4. `jal func_0013EBE0` (state resolver — obtém próximo node)

**Pré-requisito**: entender o que `func_0013EB50` e `func_0013EBE0`
retornam e como a iteração funciona.

### ⏸ Needs runtime (1)

| Função | Bytes | Motivo |
|---|---|---|
| `cloth_payload_init_001d27a8` | 528 | `a1` não resolvido estaticamente |

### ❌ Avoid jump table (1)

| Função | Bytes | Motivo |
|---|---|---|
| `cloth_dispatcher_001d37c8` | 616 | Jump table com `.word` (4B) — GCC 2.95.2 gera `.dword` (8B) |

### ❌ Keep asm (1)

| Função | Bytes | Motivo |
|---|---|---|
| `func_001D4348` | 16 | Thunk: tail call `j` com GP-relative addressing |

### ⏸ Needs more context (8)

| Função | Bytes | Motivo |
|---|---|---|
| `cloth_payload_init` | 528 | Runtime-dependent (a1) |
| `cloth_setup_001d29b8` | 568 | Setup complexo, sem contexto de structs |
| `cloth_sim_001d2bf0` | 3032 | Maior função — contém possíveis inlines |
| `cloth_sub_001d3e80` | 248 | Float, chamadas diretas |
| `cloth_sub_001d3f78` | 296 | Float, chamadas diretas |
| `cloth_sub_001d4228` | 288 | Múltiplas chamadas diretas |
| `cloth_sub_001d43f8` | 440 | Múltiplas chamadas diretas |
| `cloth_aux_001d3b28` | 200 | Chamadas para funções do domínio cloth |
| `cloth_update_cb_001d3a30` | 248 | Chamada para dispatcher (callback chain) |

## Ordem recomendada de scratches

```
Prioridade  Função        Motivo
─────────────────────────────────────────────────────────────
1º          func_001D4358  Sem branches, chamada externa conhecida
2º          func_001D3BF0  Primeira com float e chamadas
3º-5º      (state resolver)  → Após entender func_0013EB50/EBE0
6º+         (demais)       → Após contexto adicional
```

## Matriz visual

```
                     JT   IND   DIR  FLT  BR   LEAF  TAM  STATUS
3D70  16B  accessor  -    -     -    -    -    Y     16   ✅ DONE
3D80  24B  accessor  -    -     -    -    -    Y     24   ✅ DONE
3D98  24B  accessor  -    -     -    -    -    Y     24   ✅ DONE
3DB0  40B  accessor  -    -     -    -    Y    Y     40   ✅ DONE
3D40  48B  accessor  -    -     -    -    Y    Y     48   ✅ DONE
40A0  56B  accessor  -    -     -    -    Y    Y     56   ✅ DONE
─────────────────────────────────────────────────────────────
4358  160B packcol   -    -     Y    -    -    -     160  🔜 NEXT
3BF0  336B cloth_sub -    -     Y    Y    Y    -     336  🔜 NEXT
─────────────────────────────────────────────────────────────
40D8  152B iterator  -    -     Y    -    Y    -     152  ⏸ RESOLVER
3DD8  168B iterator  -    -     Y    -    Y    -     168  ⏸ RESOLVER
4170  184B iterator  -    -     Y    -    Y    -     184  ⏸ RESOLVER
─────────────────────────────────────────────────────────────
27A8  528B init      -    -     Y    Y    Y    -     528  ⏸ RUNTIME
─────────────────────────────────────────────────────────────
37C8  616B dispatch  Y    -     Y    Y    Y    -     616  ❌ JT
4348  16B  thunk     -    -     -    -    -    Y     16   ❌ ASM
─────────────────────────────────────────────────────────────
29B8  568B setup     -    -     Y    Y    Y    -     568  ⏸ CTX
2BF0  3032B sim      -    -     Y    Y    Y    -    3032  ⏸ CTX
3E80  248B sub       -    -     Y    Y    Y    -     248  ⏸ CTX
3F78  296B sub       -    -     Y    Y    Y    -     296  ⏸ CTX
4228  288B sub       -    -     Y    Y    Y    -     288  ⏸ CTX
43F8  440B sub       -    -     Y    Y    Y    -     440  ⏸ CTX
3B28  200B aux       -    -     Y    -    Y    -     200  ⏸ CTX
3A30  248B cb        -    -     Y    -    Y    -     248  ⏸ CTX
```

## Regras para mover de status

| De | Para | Requisito |
|---|---|---|
| `needs_state_resolver` | `good_candidate` | func_0013EB50/EBE0 entendidas |
| `needs_runtime` | `good_candidate` | a1 capturado no PCSX2 |
| `needs_context` | `good_candidate` | Structs validadas + funções vizinhas decompiladas |
| `avoid_jump_table` | `good_candidate` | Compilador que reproduza .word (não temos) |

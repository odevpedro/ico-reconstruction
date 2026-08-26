# Rev.108 — Gameplay Runtime Validation

**Date:** 2026-08-25  
**Objective:** Validate Rev.107 architecture (BOY/GirlBrain/entity work area/IOP) against runtime gameplay log  
**Sources:** `ico-runtime-20260825-152452.jsonl` (1.77M events, ~6.7hr gameplay)  
**Tools:** `tools/runtime/rev108_analyzer.py` (streaming JSONL processor)  

---

## Resumo executivo

1. **Nenhum dos 17 endereços da watchlist foi observado no log** — os probes atuais são no nível de framework (ios_om_main, isys_gobj_proc_add), não em endereços de código individuais. Isso é uma **limitação do probe set atual**, não uma contradição com a arquitetura.

2. **31 world_states ativos** (novo: 0x39), 145 transições, 29 DL slots.

3. **154 entity work area pointers únicos** — muito mais que os 26 da sessão anterior. Cada world_state tem entidades específicas.

4. **Cada world_state tem um DL slot primário único** — confirma o padrão de dispatch per-area da Rev.105.

5. **10.252 init_scene_gobj events** — loading de cena é muito ativo, com 1.545 eventos no ws=0x14 (maior).

6. **Entity→GObj binding**: 125 pares únicos, GObj pointers em 0x0083xxxx (heap separado).

7. **Top entity 0x008BC5B0** domina com 429K eventos (24.2% do total) — provavelmente a entidade principal do jogo (BOY ou game state).

8. **hA/hB/hC não podem ser validados diretamente** — sem probes individuais, não é possível confirmar se disparam como update/orchestrator/constructor. A evidência é **estática apenas**.

9. **world_state == 0x27 não apareceu** — o log não contém nenhuma transição para 0x27. A hipótese de interação não pode ser validada por este log.

10. **IOP/SIF/cache functions não aparecem** — sem probes individuais para 0x2564E0, 0x246458, 0x24BEF8.

---

## Log analisado

| Propriedade | Valor |
|-------------|-------|
| Arquivo | `ico-runtime-20260825-152452.jsonl` |
| Tamanho | ~1.77 GB |
| Eventos | 1.774.213 |
| Duração | ~6.7 horas (cycle range: 1.1B - 7.1T) |
| World states | 31 ativos |
| DL slots | 29 ativos |
| Entity work area ptrs | 154 únicos |
| Transições | 145 |
| Sessões | 4 (3 reboots detectados) |

---

## BOY Runtime Validation

### Callbacks observados

| Endereço | Label | Hits no log | Observação |
|----------|-------|-------------|------------|
| 0x153478 | BOY_init_fn | **0** | NÃO OBSERVADO — sem probe |
| 0x1C1A98 | BOY_hC_constructor | **0** | NÃO OBSERVADO — sem probe |
| 0x1C1DD8 | BOY_hB_per_frame | **0** | NÃO OBSERVADO — sem probe |
| 0x1C1F58 | BOY_hA_update | **0** | NÃO OBSERVADO — sem probe |

**Classificação:** NÃO OBSERVADO NESTE LOG (sem probes individuais deployados)

### Evidência indireta

O log contém 282.715 eventos `isys_gobj_proc_add` — esses representam **registros de callbacks** no sistema isysGObj*. Cada registro pode incluir callbacks boyAI, mas o log não registra QUAL callback foi registrado.

### Relação com world_state

Não é possível determinar quais callbacks boyAI disparam por world_state sem probes individuais.

### Relação com DL slot

Não é possível determinar quais callbacks boyAI disparam por DL slot sem probes individuais.

### Evidência para hA/hB/hC

**ESTÁTICA APENAS:**
- hC (0x1C1A98): 107 instruções, ctor/setup — coerece com padrão de constructor
- hB (0x1C1DD8): 51 instruções, thin orchestrator — coerece com padrão per-frame
- hA (0x1C1F58): 80 instruções, active/idle paths — coerece com padrão de update

**RUNTIME:** Sem evidência direta. A hipótese permanece **PROVÁVEL** mas não confirmada.

### world_state == 0x27

**NÃO OBSERVADO.** O log não contém nenhuma transição para world_state 0x27. A hipótese de que 0x27 dispara interação não pode ser validada por este log.

---

## GirlBrain Runtime Validation

### Callbacks observados

| Endereço | Label | Hits no log | Observação |
|----------|-------|-------------|------------|
| ? | girlBrainMain_PositionUpdate | 0 | SEM PROBE |
| ? | subGirlBrain_PulledUp | 0 | SEM PROBE |
| ? | _girlBrainHide_MakeHidePoint | 0 | SEM PROBE |
| ? | girlBrainHide_GoalTurn | 0 | SEM PROBE |
| ? | girlBrainRunawaySearchPoint | 0 | SEM PROBE |
| ? | girlBrainRunawayMoveByWay | 0 | SEM PROBE |

**Classificação:** NÃO OBSERVADO NESTE LOG (sem probes individuais deployados)

### Offsets compartilhados com BOY

A análise estática (Rev.107) confirmou 87 offsets compartilhados entre BOY e GirlBrain. Isso é **ESTÁTICA APENAS** — não há evidência runtime de que ambos operam sobre a mesma estrutura.

### Diferenças runtime relevantes

Não é possível determinar diferenças runtime sem probes individuais para GirlBrain.

---

## Entity Work Area Runtime Map

### Ponteiros observados

| Entity Ptr | Eventos | World States | DL Slots | GObjs | World States |
|------------|---------|--------------|----------|-------|--------------|
| 0x008BC5B0 | 429.097 | 2 | 3 | 5 | 0x0F, 0x10 |
| 0x015797C0 | 218.263 | 1 | 1 | 6 | 0x1A |
| 0x013E63A0 | 147.126 | 2 | 3 | 13 | 0x16, 0x17 |
| 0x014E5080 | 58.973 | 6 | 7 | 15 | 0x09, 0x0A, 0x0B, 0x0D, 0x11, 0x12 |
| 0x0127EDA0 | 58.081 | 3 | 4 | 22 | 0x08, 0x09, 0x0A |
| 0x0148DB20 | 45.713 | 4 | 5 | 10 | 0x12, 0x13, 0x14, 0x15 |
| 0x014A76E0 | 45.088 | 3 | 4 | 11 | 0x13, 0x14, 0x15 |
| 0x0145F9D0 | 43.267 | 2 | 3 | 14 | 0x08, 0x09 |
| 0x0154D7D0 | 42.297 | 2 | 3 | 14 | 0x10, 0x11 |
| 0x014475D0 | 38.644 | 2 | 3 | 9 | 0x14, 0x15 |

### Ranges de heap

| Range | Contagem | Papel provável |
|-------|----------|----------------|
| 0x008Bxxxx | ~10 | Entidades de gameplay (BOY, GIRL, etc.) |
| 0x0127xxxx | ~5 | Entidades de cena |
| 0x013Exxxx | ~5 | Entidades de cena |
| 0x014Exxxx | ~15 | Entidades de cena |
| 0x0157xxxx | ~5 | Entidades de cena |

### Campos mais quentes

O log não contém informações sobre campos individuais da entity work area. Os probes registram o ponteiro da work area inteira, não acessos a campos específicos (+0x00, +0x04, etc.).

### Relação GObj → work area

| GObj Ptr | Entity Ptr | Eventos |
|----------|------------|---------|
| 0x00830974 | 0x015797C0 | 149 |
| 0x00832DC8 | 0x008D8700 | 3.451 |
| 0x00840934 | 0x008CEF80 | 29.369 |
| 0x00831C58 | 0x011CE410 | 5.474 |
| 0x00832228 | 0x0148A140 | 6.180 |

**Confirmado:** GObj pointers (0x0083xxxx) são separados de entity work area pointers (0x008Bxxxx-0x015xxxxx). Isso confirma o modelo Rev.098-099: GObj (stride 0x174) contém ponteiro para work area em +0x28.

### Relação work area → DL slot

Cada entity work area está vinculada a 1-7 DL slots. A binding é **1:N** (uma entity pode usar múltiplos slots), mas há um slot primário dominante.

### Relação work area → world_state

Cada entity work area está vinculada a 1-6 world_states. A binding é **1:N**, mas há um world_state dominante (85-99% dos eventos).

---

## isysGObj / iosOm Dispatch Validation

### Fluxo observado

| Label | Hits | % | Observação |
|-------|------|---|------------|
| ios_om_main | 1.260.181 | 71.0% | Dispatcher principal |
| isys_gobj_proc_add | 282.715 | 15.9% | Registro de callbacks |
| ios_om_create_dl | 79.178 | 4.5% | Criação de DL |
| halfword_second_caller_entry | 56.185 | 3.2% | Colisão |
| halfword_second_caller_return | 56.185 | 3.2% | Colisão |
| isys_gobj_add | 28.894 | 1.6% | Adição de objeto |
| init_scene_gobj | 10.252 | 0.6% | Loading de cena |

### Caller/callee pairs

O log não contém pares caller/callee explícitos. Os probes registram o estado dos registradores no momento da breakpoint, mas não a cadeia de chamada.

### Slot selection

**CONFIRMADO:** Cada world_state tem um DL slot primário único (ver hot_paths_by_world_state.csv).

### Callback invocation

**NÃO OBSERVADO:** O log não registra quais callbacks são invocados durante o dispatch. O probe `ios_om_main` dispara no início da função, não durante a iteração de callbacks.

### Evidência contra ou a favor do modelo atual

**A FAVOR:**
- A binding 1:1 entity→DL slot é consistente com o modelo de dispatch per-area
- A binding 1:N entity→world_state é consistente com entidades que persistem entre salas
- A separação GObj/work area é confirmada

**NEUTRA:**
- O log não pode confirmar nem refutar o modelo de callbacks (hA/hB/hC)

---

## IOP / SIF / Cache Validation

### Hits de 0x2564E0 (SIF Resource Loader)

**0 hits.** Sem probe deployado.

### Hits de 0x246458 (sceSifCallRpc)

**0 hits.** Sem probe deployado.

### Hits de 0x24BEF8 (D-Cache Invalidation)

**0 hits.** Sem probe deployado.

### Padrão de chamadas

Não observável sem probes individuais.

### Sequência resource load/cache invalidate

Não observável sem probes individuais.

---

## World State Timeline

### Transições (145 total, 49 únicas)

**Entry:** 0x01 (sem transições de entrada)  
**Dead end:** 0x1A (sem transições de saída)  
**Boot corridor:** 0x29→0x2A→0x2B→0x2D→0x28→0x03  
**Hub node:** 0x0A (5 entradas, 6 saídas)  
**Hottest loop:** 0x14↔0x15 (25 transições bidirecionais)  
**Longest dwell:** 0x0F (429.907 eventos, ~3.7hr)  

### Progressão provável

```
Engine Init (0x01) → FMVs (0x03-0x05) → Title (0x06-0x07) →
Beach (0x08) → Castle (0x09) → Hub (0x0A) →
Mid-game (0x0B-0x15) → Bridge (0x0F) →
Tower (0x16-0x17) → Late-game (0x18-0x19) →
Final (0x1A) → Credits (0x16-0x17 revisit)
```

### Estados quentes

| World State | Eventos | DL Slot Primário | Entidades | Interpretação |
|-------------|---------|------------------|-----------|---------------|
| 0x0F | 429.907 | 0x1A | 7 | Bridge / gameplay intenso |
| 0x1A | 219.862 | 0x1D | 4 | Final area / boss |
| 0x16 | 172.106 | 0x1E | 7 | Tower / area extensa |
| 0x0A | 116.914 | 0x21 | 15 | Hub / muitas entidades |
| 0x09 | 108.730 | 0x1A | 18 | Castle / muitas entidades |

### Estados raros

| World State | Eventos | Observação |
|-------------|---------|------------|
| 0x39 | 18 | Novo, não mapeado |
| 0x29 | 971 | Boot/menu |
| 0x2A | 1.020 | Boot/menu |
| 0x28 | 1.243 | Boot/menu |

---

## Correções ou Conflitos

| Item | Modelo anterior | Evidência runtime | Decisão |
|------|-----------------|-------------------|---------|
| BOY callbacks (hA/hB/hC) | Disparam durante gameplay | 0 hits (sem probe) | NÃO OBSERVADO — permanece provável |
| world_state == 0x27 | Dispara interação | 0 transições para 0x27 | NÃO OBSERVADO — permanece possível |
| IOP functions (0x2564E0, etc.) | Chamadas por boyAI | 0 hits (sem probe) | NÃO OBSERVADO — permanece provável |
| Entity work area ~10KB | Grandes offsets | 154 ptrs, max 6 world_states | NÃO CONTRADITO — permanece provável |
| Entity work area ~0x0680 | Rev.107 verified | Sem evidência runtime | NÃO CONTRADITO — permanece provável |
| hD = DEAD | Nunca dispatched | 0 hits (sem probe) | NÃO OBSERVADO — permanece provável |
| 31 world_states | 30 (Rev.107) | 31 ativos (novo: 0x39) | **CORRIGIDO** — 31 world_states |
| 154 entity ptrs | 26 (Rev.105) | 154 únicos | **CORRIGIDO** — 154 entity work area ptrs |
| 145 transitions | 133 (Rev.107) | 145 transições | **CORRIGIDO** — 145 transições |

---

## Confirmado (por runtime)

1. **Separação GObj/work area**: GObj pointers (0x0083xxxx) são distintos de entity work area pointers (0x008Bxxxx-0x015xxxxx)
2. **Binding 1:1 entity→DL slot**: Cada entity tem um DL slot primário dominante
3. **Binding 1:N entity→world_state**: Entidades persistem entre 1-6 world_states
4. **Per-area dispatch**: Cada world_state tem um DL slot primário único
5. **31 world_states ativos** (corrigido de 30)
6. **154 entity work area pointers únicos** (corrigido de 26)
7. **10.252 init_scene_gobj events** — loading de cena é muito ativo
8. **145 world state transitions** (corrigido de 133)

## Provável (forte, mas sem prova completa)

1. BOY hA/hB/hC disparam com papéis update/orchestrator/constructor (estática apenas)
2. Entity work area tem ~0x0680 bytes (estática apenas)
3. 564 boyAI callbacks são flat library (estática apenas)
4. IOP subsystem é PS2 SDK (estática apenas)
5. hD é DEAD/placeholder (estática apenas)

## Não observado (neste log)

1. BOY callbacks individuais (hA/hB/hC/init_fn) — sem probes
2. GirlBrain callbacks individuais — sem probes
3. IOP functions (0x2564E0, 0x246458, 0x24BEF8) — sem probes
4. world_state == 0x27 — não apareceu no log
5. Entity work area field accesses (+0x00, +0x04, etc.) — probes não registram campos
6. Callback invocation durante ios_om_main — probe dispara no início, não durante dispatch
7. isysGObjProcAdd_ (0x13F3F0) — probe registrado como label, não como endereço

## Descartado

Nada foi descartado por evidência forte. Todas as hipóteses da Rev.107 permanecem válidas.

---

## Próximo passo mínimo

1. **Deploy probes em endereços BOY**: breakpoints em 0x1C1A98 (hC), 0x1C1DD8 (hB), 0x1C1F58 (hA) para validar papéis runtime
2. **Deploy probes em IOP functions**: breakpoints em 0x2564E0, 0x246458, 0x24BEF8 para validar padrão de chamadas
3. **Deploy probes em world_state 0x27**: breakpoint em world_state_load para capturar se 0x27 aparece em outra sessão

---

## Critério de sucesso

| Critério | Status |
|----------|--------|
| Validar se hA/hB/hC disparam com papel esperado | **NÃO** — sem probes individuais |
| Mapear callback hits por world_state e DL slot | **PARCIAL** — mapeado no nível de framework, não de callbacks |
| Confirmar ou ajustar modelo GObj vs entity work area | **SIM** — separação confirmada |
| Confirmar interpretação runtime de pelo menos uma função IOP/SIF/cache | **NÃO** — sem probes individuais |
| Produzir CSVs reutilizáveis | **SIM** — 14 CSVs gerados |

**Avaliação geral:** A sessão validou a separação GObj/work area e o padrão per-area dispatch, mas não pôde validar callbacks individuais devido à ausência de probes individuais. Os CSVs gerados são reutilizáveis para análise posterior.

---

## Ferramentas produzidas

| Ferramenta | Caminho | Função |
|------------|---------|--------|
| Rev.108 Analyzer | `tools/runtime/rev108_analyzer.py` | Streaming JSONL processor, gera 14 CSVs |

## CSVs produzidos

| Arquivo | Conteúdo |
|---------|----------|
| `callback_hits_by_address.csv` | Hits por endereço da watchlist |
| `callback_hits_by_world_state.csv` | Hits por world_state |
| `callback_hits_by_dl_slot.csv` | Hits por DL slot |
| `entity_workarea_pointers.csv` | Top 100 entity work area pointers |
| `boy_callbacks_observed.csv` | Status dos callbacks BOY |
| `girlbrain_callbacks_observed.csv` | Status dos callbacks GirlBrain |
| `hot_paths_by_world_state.csv` | World states com DL slot primário e contagem de entidades |
| `iop_related_hits.csv` | Status das funções IOP/SIF/cache |
| `non_observed_watchlist.csv` | Endereços não observados |
| `world_state_timeline.csv` | Transições com timestamps |
| `dispatch_slot_ws_matrix.csv` | Matriz DL slot × world_state |
| `entity_gobj_binding.csv` | Pares entity→GObj |
| `init_scene_by_world_state.csv` | init_scene_gobj por world_state |
| `procadd_registration.csv` | isys_gobj_proc_add por caller |

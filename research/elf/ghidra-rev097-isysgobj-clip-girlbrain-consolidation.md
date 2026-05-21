# Rev.097 — isysGObj, _Clip Correction and GirlBrain Surface Map

**Date:** 2026-05-21

---

## Resumo executivo

A sessao Ghidra headless com 2886 simbolos PAL reconciliados (Rev.096 + exploracao posterior) revelou que a arquitetura do jogo havia sido interpretada incorretamente desde Rev.062.

**O erro central:** a funcao `_Clip` em `0x166E10` foi tratada como "main_dispatcher" geral de entidades. Ela nao e. E uma funcao de colisao/clipping dentro do sistema `DispCollisionPC`.

**O sistema real de objetos do jogo** e a familia `isysGObj*` (`0x13DDA0`-`0x141D18`, 30 funcoes), que gerencia registro de callbacks, alocacao, remocao, e travessia de game objects.

**Correcoes derivadas:**
- A tabela de 17 slots em `0x282690` e a tabela de configuracao de clipping, nao um dispatcher geral.
- Apenas 4 dos 17 slots tem callbacks reais (`_clipW*`), nao 15 como os dados runtime sugeriam.
- Os 42.2M eventos runtime (15 slots ativos) pertencem ao sistema `isysGObj*`, nao ao `_Clip`.
- `execBombGeo`/`ItemGeo` sao funcoes de geometria fisica (cloth), nao estado de entidade.
- GirlBrain e a maquina de estado da IA da garota (30+ funcoes nomeadas).
- O scene loader reside em `kanban.c` (GP=`0x27A7A8`).

---

## Correcao principal

### O que era (modelo supersedido, Rev.062-076)

```txt
_Clip (0x166E10) = "main_dispatcher" geral de entidades
Tabela 0x282690 = "dispatch table" de 17 slots
callback_register (0x13F7A8) + mask_set (0x13ED40) = sistema de dispatch
execBombGeo (0x1D37C8) = "cloth_dispatcher" com 5 estados internos
ItemGeo (0x1D3A30) = "cb_routine2" misterioso
Slots runtime (42.2M eventos) = distribuicao do _Clip
```

### O que e (modelo corrigido, Rev.097)

```txt
isysGObj* (0x13DDA0-0x141D18, 30 funcoes) = sistema real de game objects
isysGObjProcAdd_ (0x13F3F0) = registro de callbacks (era "callback_register")
iosOmExeEachGObj (0x13FD10) = travessia/dispatcher de objetos
_Clip (0x166E10) = funcao de clipping/colisao dentro de DispCollisionPC
Tabela 0x282690 = config de 17 slots de clipping (4 callbacks _clipW* ativos)
execBombGeo (0x1D37C8) = geometria de explosao/bomba (cloth, 5 estados)
ItemGeo (0x1D3A30) = geometria de item (physics constraint solver)
GirlBrain (0x191B70-0x19C040) = IA da garota, 30+ funcoes nomeadas
kanban.c (GP=0x27A7A8) = scene loader (8+ funcoes)
42.2M eventos runtime = sistema isysGObj*, NAO _Clip
```

### O que permanece valido do modelo anterior

- `code_0x166BB0` = halfword writer ativo no hot path (Rev.093b). Continua sendo parte do pipeline de colisao, nao do objeto generico.
- `code_0x166DFC` = single-cell fast path inferido. Continua em clipping.
- Descritor em `0x2A31B8`: 68 tipos de entidade. A estrutura do descritor permanece.
- `execBombGeo` jump table em `0x618FB0` com 5 alvos. Os alvos sao estados de geometria.
- `FUN_0013f7a8` em `0x13F7A8` = funcao interna do sistema isysGObj, nao o unico registro.
- `FUN_0013ed40` em `0x13ED40` = ShockRequestBox_RequestCancel. Nao muda.

---

## isysGObj como sistema real de objetos

### Escopo

30 funcoes nomeadas entre `0x13DDA0` e `0x141D18`, mais tres em `0x1FC168`-`0x1FC2E0` (camera).

### Funcoes principais

| VA | Nome real | Papel |
|----|-----------|-------|
| `0x0013DDA0` | `isysGObjInit` | Inicializacao do sistema |
| `0x0013E4D0` | `isysGObjAlloc` | Alocacao de GObj |
| `0x0013E548` | `isysGObjRemove` | Remocao de GObj |
| `0x0013E8D8` | `isysGObjAdd` | Adicao de GObj a lista |
| `0x0013F130` | `isysGObjLinkObjDL` | Vinculacao a display list |
| `0x0013F3F0` | `isysGObjProcAdd_` | **Registro de callback** (488B, =antigo "callback_register") |
| `0x0013F6B8` | `isysGObjProcRemove` | Remocao de callback |
| `0x0013F8C0` | `isysGObjProcRemoveAll` | Remocao em massa |
| `0x0013F9D0` | `_iosOmMain` | Loop principal de processamento |
| `0x0013FC00` | `iosOmCreateDL` | Criacao de slot/display list |
| `0x0013FD10` | `iosOmExeEachGObj` | **Travessia de objetos** (=antigo dispatcher) |
| `0x0013FD78` | `iosOmExeEachGObjAll` | Travessia de todos os objetos |
| `0x0013FE18` | `iosOmReturnExeEachGObj` | Travessia com retorno |

### Implicacoes

- O antigo "callback_register" (`0x13F7A8`) nao e o unico registro — e `isysGObjProcAdd_` em `0x13F3F0`.
- O antigo "mask_set" (`0x13ED40`) e uma funcao auxiliar de I/O (ShockRequestBox_RequestCancel).
- `FUN_0013f7a8` deve ser renomeada para `isysGObjProcAdd` ou similar nas proximas revisoes.
- A travessia de objetos (`iosOmExeEachGObj`) explica a distribuicao de slots runtime.

---

## _Clip / DispCollisionPC nao e dispatcher geral

### Evidencia

1. **Xrefs estaticos**: `_Clip` (`0x166E10`) tem apenas 2 callers — ambos em `DispCollisionPC` (`0x166A10`).
2. **Conteudo da funcao**: 34 instrucoes. Nao ha loop de travessia de entidades. A funcao carrega configuracao de um slot, aplica clipping a uma entidade, e retorna.
3. **Tabela de 17 slots** (`0x282690`): 16-byte entries `[flag 4B] [mode 4B] [tier 4B] [callback_ptr 4B]`. So 4 slots tem callbacks reais (`_clipW*` nos slots 0, 4, 8, 12). Slots 1-3, 5-7, 9-11, 13-15 tem flag=0, callback=0.
4. **Dados runtime nao batem**: 15 slots ativos medidas no PCSX2 (13 com >0 eventos) vs 4 callbacks ativos na tabela.

### Tabela de 17 slots — estado real

| Slot | Flag | Callback | Ativo? |
|------|------|----------|--------|
| 0 | 0x00000001 | `_clipWDebug` | SIM |
| 1 | 0x00000000 | (null) | nao |
| 2 | 0x00000000 | (null) | nao |
| 3 | 0x00000000 | (null) | nao |
| 4 | 0x00000001 | `_clipW` | SIM |
| 5 | 0x00000000 | (null) | nao |
| 6 | 0x00000000 | (null) | nao |
| 7 | 0x00000000 | (null) | nao |
| 8 | 0x00000001 | `_clipWR` | SIM |
| 9 | 0x00000000 | (null) | nao |
| 10 | 0x00000000 | (null) | nao |
| 11 | 0x00000000 | (null) | nao |
| 12 | 0x00000001 | `_clipWField` | SIM |
| 13 | 0x00000000 | (null) | nao |
| 14 | 0x00000000 | (null) | nao |
| 15 | 0x00000000 | (null) | nao |
| 16 | 0x00000001 | (null) | flag, sem callback |

### Callbacks _clipW*

| Slot | Nome real | Endereco |
|------|-----------|----------|
| 0 | `_clipWDebug` | `0x00168DA8` |
| 4 | `_clipW` | `0x00168ED0` |
| 8 | `_clipWR` | `0x001692F0` |
| 12 | `_clipWField` | `0x00169440` |

### Halfword writer (0x166BB0)

Continua sendo parte do pipeline de colisao — reside em `DispCollisionPC` ou funcao adjacente (`0x166A10-0x166E10`). Os probes de Rev.093b/094 confirmaram que esta ativo no hot path. O fast path em `0x166DFC` ainda nao foi diretamente verificado em runtime. A correcao arquitetural nao muda o que ja foi validado sobre o halfword writer — apenas esclarece que ele opera no contexto de clipping, nao de dispatch geral de entidades.

---

## Relacao com os dados runtime

### Problema central

As sessoes PCSX2 entre Rev.074 e Rev.084 capturaram uma **distribuicao de 15 slots ativos** com 42.2M eventos. Esses dados foram interpretados como pertencendo ao "main_dispatcher" (`_Clip`).

### Evidencia da incompatibilidade

A tabela de 17 slots do `_Clip` tem somente 4 callbacks ativos. Uma tabela com 4 callbacks nao pode produzir uma distribuicao de 15 slots com eventos. Os valores de slot vistos em runtime (0-15 com contagens variaveis) nao correspondem aos slots do `_Clip`.

### Explicacao provavel

O sistema `isysGObj*` gerencia seus proprios slots/grupos de objetos. `iosOmCreateDL` (`0x13FC00`) cria slots, e `iosOmExeEachGObj` (`0x13FD10`) itera objetos por slot. E provavel que os 15 slots runtime pertencam a este sistema.

### O que fazer

- Nao reinterpretar os dados runtime antigos como isysGObj* sem nova captura.
- A proxima sessao runtime deve marcar eventos de `iosOmExeEachGObj` para confirmar.
- Manter os dados de Rev.074-084 como validos, mas sem vinculo ao _Clip.

---

## GirlBrain surface map

### Descoberta

30+ funcoes nomeadas com prefixo `eBrain*`, `GirlBrain*`, `girlBrain*`, `subGirlBrain*`, `_girlBrain*` entre `0x00191B70` e `0x0019C040`. Isto e uma maquina de estado de IA dedicada a personagem da garota.

### Funcoes identificadas

| VA | Nome | Papel provavel |
|----|------|----------------|
| `0x0016AC10` | `GirlBrainClearTarget` | Limpeza de alvo |
| `0x0016BCA0` | `girlBrainMain_PositionUpdate` | Atualizacao de posicao |
| `0x0016CED0` | `subGirlBrain_PulledUp` | Reacao a ser puxada |
| `0x0016E910` | `_girlBrainHide_MakeHidePoint` | Criar ponto de esconderijo |
| `0x0016EB68` | `girlBrainHide_GoalTurn` | Rotacao para esconderijo |
| `0x0016F410` | `girlBrainRunawaySearchPoint` | Busca de ponto de fuga |
| `0x00191B70` | `eBrainSystemInit` | Inicializacao do sistema eBrain |
| `0x00191D20` | `eBrainGetStatus` | Status atual |
| `0x00191EE8` | `eBrainSetFlag` | Setar flag |
| `0x00191FB8` | `eBrainMovePos` | Movimento para posicao |
| `0x001921E8` | `eBrainMotionSe` | Efeito sonoro de movimento |
| `0x001923B0` | `eBrainPursuit` | Perseguicao |
| `0x001928A8` | `eBrainAvoid` | Desvio |
| `0x00192D78` | `eBrainReturnInit` | Retorno ao estado inicial |
| `0x00192E60` | `eBrainTargetGenerator*` (8 variantes) | Geracao de alvo |
| `0x00195C88` | `eBrainEntryStart` | Entrada inicial |
| `0x00195E10` | `eBrainEntryFront` | Entrada frontal |
| `0x00196040` | `eBrainEntry*` (4+ entradas adicionais) | Entradas de estado |

### O que NAO sabemos

- A estrutura exata da maquina de estados (tabela de transicoes, condicoes).
- Se ha uma funcao dispatcher unica ou multiplos pontos de entrada.
- A relacao com o sistema isysGObj* (GirlBrain provavelmente e um process registrado).
- Se TODOS os 30+ estados sao alcancaveis (alguns podem ser dead code ou debugging).
- Nomes internos dos estados — os nomes das funcoes indicam semântica, mas nao substituem analise estrutural.

### O que e provavel

- GirlBrain e registrada como um process no sistema isysGObj* (via `isysGObjProcAdd_`).
- A garota tem estados de: seguir (pursuit), fugir (avoid/runaway), esconder (hide), ser puxada (pulled up), entrar em cena (entry).
- `eBrainTargetGenerator*` em `0x00192E60`-`0x00195C88` sugere 8 variantes de geracao de alvo (patrulha, fuga, esconderijo, etc.).
- Nao ha indicacao de que GirlBrain use o mesmo dispatcher de _Clip — sao sistemas ortogonais.

### Proximo passo (nao agora)

Selecionar 1-2 funcoes pequenas de GirlBrain (ex: `GirlBrainClearTarget` ou `eBrainGetStatus`) para confirmar assinatura e estrutura em C ou .s. Mas nao agora — a tarefa de consolidacao nao abre novas frentes.

---

## Main loop e kanban.c

### Cadeia principal confirmada

```
vblankHandler (0x1BDE48)
  -> ACTGame (0x1A63E0)
       -> backStageProcessMain (0x1A05D0)
       -> stage_ApplyData (0x1A2A1D8)
       -> kanbanExec (0x1B05A8)  [scene loader]
       -> initSceneGObj (0x1B76F8)
       -> HotInitSceneObjects (0x1B7F20)
       -> la_load_processing (0x1B2A30)
       -> la_switching_stage (0x1B5958)
```

### kanban.c — modulo de stage/scene loading

GP=`0x27A7A8` indica compilacao separada (outro modulo). Funcoes confirmadas:

| VA | Funcao | Papel |
|----|--------|-------|
| `0x001AFB98` | `kanbanReqAdd` | Adiciona requisicao de carregamento |
| `0x001AFE50` | `kanbanInit` | Inicializa sistema kanban |
| `0x001B0538` | `kanbanReqAllDel` | Remove todas as requisicoes |
| `0x001B05A8` | `kanbanExec` | Executa fila de carregamento |
| `0x001B2A30` | `la_load_processing` | Processa carregamento |
| `0x001B5958` | `la_switching_stage` | Transicao de stage |
| `0x001B76F8` | `initSceneGObj` | Inicializa GObj da cena |
| `0x001B7F20` | `HotInitSceneObjects` | Inicializacao "quente" de objetos |
| `0x001B81A8` | `MoveNextStage_Clear` | Limpeza na transicao |

### Implicacao

O scene loader e um sistema separado, compilado com GP proprio. Nao faz parte do loop isysGObj*, mas interage com ele via `initSceneGObj`.

---

## Mapa de modulos provaveis

Com base nos simbolos Ghidra, strings de depuracao, e agrupamento de GP, os modulos de codigo fonte do jogo podem ser mapeados assim:

### Modulos confirmados

| Modulo | Path provavel | GP | Funcoes identificadas | Evidencia |
|--------|---------------|----|----------------------|-----------|
| Core engine | `src/fumi/` | — | vblank, IO, CDVD, heap | Strings de path em `0x00554XX` |
| Item/Physics | `src/sugipon/item.c` | — | BARREL hC (0x1D27A8), ROPE hC | Assertion string "src/item.c" linha 434 |
| Field collision | `src/sugipon/fieldCollision.c` | — | barrel_init (0x166028) | Assertion string linha 533 |
| Scene loading | `src/sugipon/kanban.c` | 0x27A7A8 | kanbanExec, initSceneGObj, la_* | GP proprio + nomes kanban |
| Cloth physics | `src/sugipon/clothAnimation.c` | — | execBombGeo, ItemGeo, GetCloth4D | Cross-ref ICO-decomp (range 0x1D37C8-0x1D3B28) |
| BOY handler | `src/omori/boy.c` | — | boy_hA, boy_hB, boy_hC, boy_init | Nome do arquivo na string |
| ENEMY1 handler | `src/omori/enemy1.c` | — | enemy1_hA, enemy1_hB, enemy1_hC | Nome do arquivo na string |
| GirlBrain AI | `src/omori/` (provavel) | — | eBrain*, GirlBrain*, girlBrain* | Co-localizacao com BOY (omori) |
| Camera | `src/omori/` | — | isysGObjMoveCameraDL, etc. | Co-localizacao com omori |
| Stage manager | `src/sugipon/stage.c` | — | stage_ApplyData, stage_SetAnimation | Nome stage_* ~50 funcoes |

### Modulos provaveis

| Modulo | Path provavel | Evidencia |
|--------|---------------|-----------|
| Audio | `src/fumi/snd*.c` | `sndBgmReadyNextStage` em 0x144780 |
| Debug UI | `src/sugipon/debug*.c` | debug_SelectStage, debug_Load |
| Animation | `src/sugipon/StageAnimation.c` | String "src/StageAnimation.c" em 0x555BF8 |
| Texture loading | `src/seki/` (ou sugipon) | tex_loadImage, tex_LoadTexture |
| Rendering pipeline | `src/seki/` | isysGObjLinkObjDL, isysGObjMoveCameraDL |
| Queue/deferred | `src/seki/` ou `src/fumi/` | VU0 queue em 0x117768 |
| Boss Queen | `src/ito/` | queenBrain* (nao explorado) |
| System/BIOS wrappers | `src/fumi/` | isys*, ios* — funcoes de kernel-like |

### Nota

Estes paths sao **reconstrucoes** baseadas em nomes de simbolos e strings de depuracao. O diretorio real no codigo fonte original pode diferir. O importante e o **agrupamento logico**: funcoes que compartilham GP ou prefixo de nome provavelmente vieram do mesmo arquivo .c.

---

## Renomeacoes ja aplicadas

### Arquivos .s renomeados

| Nome antigo | Nome novo | VA |
|-------------|-----------|----|
| `cb_routine2.s` | `ItemGeo.s` | `0x001D3A30` |
| `cloth_dispatcher.s` | `execBombGeo.s` | `0x001D37C8` |
| `fn_1D3DD8.s` | `ReviveAllCarryableItems.s` | `0x001D3DD8` |
| `fn_1D2550.s` | `HoldItem.s` | `0x001D2550` |
| `sub_1D2650.s` | `avoidInsideOfWall.s` | `0x001D2650` |
| `sub_1C1C48.s` | `synchronizeMotionOutputOriginForGirl.s` | `0x001C1C48` |
| `sub_1C1EA8.s` | `boy_dispCrown.s` | `0x001C1EA8` |

### Codigo C atualizado

- `src/entity/barrel.c`: `fn_1D2550` -> `HoldItem`, `sub_1D2650` -> `avoidInsideOfWall`, `cb_routine2` -> `ItemGeo`, `fn_1D3DD8` -> `ReviveAllCarryableItems`
- `src/entity/boy.c`: `sub_1C1C48` -> `synchronizeMotionOutputOriginForGirl`, `sub_1C1EA8` -> `boy_dispCrown`, `cb_routine2` -> `ItemGeo`
- `src/cloth/near_matches.c`: `cloth_dispatcher` -> `execBombGeo`

### Nao renomeado (ainda)

- `sub_1D2738.s` — sem simbolo PAL. Nome atual e funcional.
- `fn_1D3BF0.s` — sem simbolo PAL. Nome atual e funcional.
- `FUN_0013f7a8` — provavelmente funcao interna de isysGObj. Renomear na proxima revisao.
- `FUN_0013ed40` — ShockRequestBox_RequestCancel. Nome atual descritivo.

---

## O que fica confirmado

- `isysGObj*` (30 funcoes, 0x13DDA0-0x141D18) e o sistema de objetos do jogo.
- `_Clip` (0x166E10) e funcao de clipping, nao o dispatcher geral.
- A tabela 0x282690 e de clipping, com 4 callbacks ativos (slot 0/4/8/12).
- `execBombGeo` e ItemGeo sao funcoes de geometria (cloth/item physics).
- GirlBrain existe e tem 30+ funcoes nomeadas de IA.
- Main loop: vblankHandler -> ACTGame -> backStageProcessMain/stage_ApplyData/kanbanExec.
- Scene loader esta em kanban.c com GP proprio (0x27A7A8).
- .s files renomeados para simbolos reais — OK.
- C sources atualizados — OK.
- Pesquisa de exploracao Ghidra completa (5 scripts) — OK.

---

## O que fica provavel

- Os 42.2M eventos runtime pertencem ao sistema isysGObj*, nao ao _Clip. (Nao confirmado — requer nova captura runtime.)
- `FUN_0013f7a8` e uma funcao interna de isysGObj* (`isysGObjProcAdd` ou similar). (Nao confirmado — requer analise de xrefs.)
- A tabela 0x281A70 (referenciada em `_iosOmMain`) e a tabela de dispatch de processes do isysGObj*. (Nao confirmado — requer verificacao estatica.)
- GirlBrain e registrada como process via `isysGObjProcAdd_`. (Nao confirmado — requer xrefs ou runtime.)
- `kanban.c` pertence a `src/sugipon/` (nao confirmado — GP diferente sugere modulos separados).
- Os 8+ estagios do scene loader sao executados em ordens especificas durante `kanbanExec`. (Nao confirmado — requer analise de fluxo.)
- `0x13FC00` (`iosOmCreateDL`) cria os slots que aparecem nos dados runtime. (Provavel — mas requer verificacao.)

---

## O que foi descartado

- `_Clip` como "main_dispatcher" geral. Revogado. O modelo Rev.062-076 esta incorreto neste ponto.
- Tabela 0x282690 como "dispatch table" de entidades. E tabela de configuracao de clipping.
- `execBombGeo` como dispatcher de estado de entidade. E funcao de geometria cloth.
- `ItemGeo` como "cb_routine2" misterioso com registro desconhecido. E ItemGeo, registrado via descritor +0x50.
- Distribuicao de 15 slots runtime como pertencente ao _Clip. Os dados existem mas pertencem a outro sistema.
- `FUN_0013f7a8` como o unico mecanismo de registro de callback. isysGObjProcAdd_ e o primario.
- Slots 1-3, 5-7, 9-11, 13-15 como tendo callbacks ativos no _Clip. Todos tem flag=0.
- `_clipWField` como funcao de "field clipping" generico. E especifica do _Clip.

---

## Proximos alvos pequenos

Prioridade baixa — estes sao apenas registrados como candidatos naturais para apos a consolidacao:

1. **Xrefs de `FUN_0013f7a8`**: verificar se e chamada por `isysGObjProcAdd_` ou vice-versa.
2. **Estrutura de `iosOmExeEachGObj`**: confirmar se itera uma lista de processes que corresponde aos slots runtime.
3. **`_iosOmMain` em 0x13F9D0**: analisar o dispatcher principal do isysGObj*.
4. **Tabela 0x281A70**: dump e decode como tabela de dispatch de processes.
5. **`kanbanExec`**: analisar estagios do scene loader.
6. **`GirlBrainClearTarget`**: funcao pequena candidata a C ou .s.
7. **`eBrainGetStatus`**: funcao pequena candidata a C ou .s.
8. **Corrigir nomenclatura**: `FUN_0013f7a8` -> `isysGObjProcAdd`, `FUN_0013ed40` -> permanece como `ShockRequestBox_RequestCancel`.
9. **Remover `FUN_0013ed40` do grupo "mask_set"** em docs — nao e mascara de dispatch.

---

## Riscos

### Risco 1: Supercorrecao

Ha o risco de tratar `_Clip` como irrelevante. Ele nao e — e parte central do pipeline de colisao, e o halfword writer que investigamos por 3 revisoes reside nele. A correcao e de interpretacao, nao de descarte.

### Risco 2: Subestimar isysGObj*

Com 30 funcoes, isysGObj* e um sistema grande. Pode conter complexidade inesperada (threading, filas, sync com IOP). Nao assumir que e simples.

### Risco 3: GirlBrain excesso de confianca

Os nomes das funcoes GirlBrain sao sugestivos mas nao substituem analise estrutural. "Pursuit", "Avoid", "Runaway" sao nomes de funcao — nao sabemos se correspondem a estados reais, funcoes auxiliares, ou debug stubs.

### Risco 4: Dados runtime orfaos

Os 42.2M eventos de Rev.074-084 ficam sem "dono" confirmado. Se usarmos esses dados para hipoteses sobre isysGObj* sem verificar, corremos risco de repetir o erro do _Clip. Melhor manter os dados como "nao atribuidos" ate nova captura.

### Risco 5: GP duplicado

Se kanban.c (GP=0x27A7A8) e um modulo separado, pode haver conflito de GP com outros modulos. Nao assumir que GP+0x6F60 (world_state) e o mesmo para todos os modulos.

---

## Veredito

**A correcao arquitetural desta revisao e o avanco mais significativo desde a descoberta do descritor de 68 entidades (Rev.074).**

O modelo antigo (Rev.062-076) colocava `_Clip` no centro do universo do jogo — um dispatcher de 17 slots que processava clipping, halfword tables, e callbacks. Esse modelo era internamente consistente, mas estava errado.

O modelo corrigido e mais modular:

```
isysGObj* (0x13DDA0)  -- sistema de objetos (registro, alocacao, travessia)
  |
  +-- processes registrados (incluindo handlers de entidade, IA, etc.)
  |
  +-- travessia via iosOmExeEachGObj (explica dados runtime)
  |
_Clip/DispCollisionPC (0x166A10)  -- pipeline de colisao separado
  |
  +-- 17 slots de configuracao (compile-time .data)
  +-- 4 callbacks _clipW* ativos
  +-- halfword writer inline
  |
kanban.c (0x1AFB98, GP=0x27A7A8)  -- scene loader separado
  |
  +-- 9+ funcoes de carregamento de stage
  +-- initSceneGObj / HotInitSceneObjects
  |
GirlBrain (0x191B70)  -- IA da garota (sistema separado)
  |
  +-- 30+ funcoes nomeadas
  +-- pursuit, avoid, hide, runaway states
```

Este layout faz mais sentido para um jogo de 2001: um kernel de objetos generico (isysGObj*), um sistema de colisao dedicado (_Clip/DispCollisionPC), um loader de cenas (kanban.c), e IA especifica por personagem (GirlBrain para garota, e provavelmente sistemas similares para inimigos e boss).

**Toda a nomenclatura do projeto deve ser atualizada para refletir este modelo.** Pesquisas futuras devem referir-se a `isysGObj*` como sistema de objetos, `_Clip` como clipping, e assim por diante. Arquivos .s ja foram renomeados. O passo seguinte (apos consolidacao) e renomear `FUN_0013f7a8` e atualizar a nomenclatura nos documentos de alto nivel.

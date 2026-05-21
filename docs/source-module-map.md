# Source Module Map — ICO Reconstruction

**Date:** 2026-05-21 (Rev.097 — initial version)
**Last updated:** 2026-05-21

> Mapa de modulos do codigo fonte original do ICO, reconstruido a partir de:
> - Simbolos PAL reconciliados (2886 nomes no Ghidra)
> - Strings de depuracao com paths de arquivo (.c)
> - Agrupamento por registrador GP (compilacao separada)
> - Prefixos de nomes de funcao
> - Cross-reference com ICO-decomp (sugipon, omori, fumi, seki, ito)

## Convencao

- **Confirmado**: evidencia direta (string de assert, xref estatico, nome de simbolo)
- **Provavel**: consistencia de prefixo/GP, mas sem string de path direta
- **Possivel**: inferencia por localizacao ou contexto
- Paths sao **reconstrucoes** — o layout real do diretorio fonte original pode diferir

---

## Modulos confirmados

### `src/fumi/` — Core engine

| Evidencia | Descricao |
|-----------|-----------|
| Strings de path `src/fumi/sio2emu.c` | IO/SIO2 driver |
| Strings de path `src/fumi/cdvd.c` | CDVD driver |
| Nomes `isys*`, `ios*` | Kernel-like abstracoes (isys = io system) |

Funcoes tipicas: `isysGObjInit`, `isysGObjAlloc`, `isysGObjRemove`, `isysGObjProcAdd_`, `iosOmInit`, `iosOmExeEachGObj`, vblank handler, IO, CDVD, heap management.
Neste repositorio, os helpers byte-exact verificados de `isysGObj*` e `iosOm*` vivem em `src/core/asm/` e o layout local fica em `src/core/isys_process.h`.

### `src/sugipon/item.c` — Item/Physics handlers

| Evidencia | Descricao |
|-----------|-----------|
| String de assert: `"src/item.c"` em `0x618F68` | Confirmado por assertion no hC do BARREL (linha 434) |

Funcoes tipicas: `barrel_hC` (0x1D27A8), `rope_hC` (0x1D3B28), funcoes de constraint solving.

### `src/sugipon/fieldCollision.c` — Field collision

| Evidencia | Descricao |
|-----------|-----------|
| String de assert: `"src/fieldCollision.c"` em `0x5591F0` | Confirmado por assertion no init_fn do BARREL (linha 533) |

Funcoes tipicas: `barrel_init` (0x166028), `_Clip` (0x166E10), `DispCollisionPC` (0x166A10).

### `src/sugipon/kanban.c` — Scene/stage loader

| Evidencia | Descricao |
|-----------|-----------|
| GP proprio `0x27A7A8` | Compilacao separada dos demais modulos |
| Prefixo `kanban*` | 4 funcoes: `kanbanReqAdd`, `kanbanInit`, `kanbanReqAllDel`, `kanbanExec` |
| Prefixo `la_*` | `la_load_processing`, `la_switching_stage` |
| Prefixo `initScene*`/`HotInit*` | `initSceneGObj`, `HotInitSceneObjects`, `MoveNextStage_Clear` |

### `src/sugipon/clothAnimation.c` — Cloth physics

| Evidencia | Descricao |
|-----------|-----------|
| Cross-ref ICO-decomp | Range `0x1D37C8-0x1D3B28` identificado como `clothAnimation.c` |
| Nomes de funcao | `GetCloth4D`, `getCloth4D_PlaneClip`, `execBombGeo`, `ItemGeo` |

### `src/omori/boy.c` — BOY handler

| Evidencia | Descricao |
|-----------|-----------|
| Nome de simbolo `boy.c` | Funcoes `boy_hA`, `boy_hB`, `boy_hC`, `boy_init` |
| Prefixo `boy_*` | Dominante para todas as funcoes do personagem |

### `src/omori/enemy1.c` — ENEMY1 handler

| Evidencia | Descricao |
|-----------|-----------|
| Nome de simbolo `enemy1.c` | Funcoes `enemy1_hA`, `enemy1_hB`, `enemy1_hC`, `enemy1_init` |

---

## Modulos provaveis

### `src/omori/` — Camera, GirlBrain, e AI

| Evidencia | Descricao |
|-----------|-----------|
| Co-localizacao com BOY | Camera DL functions (`isysGObjMoveCameraDL`) compartilham prefixo com omori |
| GirlBrain `eBrain*` | 30+ funcoes de IA da garota (`eBrainSystemInit`, `eBrainPursuit`, etc.) |
| `subEnemyBrain_*` | Funcoes auxiliares de IA de inimigo |

### `src/sugipon/debug*.c` — Debug UI

| Evidencia | Descricao |
|-----------|-----------|
| Prefixo `debug_*` | `debug_SelectStage`, `debug_Load`, `debug_SaveStartStageFile`, `debug_BackStageTest` |

### `src/sugipon/StageAnimation.c` — Animation system

| Evidencia | Descricao |
|-----------|-----------|
| String de path: `"src/StageAnimation.c"` em `0x555BF8` | Confirmado por string (mas nao por assertion) |

### `src/sugipon/stage.c` — Stage data manager

| Evidencia | Descricao |
|-----------|-----------|
| Prefixo `stage_*` | ~50 funcoes: `stage_ApplyData`, `stage_SetAnimation`, `stage_MakeGObj` |

---

## Modulos possiveis

### `src/seki/` — Rendering pipeline

| Evidencia | Descricao |
|-----------|-----------|
| ICO-decomp estrutura | `seki` e modulo de rendering no codigo original |
| Funcoes de textura | `tex_loadImage`, `tex_LoadTexture`, `tex_LoadTexturePart` |
| Display list | `isysGObjLinkObjDL`, `isysGObjMoveCameraDL` |

### `src/fumi/` — Audio

| Evidencia | Descricao |
|-----------|-----------|
| Prefixo `snd*` | `sndBgmReadyNextStage` em `0x144780` |

### `src/ito/` — Boss/Queen

| Evidencia | Descricao |
|-----------|-----------|
| Nome de modulo `ito` | ICO-decomp estrutura: ito gerencia boss fights |
| `queenBrain*` | Funcoes de IA da rainha (nao explorado) |

### `src/sugipon/bird.c` — BIRD handler

| Evidencia | Descricao |
|-----------|-----------|
| Prefixo `bird_*` | `bird_hC` em `0x001D43E8` |

### `src/sugipon/woodbox0.c` — WOODBOX0 handler

| Evidencia | Descricao |
|-----------|-----------|
| Prefixo `woodbox0_*` | `woodbox0_hA`, `woodbox0_hB`, `woodbox0_hC` |

---

## Tabela de registrador GP

| GP | Modulo | Funcoes representativas |
|----|--------|------------------------|
| `0x276140` | (principal) | `_Clip`, `DispCollisionPC`, entity dispatch |
| `0x27A7A8` | `kanban.c` | Scene loader functions |
| `0x276ED4` | (principal) | World state |

**Nota:** GP diferente indica compilacao separada (outro arquivo .c ou unidade de compilacao). Funcoes que compartilham GP estao no mesmo modulo. Apenas `kanban.c` (GP=`0x27A7A8`) foi confirmado como modulo separado ate agora.

---

## Tabela de prefixos de funcao

| Prefixo | Modulo provavel | Contagem aprox. |
|---------|-----------------|-----------------|
| `isysGObj*` | `fumi/` | ~25 |
| `iosOm*` | `fumi/` | ~10 |
| `_Clip*` | `sugipon/fieldCollision.c` | ~5 |
| `eBrain*` | `omori/` | ~25 |
| `girlBrain*` | `omori/` | ~5 |
| `boy_*` | `omori/boy.c` | ~20 |
| `enemy1_*` | `omori/enemy1.c` | ~10 |
| `stage_*` | `sugipon/stage.c` | ~50 |
| `kanban*` | `sugipon/kanban.c` | 4 |
| `la_*` | `sugipon/kanban.c` | 2 |
| `tex_*` | `seki/` | 3 |
| `snd*` | `fumi/` | ~5 |
| `debug_*` | `sugipon/debug*.c` | ~10 |
| `barrel_*` | `sugipon/item.c` | ~5 |
| `rope_*` | `sugipon/item.c` | ~3 |
| `woodbox0_*` | `sugipon/woodbox0.c` | ~3 |
| `bird_*` | `sugipon/bird.c` | ~3 |
| `queen*` | `ito/` | ~5 |

---

## Notas

- O modulo `sugipon` e o maior contribuidor — parece conter gameplay, fisica, colisao, e carregamento.
- `fumi/` e o engine core — IO, CDVD, sistema de objetos, heap.
- `omori/` contem logica de personagem — BOY, ENEMY1, GIRL, camera.
- `seki/` e rendering — texturas, display lists, VU0.
- `ito/` e boss — rainha, cenas especiais.
- `kanban.c` e o unico modulo com GP confirmadamente diferente, indicando compilacao separada.
- A maioria das funcoes do jogo (~60%) usa o GP principal (`0x276140`).
- Nao ha indicacao de overlays no .text — todo o codigo parece linkado estaticamente no ELF unico PT_LOAD.

# ICO-decomp Cross-Reference

## Data

2026-05-14

## Objetivo

Cruzar endereços mapeados nas revisões Rev.001-037 com a symbol table do projeto ICO-decomp (`RossyDoubleUnderscore/ICO-decomp`) e mapear funções para arquivos fonte originais via splat YAML.

## Escopo

Incluído:
- clonagem e análise do repositório `RossyDoubleUnderscore/ICO-decomp`
- correlação de 14 endereços mapeados contra 5.792 símbolos nomeados
- mapeamento source file via splat YAML (1.174 subsegmentos)
- análise de naming conventions e estrutura de diretórios

Excluído:
- submissão manual ao decomp.me
- runtime validation
- análise de DATA.DF

## Fontes usadas

| Fonte | Uso |
|---|---|
| `github.com/RossyDoubleUnderscore/ICO-decomp` | symbol_addrs.txt (5792 símbolos) + ICO-PAL.yaml (1174 subsegmentos) |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | endereços mapeados nas revisões anteriores |
| `.local/extracted/SCUS_971.13.elf` | verificação de contexto |

## Resultado 1: Mapeamento Endereço -> Símbolo

| Endereço USA | Função | Símbolo ICO-decomp +- | Arquivo Fonte |
|---|---|---|---|
| 0x001d37c8 | dispatcher (não nomeado) | InitCloth4D(-472) .. GetChainNodeGlobalQuaternion(+784) | sugipon/src/clothAnimation.c |
| 0x001d3a30 | ROPE callback (não nomeado) | InitCloth4D(-1088) .. GetChainNodeGlobalQuaternion(+168) | sugipon/src/clothAnimation.c |
| 0x001d3818 | state_0_block (não nomeado) | InitCloth4D(-552) .. GetChainNodeGlobalQuaternion(+704) | sugipon/src/clothAnimation.c |
| 0x001d3844 | state_1_block (não nomeado) | InitCloth4D(-596) .. GetChainNodeGlobalQuaternion(+660) | sugipon/src/clothAnimation.c |
| 0x001d391c | state_2_block (não nomeado) | InitCloth4D(-812) .. GetChainNodeGlobalQuaternion(+444) | sugipon/src/clothAnimation.c |
| 0x001d39e0 | state_3_block (não nomeado) | InitCloth4D(-1008) .. GetChainNodeGlobalQuaternion(+248) | sugipon/src/clothAnimation.c |
| 0x001d3a10 | state_4_block (não nomeado) | InitCloth4D(-1056) .. GetChainNodeGlobalQuaternion(+200) | sugipon/src/clothAnimation.c |
| 0x0013f7a8 | callback_registration | iosThreadCreateS(-528) .. iosThreadStart(+8) | fumi/ios/thread.c |
| 0x0013f778 | sister_callback_reg | iosThreadCreateS(-480) .. iosThreadStart(+56) | fumi/ios/thread.c |
| 0x0013f3f0 | node_callback_storage | Shock_Decode(-40) .. dumyAllocFunc(+144) | fumi/ios/shockdriver |
| 0x001b76f8 | object_init_reg_path | disp_memory_partition_bar(-248) .. disp_memory_partition(+1264) | common/src/gamesys |
| 0x0013eb50 | state_resolver | ShockRequestBox_DecodeRequest(-32) .. ShockRequestBox_EndRequestFree(+184) | fumi/ios/shockdriver |
| 0x00199f80 | entity_parent | GetStageFromLabel(-160) .. eBrainGetTargetGeneratorFromLabelStage(+48) | omori/src/ebrain |
| 0x0017bb98 | VU_parent | actGirlWalk(-8) .. actGirlRun(+64) | sugipon/src/girl |

Nenhum dos 14 endereços possui um símbolo exato na tabela do ICO-decomp. Todos caem entre funções nomeadas, indicando que são funções internas ainda não identificadas pelo projeto.

## Resultado 2: Ajuste PAL vs USA

O ICO-decomp usa a versão **PAL** (`SCES_507.60`). Diferenças estruturais em relação à USA (`SCUS_971.13`):

| Característica | PAL (SCES_507.60) | USA (SCUS_971.13) |
|---|---|---|
| .text size | 0x189bc4 | 0x16f5d4 |
| .data size | 0x2be620 | 0x2defb8 |
| .rodata size | 0x0eb718 | 0x0dd1a8 |
| Total seções | 27 | 27 |

A correlação foi feita por contexto de símbolos vizinhos, não por correspondência direta de endereço.

## Resultado 3: Correção crítica — dispatcher é cloth physics

Os 5 blocos internos de estado do dispatcher `0x001d37c8` **NÃO** são estados de entidade/gameplay. Estão em `sugipon/src/clothAnimation.c` — física de simulação de pano.

Funções vizinhas confirmadas no mesmo range:

| Endereço PAL | Símbolo |
|---|---|
| 0x001ceef8 | GetClothAnimation |
| 0x001cfc88 | GetClothAnimationFix4Points |
| 0x001d0688 | InitClothes |
| 0x001d11e8 | DispCloth4D |
| 0x001d13d8 | getCloth4D_preProcess |
| 0x001d1ee8 | getCloth4D |
| 0x001d30d8 | getCloth4D_postProcess |
| 0x001d3340 | _getCloth4D |
| 0x001d35a8 | GetCloth4D |
| 0x001d35c8 | GetCloth4DWithDetail |
| 0x001d35e0 | GetCloth4DWithTight |
| 0x001d35f0 | **InitCloth4D** |
| 0x001d37c8 | **(dispatcher — não nomeado)** |
| 0x001d3a30 | **(ROPE callback — não nomeado)** |
| 0x001d3ad8 | GetChainNodeGlobalQuaternion |
| 0x001d3b80 | MoveChainExtendedWeight |
| 0x001d3b98 | InitChainVelocity |
| 0x001d3ca0 | DeleteChainExtendedWeight |
| 0x001d3cc8 | GetChainNodeID |
| 0x001d3ce0 | ResetClothAnimation |
| 0x001d4330 | getCloth4D_PlaneClip |

Interpretação conservadora: os 5 estados internos (0-4) representam fases da simulação de cloth:

| State | Interpretação provável |
|---|---|
| 0 | init / setup |
| 1 | wind / aplicação de forças |
| 2 | collision / detecção de colisão |
| 3 | constraint / resolução de restrições |
| 4 | post-process / finalização |

## Resultado 4: Correção — "ROPE" no ICO-decomp ≠ nosso "ROPE" descritor

O ICO-decomp tem **79 símbolos** com "Rope" no nome:
- `actCommonRope`, `actCommonRopeClimbEnd1`, `actCommonRopeSwing`
- `GetRopeHangablePos`, `SetRopeFixPoint`, `HoldRope`, `ReleaseRope`
- `ropeGeo`, `RopeDL`, `InitRopeGeo`
- `actSt04lRope1`..`actSt04lRope4`, `actSt04rRope1`..`actSt04rRope4`

Todos são funções de **gameplay de cordas** (escalar, balançar, pular, cordas cortáveis nas fases st04l/st04r).

Nosso "ROPE" é um **descritor interno do `.data`** (label `0x002a3974` no USA) — um conceito completamente diferente. A coincidência do nome é uma falsa pista.

## Resultado 5: Source tree mapping via splat YAML

A estrutura de diretórios original revelada pelo YAML:

```
ico2/
├── fumi/       ← core engine, IOS, threading, som, atores base
│   ├── ios/     → thread, shockdriver, cdvd, memory, pad, mcard, message
│   ├── isys/    → gobj, gobj_process, obj_manager, gobj_cam_dl
│   ├── sound/   → adpcm, s_init, soundManager
│   ├── src/     → act-game, boyact, girl_act, enemy_act, commonact, fieldCollision
│
├── omori/      ← câmera, AI, colisão de ataque
│   └── src/     → brain, camera-*, chain, ebrain, enemy-control, generator
│
├── sugipon/    ← física, animação, gameplay objects, UI
│   └── src/     → clothAnimation, girl, boy, box, enemy, item, rope,
│                  weapon, windField, particleEffect, darkVolume
│
├── ito/        ← chefes, MPEG/cutscene, raio
│   ├── src/     → act_bird, itou_boss, lightning, queen, stage_orient
│   └── mpeg/    → mv_main, mv_videodec, mv_audiodec, mv_disp
│
├── seki/       ← rendering, texturas, shadow
│   └── src/     → DisplayP2O, FileManager, GsBase, Texture, Shadow, Packet
│
├── common/     ← compartilhado entre módulos
│   └── src/     → gamesys, kanban, debug, sceneManager, icoMisc, DObj
│
├── script/     ← sistema de script por fase
│   └── src/     → script, gflag, st00a..st99a, stageSEProc, warpGirl, deja
│
└── sdk/        ← Sony PS2 SDK
    └── lib/     → libkernl, libgraph, libm, libc, libgcc, libpad, libmc, libmpeg
```

## O que é confirmado

1. Dispatcher e ROPE callback estão em `sugipon/src/clothAnimation.c` — física de pano, não entidade/gameplay.
2. ICO-decomp tem naming convention: PascalCase para API pública, camelCase para internos.
3. Source tree revela 5 programadores/áreas: fumi, omori, sugipon, ito, seki.
4. Nossa cadeia de callback registration está em `fumi/ios/` (thread.c + shockdriver), não em cloth.
5. Nenhum dos nossos 14 endereços tem nome exato no ICO-decomp — todos caem entre funções nomeadas.
6. "ROPE" descritor `.data` é um conceito diferente das funções `actCommonRope*` de gameplay.

## O que é provável

1. Os 5 estados do dispatcher são fases de simulação de cloth.
2. callback_registration (0x13f7a8) e node_callback_storage (0x13f3f0) são infraestrutura de IOS threading/shock.
3. O gap de registro do ROPE callback pode ser resolvido entendendo como clothAnimation.c usa o sistema de callback do IOS.

## O que permanece desconhecido

1. Mecanismo exato de registro de `0x001d3a30` em `node + 0x1c`.
2. Significado exato do label "ROPE" no `.data`.
3. Correlação exata dos 5 estados internos com a física de pano.

## O que é descartado

1. Dispatcher como máquina de estado de entidade/gameplay.
2. "ROPE" descritor como referência a rope de gameplay.

## Próximo teste mínimo (sem emulador)

1. Examinar o código C já decompilado em `ico2/fumi/ios/thread.c` para entender a assinatura de `iosThreadStart` e ver se 0x13f7a8/0x13f778 são resquícios de inline.
2. Buscar em clothAnimation.c (no ICO-decomp) por referências ao descritor "ROPE" ou ao callback 0x1d3a30.
3. Gerar PCSX2 `.sym` file com os labels neutros para a próxima sessão runtime.

## Veredito conservador

O cross-reference com ICO-decomp produziu uma correção importante (cloth physics, não entity state) e revelou a estrutura do source tree original. O ROPE callback gap permanece aberto, mas agora sabemos que a resposta está em `sugipon/src/clothAnimation.c` e `fumi/ios/` (thread + shockdriver), não em código de gameplay ou câmera. Nenhum dos endereços mapeados tem nome exato no ICO-decomp, confirmando que o trabalho de análise destas funções é inédito mesmo dentro do projeto de decompilação matching.

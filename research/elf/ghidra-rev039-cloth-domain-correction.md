# rev.039 — Cloth Domain Correction

## Data

2026-05-14

## Objetivo

Consolidar a correção de domínio das funções `0x001d37c8` (dispatcher) e `0x001d3a30` (ROPE callback) de "entity/gameplay state" para "cloth physics domain", com base no cross-reference com ICO-decomp (Rev.038). Limpar o mapa mental do projeto e evitar que futuras análises persistam interpretações descartadas.

## Escopo

Incluído:
- correção formal de domínio para dispatcher e ROPE callback
- separação do que é confirmado, provável, possível e descartado
- limpeza de nomenclatura e interpretações antigas
- atualização do `prompt_persona_ico_reconstruction.md`

Excluído:
- nova análise técnica
- runtime validation
- investigação de DATA.DF
- modificação de arquivos do jogo

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/ico-decomp-cross-reference-2026-05-14.md` | cross-reference ICO-decomp (5792 símbolos) |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | estado anterior da análise |
| `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | confirmação runtime |
| `research/elf/ghidra-rev024-internal-state-block-semantics.md` | análise dos 5 blocos internos |
| `research/elf/ghidra-rev023-dispatcher-table-resolution.md` | fundamento do dispatcher |
| `/tmp/ICO-decomp/config/symbol_addrs.txt` | símbolos nomeados do ICO-decomp |
| `/tmp/ICO-decomp/config/ICO-PAL.yaml` | estrutura de subsegmentos e arquivos fonte |

## Correção de domínio

### Antes (Rev.001-037)

O dispatcher `0x001d37c8` e seus 5 blocos internos eram tratados como:

```
candidate_state_dispatcher -> dispatch de estado de entidade/gameplay
state_0..4_block          -> possíveis estados: Yorda, captura, menu, morte, animação, AI
```

O callback `0x001d3a30` era tratado como:

```
rope_record_callback -> callback de entidade, update loop
```

### Depois (Rev.038-039)

O dispatcher `0x001d37c8` e `0x001d3a30` residem em **`sugipon/src/clothAnimation.c`** (confirmado por símbolos vizinhos no ICO-decomp: `InitCloth4D`, `GetCloth4D`, `getCloth4D_preProcess`, `getCloth4D_postProcess`, `getCloth4D_PlaneClip`).

```
dispatcher 0x001d37c8 -> dispatch interno de simulação de cloth (pano)
state_0..4_block      -> fases da simulação (init, wind, collision, constraint, post-process)
0x001d3a30            -> callback de atualização de cloth (não de entidade genérica)
```

### Evidência

1. **Símbolos vizinhos**: o bloco `0x001d35f0` (InitCloth4D) está 472 bytes antes do dispatcher; `0x001d3ad8` (GetChainNodeGlobalQuaternion) está 784 bytes depois. Todos os símbolos no range `0x001d3000-0x001d4500` são de física de cloth.

2. **Source tree**: o splat YAML mapeia o range `0xced48-0xd5458` (PAL) para `sugipon/src/clothAnimation.c`. Não há sobreposição com código de entity/gameplay.

3. **Naming consistency**: funções vizinhas seguem padrão `Cloth4D` / `clothAnimation` / `ChainNode` — coerente com sistema de simulação de pano com vértices, restrições e cadeias.

## O que é confirmado

1. `0x001d37c8` e `0x001d3a30` estão em `sugipon/src/clothAnimation.c`.
2. O dispatcher com jump table 0x00618fb0 e 5 blocos internos existe e funciona conforme modelado.
3. `0x001d3a30` é o único caller estático do dispatcher (confirmado em runtime Rev.025).
4. Os 5 blocos internos representam fases de processamento dentro de uma única função de cloth.
5. Nenhuma função de gameplay (girl, boy, camera, enemy) está no mesmo range de código.

## O que é provável

1. Os 5 blocos internos mapeiam para: init (0), wind/forces (1), collision (2), constraint solve (3), post-process (4).
2. `0x001d3a30` é chamado via callback registration do IOS para atualizar a simulação de cloth a cada frame.
3. O descritor "ROPE" no `.data` não tem relação com gameplay ropes — é um identificador interno de registro de objeto, possivelmente "Register Object Physics Entity" ou similar.

## O que permanece desconhecido

1. Mecanismo exato de registro de `0x001d3a30` em `node + 0x1c`.
2. Significado exato do label "ROPE" no `.data` (endereço 0x002a3974 USA).
3. Como os 5 estados de cloth se correlacionam com os parâmetros de física de pano (vento, colisão, etc.).
4. Se o USA tem diferenças estruturais relevantes na região de cloth vs PAL.

## O que é descartado

1. Dispatcher como máquina de estado de entidade/gameplay (Yorda, captura, menu, morte, animação, AI).
2. "ROPE" como referência a gameplay rope/corda no contexto do descritor `.data`.
3. Qualquer interpretação dos 5 blocos como transições de estado de gameplay.

## Nomenclatura atualizada

| Nome antigo | Nome novo | Justificativa |
|---|---|---|
| candidate_state_dispatcher | cloth_dispatcher_dispatch | dispatcher interno de simulação de cloth |
| candidate_state_jump_table | cloth_dispatcher_jumptable | jump table em 0x00618fb0 |
| state_0_block | cloth_state_0_init_like | setup/inicialização |
| state_1_block | cloth_state_1_wind_like | aplicação de forças/vento |
| state_2_block | cloth_state_2_collision_like | detecção de colisão |
| state_3_block | cloth_state_3_constraint_like | resolução de restrições |
| state_4_block | cloth_state_4_postprocess_like | pós-processamento |
| rope_record_callback | cloth_update_callback | callback de atualização de cloth |
| ROPE (descritor .data) | cloth_record_label (provisório) | label interno de registro |

Estes nomes são **provisórios baseados em contexto**. Não são conclusões de gameplay.

## Impacto no projeto

1. **AGENTS.md**: já atualizado em Rev.038 com a correção de domínio.
2. **prompt_persona_ico_reconstruction.md**: atualizado nesta revisão.
3. **Próximas análises**: qualquer investigação futura sobre `0x001d37c8` ou `0x001d3a30` deve tratá-los como código de cloth physics, não de gameplay.
4. **ROPE gap**: o mistério do registro em `node + 0x1c` continua — mas agora sabemos que a resposta está em como o sistema de cloth (sugipon) se registra no IOS threading (fumi), não em uma máquina de estado de entidade.

## Próximo teste mínimo

### Sem emulador

1. Examinar `ico2/fumi/ios/thread.c` (decompilado no ICO-decomp) para entender a assinatura de `iosThreadStart` e ver se os wrappers `0x13f7a8`/`0x13f778` são função real ou resíduo de inline.
2. Examinar a asm de `clothAnimation.c` no ICO-decomp em torno de `InitCloth4D` para entender o fluxo de chamada que leva ao dispatcher.

### Com emulador

1. Breakpoint em `0x0013f7a8` capturando `a1` quando `a3 == 0x13` — para ver qual callback é registrado.
2. Breakpoint em `0x001d37c8` para monitorar distribuição de state_id durante gameplay (ver se cloth roda em loop).

## Veredito conservador

Rev.039 encerra a ambiguidade de domínio que persistiu desde Rev.021. O dispatcher `0x001d37c8` e o callback `0x001d3a30` são código de simulação de cloth physics no arquivo `sugipon/src/clothAnimation.c`, não máquina de estado de entidade/gameplay. As interpretações antigas (Yorda, captura, menu, morte, AI) estão formalmente descartadas. O gap de registro do callback permanece aberto, mas agora delimitado ao sistema de interação entre cloth (sugipon) e IOS threading (fumi).

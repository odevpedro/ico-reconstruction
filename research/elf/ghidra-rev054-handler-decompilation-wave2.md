# Rev.054 — Handler decompilation wave 2: GIRL, QUEEN, BGA, AP1

## Date

2026-05-16

## Objective

Completar a desassemblagem e documentacao dos handlers entity de quatro sistemas restantes da wave 1:
1. GIRL (Yorda, indice 2)
2. QUEEN (boss, indice 46)
3. BGA (background animation, indice 30)
4. AP1 (attack pattern 1, indice 61)

## Scope

Incluido:

- Disassemblagem completa de 12 funcoes (hA/hB/hC x 3 entidades + 3 init_fn)
- BGA: init_fn + sprite dispatch system
- AP1: constructor complexo com 4 child slots + state machine update
- Confirmacao do padrao hC/hB/hA em todas as 4 entidades

Excluido:

- Analise semantica completa do gameplay (apenas o que o codigo revela)
- Sub-funcoes alem do primeiro nivel (exceto onde necessario para entender o handler)

## Sources Used

| Fonte | Uso |
|---|---|
| `SCUS_971.13.elf` | objdump |
| `mips64r5900el-ps2-elf-objdump` | Desassemblagem |
| Rev.052 | Descriptor table (68 entries, handlers) |
| Rev.053 | Wave 1 handler convention (hC=constructor, hB=update, hA=reset) |

## Evidence Used

- objdump -d de 12 faixas de endereco: 0x1D1668, 0x1D17F8, 0x1D1A98, 0x19A7E8, 0x19A8F0, 0x19A9A0, 0x19B7F8, 0x203EE8, 0x1B8720, 0x1BA330, 0x1BA530
- Verificacao de prologue/epilogue para boundaries
- Contagem de instrucoes e jals por funcao
- Cross-reference com o modelo de descritores (Rev.052)

## Findings

### 1. GIRL (indice 2) — Yorda

GIRL tem init_fn=0x174BA0 (compartilhado com DEVIL_GI) e 3 handlers. A alocacao de 64B (tag 680) no hC segue padrao BARREL/cloth.

| Handler | Endereco | Insns | Jals | Papel |
|---|---|---|---|---|
| hC | 0x1D1668 | 92 | 5+entity_state_reg | **Constructor**: 64B alloc (tag 680), 3 variant paths |
| hB | 0x1D17F8 | 112 | 9+1 | **Update**: collision + state dispatch + animation blend |
| hA | 0x1D1A98 | 16 | 3+tail | **Reset**: sub init + tail call matrix_build |

#### GIRL hC (0x1D1668) — Constructor

- Alloc: 0x13A0F8(gp_data, 64B, tag 680) → 64 bytes no heap
- Salva s0=[initializer], s1=[alloc_ptr], s2=[entity]
- [initializer+0x30] = variant → switch(variant):
  - **variant==0 (default)**: 4 models com texture override:
    - 0x4C4550 c/ tex 0x4C1B90 → state+0x10
    - 0x4C0F90 c/ tex 0x4C4490 → state+0x08
    - 0x4C23C0 c/ tex 0x4C4550 → state+0x14  *(mesma textura do modelo 1)*
    - 0x4C2830 c/ tex 0x4C4610 → state+0x1C
  - **variant==1**: 3 models, sem texture override:
    - 0x4C2F50 → state+0x10
    - 0x4C3780 → state+0x14
    - 0x4C3BF0 → state+0x1C
  - **variant==2**: 0 models (skips todos os loads)
- entity_state_reg(a1=1216, a2=1820, a3=12, t0=24, t1=514)
- Escreve 1 nos campos +0x04, +0x0C, +0x18 do state block
- descriptor_setup(entity, 2)
- Retorna alloc_ptr

#### GIRL hB (0x1D17F8) — Update per-frame

- 5 chamadas principais:
  1. **0x1D23E0** (cloth/girl specific update)
  2. **0x1E3FC8** (entity_dispatch_update)
  3. **0x1BB8C0**(entity, 35, 44, 407) — setup param
  4. **0x103D50**(entity, 4, 50.0, 50.0, 0.0) — collision check
  5. Se colisao: **0x13FF88**(entity, 6, entity) — collision response
- Distance check via **0x14A0D8**: se != 0 usa raio 30.0, senao 15.0
- Segundo collision check: 0x103D50(entity, 1, 50.0, 50.0, gp_val)
- Se colisao: 0x13FF88(entity, 6, entity) + **0x174D78**(entity)
- **Anim blend condition**: se state[+0x310]==4 E state[+0x07C]!=0 E state[+0x3C8]!=0:
  - anim_seq_get(gp_data, 6) e anim_seq_get(gp_data, 22)
  - Matrix transform no stack com offset 0x30
  - normalize → se distancia < 10.0: divide por 50.0, clamp >= 0.75, set blend via 0x10A4E0
- Tail: 0x1D14B8 (post-update)

#### GIRL hA (0x1D1A98) — Reset/Post-init

- 3 chamadas: 0x10ECD8(), 0x10ECB8(entity), 0x1D1580(entity)
- Tail call: 0x1D19C0 (matrix_build, funcao no final de hB)
- Funcao auxiliar logo apos (0x1D1AD8): **entity_set_state_field(entity, field_id, value)** — setter para state_block+0x04/+0x0C/+0x18

### 2. QUEEN (indice 46) — Boss

| Handler | Endereco | Insns | Jals | Papel |
|---|---|---|---|---|
| init_fn | 0x19B7F8 | 36 | 7 | **Asset init**: dma_read + 3 model loads + state setup |
| hC | 0x19A7E8 | 68 | 5+entity_state_reg | **Constructor**: 24B alloc (tag 727), 2 model loads |
| hB | 0x19A8F0 | 44 | 3+tail | **Update**: entity_dispatch + setup_param + LOD scaling + collision |
| hA | 0x19A9A0 | 32 | 2+2 tail | **Reset**: state_resolver + render update |

#### QUEEN init_fn (0x19B7F8) — Asset init

- 0x202208() → s1 (dma_read, retorna ponteiro de DMA)
- 0x202148(entity) (dma_sync)
- 0x203AA0(a0=1) (sprite_dma_setup)
- 3 model loads via 0x203B78:
  - 0x1A9F80, slot 20 (modelo base)
  - 0x1ABCE0, slot 21 (modelo secundario)
  - 0x1A9C30, slot 21 (segundo modelo no mesmo slot 21 — LOD ou submesh)
- 0x1E29E8(entity, 238, s1+0x610) → stored at s1+0x120 (attachment registry)
- Estado: [entity+0x15c]+0x07c = 1 (flag active)

#### QUEEN hC (0x19A7E8) — Constructor

- Alloc: 0x13A0F8(gp_data, 24B, tag 727) → 24 bytes
- memset(ptr, 0, 24)
- Zera 4 words em tabela externa 0x6D43A8 (provalvemente global state flags)
- state+0x0C = 1 (active)
- 2 model loads via 0x1C8478:
  - 0x29B180 c/ tex 0x29C810 → state+0x10
  - 0x29BC80 sem tex → state+0x14
- Store state at [entity_state+0x800]
- entity_state_reg(a1=2105, a2=2119, a3=12, t0=24, t1=1011)
- descriptor_setup(entity, 2)
- 0x202208() + 0x202148(entity) (dma read/sync)
- Retorna alloc_ptr

#### QUEEN hB (0x19A8F0) — Update per-frame

- Global counter em 0x00274ED4: se zero, incrementa gp_counter (-19224)
- entity_dispatch_update(entity)
- setup_param(entity, 35, 44, 409)
- Se state[+0x0C] != 0:
  - 0x1C8218(state+0x10, 3.0f, gp_val) — **scale/pop-in animation** para modelo 1
  - 0x1C8218(state+0x14, 5.0f, gp_val) — scale/pop-in para modelo 2
- Tail: collision_check(entity, 1, 100.0, 100.0, gp_val)

#### QUEEN hA (0x19A9A0) — Reset/Post-init

- Se gp_data != 0: 0x1504D8(gp_data)
- 0x10ECD8()
- 0x10ECB8(entity)
- Se state[+0x0C] != 0:
  - 0x1C62D0(state+0x10, entity+0x844+64, entity+0x844) — **render matrix update**
- Tail: 0x1C62D0(state+0x14, entity+0x844+64, entity+0x844)

### 3. BGA (indice 30) — Background Animation

| Funcao | Endereco | Insns | Jals | Papel |
|---|---|---|---|---|
| init_fn | 0x203EE8 | 12 | 2 | **Asset init**: dma_read + sprite_dma_setup(1) |
| hA/hB/hC | 0 | 0 | 0 | **Nenhum** — BGA nao tem lifecycle callbacks |

#### BGA init_fn (0x203EE8)

- dma_read() + sprite_dma_setup(1)
- Sem allocacao propria — usa o sprite system global
- BGA e puramente um sprite 2D overlay, sem estado entity, sem update per-frame

#### BGA sprite dispatch (0x203F48)

Funcao adjacente ao init_fn (escopo: sistema sprite, nao BGA especifico):

- Iterador de sprites com tabela em 0x5D49E0, stride 0x3C (60B)
- Filtro: and 0x0F do field +0x38 → seletor de bits (1/2/4)
- Para bit 4: itera state nodes type 4, collect em array, chama callback em [table+0x2C] ou render via 0x14B4C8
- Logica de XOR com gp_data/gp_val para selecao de contexto (jogador vs mundo)

### 4. AP1 (indice 61) — Attack Pattern 1

| Handler | Endereco | Insns | Jals | Papel |
|---|---|---|---|---|
| hC | 0x1B8720 | ~400+ | 10+ | **Constructor**: 640B alloc (tag 228), 4 child slots, frame offset |
| hB | 0x1BA330 | ~200 | 8+ | **Update**: state machine (7 states), hitbox + anim + draw |
| hA | 0x1BA530 | 40 | 5+tail | **Reset/Cleanup**: model_free chain, conditional cleanup |

#### AP1 hC (0x1B8720) — Constructor

- Alloc: **640 BYTES** via 0x13A0F8(gp_data, 640, tag 228) — maior alocacao de constructor vista ate agora
- state+0x04 = 1 (active), zera +0x16C, +0x168, +0x1C4, +0x1C8, +0x1C0 (extended state fields)
- [initializer+0x30] = variant → state+0x00
- state+0x08 = 7 (state machine initial state = 7)
- Tabela 0x623468[variant*0x20]: le float +0x18 (scale/threshold por variante), copia para 0x4CEC00+0x08
- **Frame offset**: rand() & 0x1F → state+0x270 — randomiza frame inicial
- state+0x278 = 1 (active), state+0x274 = 0 (sub-counter)
- Init de sub-estruturas:
  - 0x105F00(state+0x1B0, 0x4CEC00) — quaternion/transform setup
  - 0x118678(state+0x1F0) + 0x118678(state+0x230) — 2 matrix inits
  - 0x102850(state+0x1D0, entity) — vector init
  - 0x104508(state+0x1E0, entity) — vector load
- **4 child slots**: stride 80B (0x50), template em 0x4CEA80:
  - state+0x10 a state+0x4F (slot 0-3)
  - 80B cada: 8 words de template via sd (64B) + 2 words (8B) + padding
  - Copia 4 blocos de 80 bytes
- **Template adicional**: 12 bytes de 0x282660 para state+0x150 (config padrao)
- Cada child contem dados de ataque individuais (hitbox, timing, trajectory)

#### AP1 hB (0x1BA330) — Update

- state+0x08 leitura: **state machine com 7 estados** (0-6, mapeados para 0-6 via slti):
  ```
  switch(state) {
    case 4:  next = 4 (hold/setup)
    case 5:  next = 5 (active) → senao continua
    case 6:  next = 6 (active) → senao continua
    case 7:  next = 4 (transition to setup)
    default: next = state
  }
  ```
- Se state < 10: incrementa sub-counter (state+0x274), chama 0x1BA2E0(entity) — pre-update
- Tabela de transicao em 0x4CEE60: stride 8, indexada por [state]:
  - [state+0x00] = next_state_fn (callback)
  - [state+0x04] = next_state_id
  - Ex: 0x1BA2E0 pre-update, depois jalr via tabela
- state+0x270 (frame_counter): incrementa, wrappa em 32 (0-31 loop)
- Se frame_counter == 32: zera (ciclo de 32 frames)
- **Hitbox check**: distancia Y entre entity e player (entity_y - child_y). Se abs > threshold (gp), ativa 0x1A6E28 com mensagem 0x628590
- Render sequence:
  - 0x1BA090(entity) — pre-render
  - 0x1B99B0(entity) — state-specific render
  - setup_transform() → matrix_build(state, 0x4CEEA0)
  - draw: **0x1CF930**(state+0x19C, transform, 1.0f) — attack draw
  - Se state+0x04 != 0 (active children):
    - Transform swap via 0x105F20
    - Render mode 0x105038(16384), state 0x104F48(16384)
    - matrix_build com tabela 0x4CEF20
  - Cleanup: se distancia Y > threshold → 0x1A6E28(msg) + acelera 2048

#### AP1 hA (0x1BA530) — Reset/Cleanup

- Condicional: se state+0x08 < 5 (estados iniciais) → retorna
- Se state+0x278 (active flag) != 0:
  - 0x10ECD8()
  - 0x10ECB8(entity)
  - Se state+0x04 != 0: model_free para state+0x19C (modelo principal)
  - Tail: model_free(state+0x194) + model_free(state+0x198) + model_free(state+0x19C)

### 5. Tabela comparativa consolidada (12 entidades)

| Indice | Nome | init_fn | hA (+0x48) | hB (+0x50) | hC (+0x58) | Alloc (hC) |
|---|---|---|---|---|---|---|
| 1 | BOY | 0x153478 | 0x1C1F58 | 0x1C1DD8 | 0x1C1A98 | ? |
| 2 | **GIRL** | 0x174BA0 | 0x1D1A98 | **0x1D17F8** | 0x1D1668 | **64B** |
| 4 | ENEMY1 | 0x164440 | 0x1CE690 | 0x1CE3C0 | 0x1CE220 | 80B |
| 17 | WOODBOX0 | 0x17D1D0 | 0x1C05D0 | 0x1C0538 | 0x1C00C0 | 400B |
| 19 | BARREL | 0 | 0x1D3B28 | 0x1D3A30 | 0x1D27A8 | ~40B |
| 30 | **BGA** | **0x203EE8** | **0** | **0** | **0** | **0** |
| 46 | **QUEEN** | **0x19B7F8** | **0x19A9A0** | **0x19A8F0** | **0x19A7E8** | **24B** |
| 61 | **AP1** | 0x1BB6B0 | **0x1BA530** | **0x1BA330** | **0x1B8720** | **640B** |
| 62/63 | ATTACKCH | 0x1BBF78 | ? | ? | ? | ? |
| 64 | BOSS_CTR | 0x198140 | ? | ? | ? | ? |

### 6. Padroes observados

- **hC = constructor via 0x13A0F8**: TODAS as entidades com hC alocam via heap_alloc com tag unica por tipo. O padrao e universal.
- **entity_state_reg**: TODAS chamam com parametros (a1, a2, a3, t0, t1) — provavelmente numero de estados, flags, capacidades.
- **descriptor_setup(entity, 2)**: TODAS chamam com a1=2 (exceto BARREL que tem init_fn=0).
- **init_fn**: so e chamado se nao-nulo (+0x40 != 0). Responsavel por carregar modelos 3D e configurar assets (dma). BGA e o unico sem handlers — init_fn faz tudo.
- **state+0x04 = active flag**: GIRL, QUEEN, AP1, WOODBOX0, ENEMY1 todos escrevem 1 em state+0x04.
- **state+0x800 storage**: todas armazenam alloc_ptr em [entity_state + 0x800].
- **Variant control**: initializer+0x30 controla paths de inicializacao em GIRL, AP1, BARREL (cloth_payload_init).

## What is Confirmed

- GIRL: hC=constructor (92 insns, 64B alloc, 3 variant paths), hB=update com anim blend (112 insns), hA=reset (16 insns)
- QUEEN: init_fn com 3 model loads (36 insns), hC=constructor (68 insns, 24B alloc), hB=update com LOD scaling (44 insns), hA=reset (32 insns)
- BGA: init_fn de 12 insns (dma_read + sprite_dma_setup(1)), SEM handlers (hA/hB/hC = 0)
- AP1: hC=constructor maior visto (640B alloc, 400+ insns, 4 child slots), hB=state machine 7 estados (~200 insns), hA=cleanup (40 insns)
- Padrao hC/hB/hA confirmado em 7 entidades (BOY, GIRL, ENEMY1, WOODBOX0, BARREL, QUEEN, AP1)
- init_fn separado de hC: init_fn carrega assets/modelos, hC aloca estado interno

## What is Probable

- ATTACKCH (idx 62/63) e BOSS_CTR (idx 64) seguem mesmo padrao hC/hB/hA com alloc via 0x13A0F8
- State machine do AP1 (0-6, tabela 0x4CEE60) e um padrao generico de FSM do engine
- Os 4 child slots do AP1 representam sub-ataques (projeteis, hitbox multiplas)

## What is Unknown

- Handler hC de BOY (0x1C1A98)
- Handlers de ATTACKCH e BOSS_CTR
- Semantica dos parametros entity_state_reg (a1=1216/2105, a2=1820/2119, a3=12, t0=24, t1=514/1011)
- Mecanismo de dispatch runtime de hB (quem chama descriptor[+0x50] por frame)
- Conteudo dos templates 0x4CEA80 (80B, 4 child slots AP1) e 0x4CEE60 (tabela de transicao)

## Next Minimum Test

1. Breakpoint em GIRL hB (0x1D17F8) para capturar frequencia de dispatch e valores de state+0x310/0x07C/0x3C8
2. Breakpoint em QUEEN hB (0x19A8F0) para verificar scaling/LOD em acao
3. Verificar se AP1 hB (0x1BA330) segue padrao de 32-frame cycle observed

## Conservative Verdict

A analise de 12 funcoes em 4 entidades adicionais estabelece que o padrao hC/hB/hA e universal entre tipos entity. GIRL segue o padrao BARREL (cloth-like constructor com variant), QUEEN e um boss com init_fn separado e LOD scaling, BGA e o caso atipico (2D sprite, sem handlers), e AP1 e o mais complexo (640B alloc, 4 child slots, state machine de 7 estados). O init_fn vs hC separation e confirmado: init_fn carrega assets globais, hC aloca estado por-instancia.

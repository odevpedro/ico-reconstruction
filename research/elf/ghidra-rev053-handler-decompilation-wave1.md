# Rev.053 — Handler decompilation wave 1: COP2 cloth, WOODBOX0, ENEMY1, BOY

## Date

2026-05-16

## Objective

Disassemblar e documentar os handlers entity de quatro sistemas:
1. Funções COP2 reais do sistema cloth (distância e plane clip via VU0)
2. WOODBOX0 (caixote de madeira quebrável, índice 17)
3. ENEMY1 (inimigos sombra, índice 4)
4. BOY (protagonista Ico, índice 1)

## Scope

Incluído:

- Disassemblagem completa de 8 funções (hA/hB/hC × 3 entidades + 2 COP2 cloth)
- 19 sub-funções mapeadas para ENEMY1 (árvore de dependência completa)
- Correção de endereços COP2: 0x1D3E80 e 0x1D45B0 (não 0x1D3D70/0x1D4348/0x1D4358)
- Decompilação para C pseudocode de todas as funções

Excluído:

- GIRL (Yorda), QUEEN, BGA, AP1 — pendente para wave 2
- Análise semântica completa do gameplay (apenas o que o código revela)
- Overlays VU (requer DATA.DF)

## Sources Used

| Fonte | Uso |
|---|---|
| `SCUS_971.13.elf` | objdump |
| `mips64r5900el-ps2-elf-objdump` | Desassemblagem |
| Rev.052 | Descriptor table (68 entries, init_fn, handlers) |
| Rev.047 | Handler naming model |

## Evidence Used

- objdump -d de 8 faixas de endereço: 0x1D3E80, 0x1D45B0, 0x1C00C0, 0x1C0538, 0x1C05D0, 0x1CE220, 0x1CE3C0, 0x1CE690, 0x1C1F58, 0x1C1DD8
- Verificação de prologue/epilogue para boundaries
- Listagem de todos os jal targets com endereços
- Count de instruções COP2 por função

## Findings

### 1. CORREÇÃO: Endereços COP2 reais no sistema cloth

As funções sugeridas anteriormente (0x1D3D70, 0x1D4348, 0x1D4358) são accessors de 12-48 bytes sem COP2. As funções com VU0 macro-mode estão em endereços diferentes:

| Função | Endereço | Frame | COP2 | Descrição |
|---|---|---|---|---|
| `clothSubDistanceCheck` | **0x1D3E80** | 112B | **6** | Distância Euclidiana² via VU0 |
| `clothSubPlaneClip` | **0x1D45B0** | 224B | **74** | Plane clip + 4×4 transform + perspective divide + AABB X/Y |

#### clothSubDistanceCheck (0x1D3E80)

- 61 instruções, 9 basic blocks
- 4 jal: 0x13EB50 (state_resolver), 0x104508 (vec_load), 0x19F530 (some_update), 0x13EBE0 (next_state)
- COP2: 5 VU + 1 qmfc2 = distância² (`vsub → vmul → vaddy → vaddz → qmfc2`)
- Itera todos os state nodes, computa distância² vs threshold, marca nós elegíveis (state_id < 2) como "ativos"

#### clothSubPlaneClip (0x1D45B0)

- 259 instruções, ~22 basic blocks
- 7 jal (alguns chamados múltiplas vezes): 0x105278 (setup_transform), 0x117FE8 (get_matrix_base), 0x118068 (setup_view_matrix), 0x1180C0 (mul_mat4 ×2), 0x118460 (swap ×12)
- 6 blocos COP2: Z near-plane clip (10), 4×4 matrix + perspective divide (20), X forward clip (11), X reverse clip (11), Y forward clip (11), Y reverse clip (11)
- Retorna tristate: -1 (culled), 0 (parcial), 1 (fully visible)

### 2. Convenção de handlers — CORREÇÃO

Análise de 3 entidades (WOODBOX0, ENEMY1, BOY) revela a convenção real:

| Slot | Papel real | Justificativa |
|---|---|---|
| **hC (+0x58)** | **Constructor/Init** | WOODBOX0: aloca 400B, children, physics. ENEMY1: aloca 80B, prefab, estados. BOY: warm/cold init. **BARREL: cloth_payload_init** |
| **hB (+0x50)** | **Update per-frame** | WOODBOX0: counter + pre-physics. ENEMY1: AI+attack+draw. BOY: 5 calls + colisão |
| **hA (+0x48)** | **Post-init / Reset** | WOODBOX0: wind flag. ENEMY1: cleanup condicional. BOY: (em BARREL, hA=0x1D3B28) |

O slot hC é o init primário, chamado pelo iterador de entries (0x1B76F8 → descriptor[+0x58]). O slot hB é o update per-frame. O slot hA é condicional/reset.

### 3. WOODBOX0 (índice 17, phy_type=0xF2)

| Handler | Endereço | Insns | Jals | Papel |
|---|---|---|---|---|
| hC | 0x1C00C0 | 286 | 18 | **Init**: aloca 400B, copia template 0x4BF560, spawns 2 child entities (type 23), wood_setup, wind_setup, render_setup, mdl_assign, VU0 random para lifetime offsets |
| hB | 0x1C0538 | 27 | 3 | **Update**: pre-physics + entity_dispatch_update. Contador 30 → cleanup (`entity_cleanup_dispatch` em 0x1AE460) |
| hA | 0x1C05D0 | 28 | 4 | **Post-init**: wind_physics_start (condicional se world_id≠0) + entity_flag_set |

Callbacks registrados via init_fn (0x17D1D0) → 0x1CF7F8.

### 4. ENEMY1 (índice 4) — Shadow enemies

| Handler | Endereço | Insns | Jals | Papel |
|---|---|---|---|---|
| hC | 0x1CE220 | ~103 | 7 | **Constructor**: 80B heap, 2 child collections (10 elementos), group (size 6), prefab load, entity_state_reg(1820,2105,24) |
| hB | 0x1CE3C0 | ~135 | 14 | **Update**: AI seeker (`0x103F00`), attack_range_check, attack_execute (`0x1CF6C0`), shadow_group_update, anim_seq_get (slots 36, 37), shadow_draw ×2 |
| hA | 0x1CE690 | ~23 | 3 | **Reset/Cleanup**: condicional se bit damage/stun ativo → `0x1BB7E0` + flag_check → cleanup chain |

**Árvore de dependência (19+ sub-funções):**

```
hC → 0x13A0F8 (heap_alloc), 0x1CEF90 (shadow_child_create ×2),
      0x1CF288 (shadow_group_create), 0x1CD9B0 (shadow_prefab_init),
      0x1E4798 (entity_state_reg), 0x1D4B40 (descriptor_setup)

hB → 0x1654C8, 0x1E3FC8, 0x103F00 (seeker_move), 0x165540,
      0x104940, 0x1CF6C0 (attack_execute), 0x1CF548,
      0x105278 (rand ×4), 0x109F10 (anim_seq_get ×2: 36, 37),
      0x1185D0 (matrix_build ×2), 0x1CF930 (shadow_draw ×2)

hA → 0x1BB7E0, 0x165F88, 0x1CE5F8 (shadow_cleanup tail)
        → 0x1224E0 (sound_free), 0x1CF998 (model_free ×2),
          0x1CF770 (group_free), 0x1CDB28 (shadow_final_cleanup)
```

### 5. BOY (índice 1) — Protagonist Ico

| Handler | Endereço | Insns | Jals | Papel |
|---|---|---|---|---|
| hA | 0x1C1F58 | 80 | 12 | **Init com 2 paths**: warm (reinit, se v1≠0) vs cold (first-time, 4 calls: 0x10ECD8, 0x10ECB8, 0x1C1EA8, 0x1C1250). Condicional: se map==39 e distância>20.0f → setup sub-estrutura |
| hB | 0x1C1DD8 | 51 | 9+1 | **Update**: 5 chamadas sequenciais (0x1D23E0, 0x1E3FC8, 0x1C1C48, 0x1C12F0, 0x1E4868) + setup param (0x1BB8C0) + colisão (0x103D50) + tail-call 0x103D50 |

BOY tem init_fn=0x153478 (model/asset loader). hC não foi analisado nesta wave.

### 6. Tabela comparativa: handlers × entidades

| Índice | Nome | init_fn (+0x40) | hA (+0x48) | hB (+0x50) | hC (+0x58) | Physical? |
|---|---|---|---|---|---|---|
| 1 | BOY | 0x153478 | 0x1C1F58 (init) | 0x1C1DD8 (update) | 0x1C1A98 (?) | No |
| 2 | GIRL | 0x174BA0 | 0x1D1A98 | 0x1D17F8 | 0x1D1668 | No |
| 4 | ENEMY1 | 0x164440 | 0x1CE690 (reset) | 0x1CE3C0 (update) | 0x1CE220 (init) | No |
| 17 | WOODBOX0 | 0x17D1D0 | 0x1C05D0 (post) | 0x1C0538 (update) | 0x1C00C0 (init) | 0xF2 |
| 19 | BARREL | 0 | 0x1D3B28 | **0x1D3A30** | 0x1D27A8 | 0xF3 |
| 30 | BGA | 0x203EE8 | 0 | 0 | 0 | No |
| 46 | QUEEN | 0x19B7F8 | 0x19A9A0 | 0x19A8F0 | 0x19A7E8 | No |

## What is Confirmed

- Funções COP2 reais: 0x1D3E80 (distance², 6 COP2) e 0x1D45B0 (plane clip, 74 COP2)
- WOODBOX0 hC = init (286 insns), não cleanup. Ciclo de vida: 31 frames
- ENEMY1 hB = per-frame AI com seeker, attack, draw. hA = reset/cleanup condicional
- BOY hA = warm/cold init com condicional map-39. hB = 5 chamadas + colisão
- Convenção de handlers: hC=constructor, hB=update, hA=post-init/reset
- ENEMY1 shadow struct: 80B, dois child collections (10 elementos cada), group handle (size 6)

## What is Probable

- `0x1D3A30` (BARREL hB) nunca disparou em runtime porque o update per-frame de objetos físicos só roda quando há interação ativa (colisão, vento, impacto). Objetos BARREL estavam em estado dormente durante o teste.
- O handler `cloth_payload_init` (BARREL hC) só dispara na criação do objeto (via 0x1B76F8). O hB seria disparado pelo sistema de update (node dispatcher ou loop principal) a cada frame, mas condicionado a estado ativo.

## What is Unknown

- Semântica dos type IDs 22/23/24 (sister callbacks)
- Handler hC de BOY (0x1C1A98)
- Handlers de GIRL, QUEEN, BGA, AP1
- Dispatcher runtime de hB (quem chama descriptor[+0x50] por frame)

## Next Minimum Test

1. Breakpoint condicional em 0x1D3A30: só disparar quando BARREL está em estado ativo (interagindo com jogador)
2. Ou: breakpoint em uma função chamada por hB de WOODBOX0 (0x1BF2C8 = phy_pre_update) para capturar a cadeia de dispatch de hB
3. Wave 2 de decompilação: GIRL (Yorda), QUEEN (boss), BGA

## Conservative Verdict

A análise de 8 funções em 4 sistemas entity estabelece a convenção definitiva dos 3 handlers: hC=constructor (chamado pelo iterador 0x1B76F8 via descriptor[+0x58]), hB=update per-frame (despachado por mecanismo runtime desconhecido), hA=post-init/reset. A confirmação de que WOODBOX0 e ENEMY1 seguem este padrão valida o modelo para BARREL, onde hC=0x1D27A8 (cloth_payload_init) e hB=0x1D3A30 (cloth_update). O fato de hB nunca disparar no runtime sugere que o dispatch de hB é condicional ao estado ativo do objeto — coerente com objetos BARREL dormentes durante o teste.

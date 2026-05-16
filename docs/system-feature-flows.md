# System & Feature Flows — ICO Reconstruction

> Documento vivo. Atualizado sempre que uma feature for criada ou modificada.
> **Ultima atualizacao:** 2026-05-16 (Rev.057 — cloth dispatcher C model: 5-state FSM confirmado; clothSubForceApply model; ENEMY1 hC model; WOODBOX0 hC ASM-HOLD)

---

## Indice

- [Cloth Physics Dispatch Flow](#cloth-physics-dispatch-flow)
- [Physics Object Type Table Initialization](#physics-object-type-table-initialization)

---

## Entry Table Descriptor Iteration Flow

> **Status:** Fluxo mapeado (Rev.050). Funcao `0x1B76F8` identificada como iteradora.

### Visao Geral

O sistema de inicializacao cloth do ICO usa uma tabela de entries (entrada/sala/zona) em `0x002A4C48` (stride 0x4C) que referencia descritores em `0x002A31B8` (stride 0x64). Cada descritor tem 3 slots de callback que controlam o ciclo de vida de objetos fisicos.

O fluxo completo e:

```
Entry Table (0x002A4C48, stride 0x4C)
  |
  | entry[+0x46] = descriptor index
  v
Descriptor Table (0x002A31B8, stride 0x64)
  |
  | descriptor+0x58 = cloth_payload_init (handler_c)
  | descriptor+0x50 = callback update (handler_b)
  | descriptor+0x48 = callback auxiliar (handler_a)
  v
Physics Type Table (0x001A48A0, stride 0x64) -- nomes APENAS, handlers duplicados
```

### Fluxo detalhado

```
1. Load room/zone data
   └─> Entry table (0x002A4C48) populada com entries por zona

2. 0x1B76F8(entry_index) -- Descriptor iteration
   ├─> Calcula entry VA = 0x002A4C48 + entry_index * 0x4C
   ├─> Le descriptor_idx = entry[+0x46]
   ├─> Le entry[+0x2C], entry[+0x30] (parametros numericos)
   ├─> Testa condicoes de tipo (idx == 0x0F?, 0x04?, 0x21?)
   │
   ├─> Setup espacial
   │   ├─> Le floats de entry[+0x0C..+0x20] (posicao/rotacao/escala)
   │   ├─> Calculos com mul.s, div.s, neg.s (transformacao)
   │   └─> Chama descriptor[+0x38] se nao-nulo (callback opcional)
   │
   ├─> Prepara descritor alvo
   │   ├─> s0 = 0x002A31B8 + descriptor_idx * 0x64
   │   ├─> Le entry[+0x48] >> 14 & 7 (subcategoria)
   │   ├─> Le entry[+0x47] & 0x1f (subtype)
   │   ├─> Chama 0x240D40(s0, ...)
   │   ├─> Chama 0x19F310() -- alocacao
   │   ├─> [context+0x15C] = retorno (cria entidade)
   │   └─> [entity+0x814] = entry[+0x30]
   │
   ├─> INIT: Chama cloth_payload_init
   │   ├─> lw v0, [descriptor + 0x58]  -- callback init
   │   ├─> beqz v0, skip
   │   ├─> jalr v0
   │   │   ├─> a0 = context
   │   │   └─> a1 = sp (initializer na stack!)
   │   └─> [[context+0x15C] + 0x800] = retorno (payload)
   │
   └─> REGISTER: Registra callback update (opcional)
       ├─> Se entry[+0x24] != NULL:
       │   └─> 0x13F7A8(context, entry[+0x24], 0, a3=0x13)
       ├─> Senao, se descriptor[+0x40] != NULL:
       │   └─> 0x13F7A8(context, descriptor[+0x40], 0, a3=0x13)
       └─> Se descriptor_idx == 1 (BOY): salva context em global
           Se descriptor_idx == 2 (GIRL): salva context em outro global

3. Per-frame update
   └─> Node system (0x13F9D0/0x13FB70) ou chamada direta
       └─> Chama descriptor[+0x50] (handler_b = 0x001D3A30)
           └─> ROPE callback prepara contexto
                └─> Chama dispatcher 0x001D37C8
                    └─> state_id de [payload + 0x48] → jump table 0x00618FB0
                        ├─> State 0 (0x1D3818): GUARD — check 0x1F2148. If pass: setup_1, setup_2, [payload+0x64]=1, state=1
                        ├─> State 1 (0x1D3844): PREPARE/TIMING — seed quaternion from 0x4C4750, timer calc, counter--. If 0: state=2
                        ├─> State 2 (0x1D391C): SIMULATE — matrix init, CD collision (0x12ABE0/0x12AC28), callbacks, result alloc. state=3
                        ├─> State 3 (0x1D39E0): CHECK — 0x12A7F8. If pass: state=4, [payload+0]=1, clear extra. Else: retry
                        └─> State 4 (0x1D3A10): IDLE/DONE — no-op. External code resets state to 0
```

### Funcoes envolvidas

| Funcao | Endereco | Status | Tamanho | Proposito |
|--------|----------|--------|---------|-----------|
| Entry iteration | `0x001B76F8` | Mapeado | ~1024B+ | Itera entry table, chama inits |
| Cloth init (handler_c) | `0x001D27A8` | Decompilado parcial | 528B | Inicializa payload cloth |
| Cloth core init | `0x001B7FE8` | Nao tentado | ??? | Init chain (chamado de 0x1D27A8) |
| Cloth physics setup | `0x001F2240` | Nao tentado | ??? | Physics setup (chamado de 0x1D27A8) |
| Transform setup | `0x0019F4E8` | Nao tentado | ??? | Camera/transform (chamado de 0x1D27A8) |
| Matrix init | `0x00105F00` | Nao tentado | ??? | Matrix init (chamado de 0x1D27A8) |
| Sound/event reg | `0x001AE6F8` | Nao tentado | ??? | Chamado em ambos paths de 0x1D27A8 |
| ROPE callback (handler_b) | `0x001D3A30` | Confirmado runtime | ??? | Update callback |
| Dispatcher | `0x001D37C8` | NEAR-STRUCTURAL | 616B | 5-state FSM (guard→prepare→simulate→check→done) |
| Node callback dispatcher | `0x0013FB70` | Mapeado | ??? | Node system dispatch |
| Callback register | `0x0013F7A8` | Mapeado | ??? | Registra callback no node system |

### Por que 0x001D3A30 nunca aparece nos logs de 0x13F7A8

O callback update `0x001D3A30` esta em `descriptor[+0x50]` do BARREL. O registro via `0x13F7A8` em `0x1B76F8` usa `entry[+0x24]` (override) ou `descriptor[+0x40]`:

- Entries BARREL tem `entry[+0x24] = 0` (sem override)
- BARREL descriptor tem `descriptor[+0x40] = 0x00000000` (sem callback padrao para registro)

Portanto, `0x13F7A8` NUNCA e chamado para entries BARREL. O callback `0x001D3A30` e chamado DIRETAMENTE, provavelmente pelo node system ou por outro dispatcher que le `descriptor[+0x50]`.

### Por que BARREL e nao ROPE

A entry table (dados de sala) usa **BARREL** (indice 0x13) para objetos cloth. O nome **ROPE** (indice 0x14) existe no descriptor table e no physics type table, mas NUNCA e referenciado pela entry table. BARREL e ROPE compartilham os mesmos 3 handlers, entao sao equivalentes em termos de comportamento runtime.

### O que falta

- Identificar o dispatcher que chama descriptor[+0x50] em runtime (provavelmente node system)
- Decompilar o dispatcher (resolver `.word` vs `.dword` da jump table)
- Decompilar os 5 state blocks
- Entender como a entry table e populada para diferentes zonas

---

## Physics Object Type Table — Relacao com Descriptor/Entry Systems

> **Status:** Estrutura mapeada (Rev.049). Tabela e de consulta, nao de inicializacao direta.

### Visao Geral

A physics type table em `0x001A48A0` (.text, stride 0x64, 31 entries) fornece **nomes** e **handlers** para tipos de objeto fisico. Ela e uma tabela de dados, nao um registro de inicializacao.

O sistema de inicializacao real e:

```
Entry Table (0x002A4C48, .data) → Descriptor Table (0x002A31B8, .data)
```

A physics type table contem os mesmos handlers que o descriptor table, mas com indices diferentes e nomes ASCII. A relacao entre os 3 sistemas:

| Sistema | Localizacao | Stride | Funcao |
|---------|-------------|--------|--------|
| Physics Type Table | `0x001A48A0` (.text) | 0x64 | Nomes + handlers por tipo fisico (31 tipos) |
| Descriptor Table | `0x002A31B8` (.data) | 0x64 | Callbacks por label (indice 0x13 = BARREL = cloth) |
| Entry Table | `0x002A4C48` (.data) | 0x4C | Objetos por sala/zona (mecanismo de inicializacao) |

### Formato da Entry

```
+0x00: count       (1)
+0x04: handler_a   (cleanup/post-dispatch, = descriptor +0x48)
+0x08: NULL
+0x0C: handler_b   (update/callback principal, = descriptor +0x50)
+0x10: NULL
+0x14: handler_c   (init/payload, = descriptor +0x58)
+0x18: NULL
+0x1C: NULL
+0x20: name[8]    (ASCII, ex: "ROPE\0\0\0\0")
```

### Mapeamento Physics Type ↔ Descriptor

| Nome | Type Index | Descriptor Label | Descriptor Index | Handlers |
|------|-----------|-----------------|-----------------|----------|
| WOODBOX01 | 0 | NONE? | ??? | ??? |
| ROTOBJEC | 1 | ??? | ??? | 0x1C05D0/0x1C0538/0x1C00C0 |
| BARREL | 2 | NONE | 0x1A | 0x1EA030/0x1E9950/0x1E9F08 |
| ROPE | 3 (tabela .text) | BARREL | 0x13 | 0x1D3B28/0x1D3A30/0x1D27A8 |
| ROPE | 3 (tabela .text) | ROPE | 0x14 | 0x1D3B28/0x1D3A30/0x1D27A8 |

**Importante:** O nome "ROPE" aparece no physics type table como type index 2. Mas a entry table (dados de sala) nunca usa type index 2 ou descriptor ROPE. Ela usa descriptor BARREL (0x13). Os handlers sao identicos.

### Observacoes

- ROPE e ROPEFIX sao duas variantes: ROPE tem funcoes cloth (0x1Dxxxx), ROPEFIX tem funcoes overlay (0x1Exxxx)
- A tabela esta no .text, acessada provavelmente via PC-relative (loader do GCC)

---

## Entity Handler Lifecycle Flow

> **Status:** Mapeado (Rev.053-054). Padrao hC/hB/hA confirmado em 7 entidades.

### Visao Geral

Cada tipo de entidade na descriptor table tem 4 slots de funcao que controlam o ciclo de vida:

```
init_fn (+0x40) → Carregamento de assets (modelos 3D, DMA setup)
hC     (+0x58) → Constructor (alocacao de heap, init de estado)
hB     (+0x50) → Update per-frame (AI, colisao, animacao, desenho)
hA     (+0x48) → Post-init / Reset (cleanup, reset condicional)
```

### Fluxo de Inicializacao

```
1. Sistema de assets detecta nova entidade
   └─> init_fn (se != 0)
       ├─> Carrega modelos 3D via 0x1C8478 ou 0x203B78
       ├─> Configura DMA (0x202208, 0x202148)
       ├─> Configura sprites (0x203AA0)
       └─> [entity+0x15C+0x07C] = 1 (flag active)

2. Entry table iteration (0x1B76F8)
   └─> Para cada entry em zona ativa:
       ├─> Le descriptor_idx de entry[+0x46]
       ├─> Prepara initializer struct na stack (a1 = sp)
       ├─> [initializer+0x30] = entry[+0x30] (variant field)
       └─> Se descriptor[+0x58] != 0:
           └─> jalr descriptor[+0x58] (hC = CONSTRUCTOR)
               ├─> a0 = entity context
               ├─> a1 = sp (initializer struct com variant)
               ├─> 0x13A0F8(alloca) — aloca heap
               ├─> [entity_state + 0x800] = alloc_ptr
               ├─> entity_state_reg(a1, a2, a3, t0, t1)
               ├─> descriptor_setup(entity, 2)
               └─> Retorna alloc_ptr em v0

3. Per-frame update (mecanismo de dispatch runtime)
   └─> Para cada entidade ativa:
       └─> Se descriptor[+0x50] (hB) != 0:
           └─> jalr descriptor[+0x50] (hB = UPDATE)
               ├─> entity_dispatch_update(entity)
               ├─> setup_param(entity, 35, 44, tag)
               ├─> collision_check(entity, ...)
               ├─> Animacao, desenho, state machine
               └─> (0x1D3A30 = BARREL hB nunca disparou em runtime)

4. Reset/Post-init (condicional)
   └─> Se descriptor[+0x48] (hA) != 0:
       └─> jalr descriptor[+0x48] (hA = RESET)
           ├─> state_resolver()
           ├─> state_update()
           └─> Cleanup de recursos

5. Despawn
   └─> Sistema de cleanup (desconhecido)
       ├─> model_free() para cada modelo
       └─> group_free() / cleanup chain
```

### Tabela de Handlers por Entidade (Rev.056)

| Indice | Nome | init_fn | hC (constructor) | hB (update) | hA (reset) | Status |
|--------|------|---------|-------------------|-------------|------------|--------|
| 1 | BOY | 0x153478 | 0x1C1A98 | 0x1C1DD8 (51 insns) | 0x1C1F58 (80 insns) | ASM |
| 2 | GIRL | 0x174BA0 | 0x1D1668 (92 insns, 64B) | 0x1D17F8 (112 insns, anim blend) | 0x1D1A98 (16 insns) | ASM |
| 4 | ENEMY1 | 0x164440 | 0x1CE220 (~103 insns, 80B) | 0x1CE3C0 (~135 insns, AI+draw) | 0x1CE690 (~23 insns) | ASM |
| 17 | WOODBOX0 | 0x17D1D0 | 0x1C00C0 (286 insns, 400B) | 0x1C0538 (27 insns, counter) | 0x1C05D0 (28 insns) | ASM |
| 19 | BARREL | 0 | 0x1D27A8 (cloth_payload_init) | 0x1D3A30 (cloth update) | 0x1D3B28 | CLOTH |
| 30 | BGA | 0x203EE8 | 0 | 0 | 0 | ASM |
| 32 | BIRD | 0x1971C0 | 0x197240 (60 insns, 64B, random timer) | 0x197078 (2 insns, tail to 0x1E3FC8) | 0x197080 (24 insns) | NEAR |
| 46 | QUEEN | 0x19B7F8 | 0x19A7E8 (68 insns, 24B) | 0x19A8F0 (44 insns, LOD scaling) | 0x19A9A0 (32 insns) | NEAR |
| 48 | DEVIL_GI | 0x174BA0 | 0x1D1668 (= GIRL) | 0x1D17F8 (= GIRL) | 0x1D1A98 (= GIRL) | ALIAS |
| 61 | AP1 | 0x1BB6B0 | 0x1B8720 (400+ insns, 640B) | 0x1BA330 (~200 insns, 7-state FSM) | 0x1BA530 (40 insns) | ASM |
| 62 | ATTACKCH | 0x1BBF78 | 0x1BBE50 (20 insns, 12B alloc) | 0x1BBEA0 (12 insns, guard) | 0x1BBEC8 (44 insns, spawner) | NEAR |
| 63 | ATTACKCH | 0x1BBF78 | 0x1BBB20 (176 insns, dyn alloc, rotation math) | 0x1BBDD8 (28 insns, linked-list detach) | 0x1BBE48 (2 insns, NO-OP) | ASM |
| 64 | BOSS_CTR | 0x198140 | 0x198218 (60 insns, 53-slot loop) | 0x197FC8 (4 insns, conditional tail) | 0x198000 (68 insns, slot cleanup) | ASM |

### hB dispatcher diversity

Nao ha um dispatcher unificado para hB (+0x50). Cada entidade tem codigo hB independente,
chamado atraves do mesmo mecanismo de dispatcher do descritor. Padroes observados:

| Pattern | Count | Entities |
|---------|-------|----------|
| Full code | 7 | BOY, GIRL/DEVIL_GI, ENEMY1, WOODBOX0, QUEEN, AP1 |
| Delegate (tail to 0x1E3FC8) | 1 | BIRD |
| Conditional tail | 1 | BOSS_CTR (0x17DD60 or 0x17DCD8) |
| Linked-list management | 1 | ATTACKCH idx 63 |
| Small guard (12 insns) | 1 | ATTACKCH idx 62 |
| Cloth physics | 1 | BARREL |

A busca por um dispatcher centralizado (Rev.055) falhou porque o dispatcher nao existe —
o runtime le `descriptor[+0x50]` e chama `jalr` diretamente para cada entidade.

### Cloth Force Application (0x1D3F78)

```
clothSubForceApply(ctx, force_h, force_v):
  1. alloc_entity(19) → s1        // 64B heap alloc
  2. if null: return 1
  3. Loop: for each entity in chain:
     ├─ rand() * 65536.0f → s16 angle
     ├─ Check extra/count: mark for update if inactive
     ├─ [es+0x130] = force_h * sin(angle)   // X-force
     ├─ [es+0x134] = force_v * rand()        // Y-force
     ├─ [es+0x138] = force_h * cos(angle)    // Z-force
     └─ s1 = alloc_next(s1)                  // next chain entity
  4. return 1
```

**EE-only (zero COP2):** usa sin/cos do FPU do EE via 0x008BAC0/0x008BB20

### Observacoes

- **hC sempre aloca via 0x13A0F8**: todas as entidades com hC usam heap_alloc com tag unica (exceto BOSS_CTR que tem alloc=0)
- **entity_state_reg**: todas chamam com (a1, a2, a3, t0, t1) — parametros especificos por tipo
- **descriptor_setup(entity, 2)**: padrao universal (exceto BARREL que tem init_fn=0)
- **BGA e o caso atipico**: 2D sprite overlay, init_fn=12 insns, sem handlers
- **AP1 e o mais complexo**: 640B de estado, 4 child slots, state machine de 7 estados, frame offset randomization
- **DEVIL_GI = GIRL alias**: todos os 4 handlers identicos. Provavelmente paleta alternativa ou versao de cutscene.
- **Nenhum handler usa COP2**: COP2 exclusivo do sistema cloth BARREL (79 COP2 total em 2 funcoes)

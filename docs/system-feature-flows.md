# System & Feature Flows — ICO Reconstruction

> Documento vivo. Atualizado sempre que uma feature for criada ou modificada.
> **Ultima atualizacao:** 2026-05-16 (Rev.050 — entry table iteration flow mapeado, descriptor chain completo)

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
                       ├─> State 0: 0x001D3818
                       ├─> State 1: 0x001D3844
                       ├─> State 2: 0x001D391C
                       ├─> State 3: 0x001D39E0
                       └─> State 4: 0x001D3A10
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
| Dispatcher | `0x001D37C8` | BLOCKED/ASM-HOLD | 616B | 5-state dispatch |
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

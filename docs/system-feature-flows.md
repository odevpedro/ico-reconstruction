# System & Feature Flows — ICO Reconstruction

> Documento vivo. Atualizado sempre que uma feature for criada ou modificada.
> **Ultima atualizacao:** 2026-05-16

---

## Indice

- [Cloth Physics Dispatch Flow](#cloth-physics-dispatch-flow)
- [Physics Object Type Table Initialization](#physics-object-type-table-initialization)

---

## Cloth Physics Dispatch Flow

> **Status:** Fluxo principal mapeado. Funcao iteradora da tabela de tipos nao identificada.

### Visao Geral

O sistema cloth do ICO usa um dispatcher de 5 estados para simular objetos do tipo "ROPE". Os handlers sao armazenados estaticamente numa tabela de tipos de objeto fisico (31 tipos, stride 0x64).

### Fluxo

```
1. Entity initialization
   └─> Funcao iteradora (nao identificada) le entry ROPE da tabela 0x1A48A0
       └─> Chama handler_c (0x001D27A8 = cloth_payload_init)
           └─> Inicializa cloth_payload na stack do caller
               ├─> Preenche dados de pose (posicao X/Y/Z)
               ├─> [a1+0x30] = variant_id (0 ou 1 por area)
               └─> [a1+0x58] = 0 (callback slot vazio)

2. Per-frame update (loop do jogo)
   └─> Sistema chama handler_b (0x001D3A30 = ROPE callback)
       └─> ROPE callback prepara contexto
           └─> Chama dispatcher 0x001D37C8
               └─> Le candidate_state_id de [payload + 0x48]
               ├─> Bounds check: state_id < 5
               ├─> state_id * 4 → jump table 0x00618FB0
               └─> Salta para um dos 5 state blocks:
                   ├─> State 0: 0x001D3818
                   ├─> State 1: 0x001D3844
                   ├─> State 2: 0x001D391C
                   ├─> State 3: 0x001D39E0
                   └─> State 4: 0x001D3A10

3. State block execution (cloth vertex simulation phases)
   └─> Cada state block executa uma fase da simulacao:
       ├─> Ex.: idle, wind, collision, constraint solve, post-process
       └─> State_id pode mudar apos execucao (nao confirmado)

4. Retorno
   └─> Dispatcher retorna ao ROPE callback
   └─> ROPE callback retorna a funcao iteradora
   └─> Funcao iteradora prossegue com o proximo objeto
```

### Funcoes envolvidas

| Funcao | Endereco | Status | Tamanho |
|--------|----------|--------|---------|
| ROPE callback (handler_b) | `0x001D3A30` | Confirmado runtime (Rev.025) | ??? |
| Dispatcher | `0x001D37C8` | BLOCKED / ASM-HOLD (jump table) | 616B |
| cloth_payload_init (handler_c) | `0x001D27A8` | Runtime-dependent (Rev.046) | 528B |
| State 0 | `0x001D3818` | Nao tentado | ??? |
| State 1 | `0x001D3844` | Nao tentado | ??? |
| State 2 | `0x001D391C` | Nao tentado | ??? |
| State 3 | `0x001D39E0` | Nao tentado | ??? |
| State 4 | `0x001D3A10` | Nao tentado | ??? |

### O que falta

- Identificar a funcao iteradora da tabela de tipos (proximo probe PCSX2)
- Decompilar o dispatcher (resolver `.word` vs `.dword` da jump table)
- Decompilar os 5 state blocks
- Decompilar cloth_payload_init (depende de runtime)

---

## Physics Object Type Table Initialization

> **Status:** Estrutura mapeada (Rev.049). Funcao de leitura nao identificada.

### Visao Geral

Tabela estatica em `0x001A48A0` dentro da secao `.text` (stride 0x64, 31 entries). Cada entry define 3 handlers para um tipo de objeto fisico.

### Formato da Entry

```
+0x00: count       (1)
+0x04: handler_a   (cleanup/post-dispatch)
+0x08: NULL
+0x0C: handler_b   (update/callback principal)
+0x10: NULL
+0x14: handler_c   (init/payload)
+0x18: NULL
+0x1C: NULL
+0x20: name[8]    (ASCII, ex: "ROPE\0\0\0\0")
```

### Tipos conhecidos

| Nome | handler_a | handler_b | handler_c |
|------|-----------|-----------|-----------|
| WOODBOX01 | ? | ? | ? |
| ROTOBJEC | 0x1C05D0 | 0x1C0538 | 0x1C00C0 |
| BARREL | 0x1EA030 | 0x1E9950 | 0x1E9F08 |
| **ROPE** | **0x1D3B28** | **0x1D3A30** | **0x1D27A8** |
| CHAIN | 0x1E9630 | 0x1E9810 | 0x1E8F38 |
| FLEVER | 0x18F640 | 0x18ECC8 | 0x18E5B0 |
| WLEVER | 0x1BC438 | 0x1BC130 | 0x1C09C8 |
| ROPEFIX | 0x1E6960 | 0x1E6788 | 0x1E6968 |
| QUEEN | 0x1D01E8 | 0x1D00F8 | 0x1CFB58 |
| CANDLE | 0x193600 | 0x1930B0 | 0x193730 |

### Observacoes

- ROPE e ROPEFIX sao duas variantes: ROPE tem funcoes cloth (0x1Dxxxx), ROPEFIX tem funcoes overlay (0x1Exxxx)
- ROPE e o unico tipo com handlers no cluster cloth-domain (0x1Dxxxx)
- A tabela esta no .text, acessada provavelmente via PC-relative (loader do GCC)

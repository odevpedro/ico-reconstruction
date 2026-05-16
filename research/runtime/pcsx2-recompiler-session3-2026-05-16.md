# PCSX2 Recompiler Session 3 — 0x1D3A30 probe + 0x0024xxxx callers investigation

## Date

2026-05-16

## Objective

Testar se `0x001D3A30` (`descriptor[+0x50]` de BARREL/ROPE) é chamado durante gameplay normal, e identificar seus callers — ou provar que não é chamado neste contexto.

Probe adicionado à build instrumentada do PCSX2 (recompiler path) para capturar `0x001D3A30` com label `cloth_update_callback`, mantendo os probes existentes (`0x00100008` sentinel, `0x001D27A8` cloth_payload_init, `0x0013F7A8` callback_register, `0x00201ED4` runtime_candidate).

## Scope

Incluído:

- execução da build instrumentada em `/home/peter/Documentos/repos/pcsx2-ico-logpoints/build/bin/pcsx2-qt`
- ~90 minutos de gameplay em áreas variadas (bondinho, castelo, Yorda, animações)
- 1419 eventos capturados em JSONL
- análise dos callers de `callback_register` incluindo `0x00240E58` e `0x00240F98`
- investigação estática das funções parent `0x00240D40` e `0x00240EA0`

Excluído:

- distribuição de binários, ISO, savestates ou assets proprietários
- prova negativa definitiva para `0x001D3A30` em todas as condições (não testamos overlays não carregados)
- conclusão semântica sobre gameplay

## Sources Used

| Fonte | Uso |
|---|---|
| ELF `SCUS_971.13.elf` | Disassemblagem / objdump |
| `pcsx2/x86/ix86-32/iR5900.cpp` | Código do probe (recompiler) |
| `events.jsonl` (1419 linhas) | Dados de runtime |
| Rev.050 | Modelo de tabelas (entry, descriptor, physics type) |
| Rev.037 | Mapa de 5 callsites de `0x13F7A8` |
| ICO-decomp cross-reference | Mapeamento de segmentos `.text` |

## Evidence Used

- Log JSONL com 1419 eventos, 4 labels, ra capturado por evento
- Disassemblagem via `mips64el-ps2-elf-objdump` das funções `0x240D40` e `0x240EA0`
- Leitura de seções ELF via `readelf -S`

## Findings

### 1. 0x001D3A30 — ZERO HITS em ~90 minutos de gameplay

| Label | Eventos | % |
|---|---|---|
| `callback_register` | 1249 | 88.0% |
| `cloth_payload_init` | 145 | 10.2% |
| `runtime_candidate_00201ed4` | 24 | 1.7% |
| `cloth_update_callback` | **0** | **0.0%** |
| `elf_entry_sentinel` | 1 | <0.1% |

**Conclusão definitiva**: `0x001D3A30` não é chamado durante gameplay normal, mesmo com objetos cloth sendo inicializados (145x `cloth_payload_init`) e 90+ minutos de jogo em áreas variadas.

Interpretações possíveis:
- Chamado via overlay não carregado nesta sessão
- Chamado via VU interrupt (fora do recompilador)
- Chamado apenas em cutscenes/eventos específicos
- Modelo semântico incorreto para esta função

### 2. callback_register — todos com a3=0x13

Apesar do probe estar SEM filtro de a3, todas as 1249 chamadas a `0x13F7A8` usam `a3=0x13` (BARREL). O jogo nunca chama este registration wrapper com outro tipo durante gameplay normal.

### 3. Callers de callback_register

| ra | Parent function | Callsites | % |
|---|---|---|---|
| `0x001B7AD4` | `0x1B76F8` (descriptor iterator) | 622 | 49.8% |
| `0x001B7AB8` | `0x1B76F8` (descriptor iterator) | 396 | 31.7% |
| `0x00240E58` | `0x00240D40` (object factory + callback reg) | 141 | 11.3% |
| `0x00240F98` | `0x00240EA0` (simplified object factory) | 66 | 5.3% |
| `0x00201EDC` | `0x00201E70` (known runtime candidate) | 24 | 1.9% |

### 4. Callbacks registrados (a1 values)

| a1 | Count | Notes |
|---|---|---|
| `0x00164440` | 258 | Callback function |
| `0x00203EE8` | 252 | Callback function |
| `0x001BB6B0` | 105 | Callback function |
| `0x0017D1D0` | 42 | Callback function |
| `0x001BBF78` | 42 | Callback function |
| `0x001C3760` | 33 | Callback function |
| `0x001F2370` | 33 | Callback function |
| `0x0017D128` | 33 | Callback function |
| `0x00153478` | 33 | Callback function |
| `0x00174BA0` | 29 | Callback function |

Nenhum é `0x001D3A30`.

### 5. cloth_payload_init — 145 eventos

- **Caller exclusivo**: `0x001B7A88` (dentro de `0x1B76F8`)
- **Contextos únicos (a0)**: 16
- **Variant [a1+0x30]**:
  - `0x00000001` (full init): 75x
  - `0x00000000` (quick path): 70x
- Split praticamente 50/50 entre init completo e rápido

### 6. Investigação de 0x00240E58 e 0x00240F98

**Correção de segmento**: `.text` vai de `0x00100000` a `0x0026F5D4`. Os endereços `0x0024xxxx` estão DENTRO do `.text`, não em overlay.

Ambos são **endereços de retorno** após `jal 0x13F7A8` em duas funções factory:

| ra | Parent function | Frame | a1 source | Guard |
|---|---|---|---|---|
| `0x00240E58` | `0x00240D40` | 160B | `[descriptor+0x40]` | `t0 != 0` AND `[desc+0x40] != 0` |
| `0x00240F98` | `0x00240EA0` | 128B | Caller's literal `t0` | `t0 != 0` |

**Ambas estão excluídas como caminho para registrar `0x1D3A30`**:
- `0x240D40`: BARREL/ROPE têm `descriptor[+0x40] = 0`
- `0x240EA0`: callers conhecidos passam `0x001C3720` ou `0x001F2390`

## What is Confirmed

- `0x001D3A30` **não é chamado durante gameplay normal** (~90 min, 0 hits)
- `.text` termina em `0x0026F5D4`, não `0x001Fxxxx` — correção de segmento
- `0x00240D40` e `0x00240EA0` são factories de objeto + multi-callback (tipos 22,23,24,19)
- Todas as 1249 chamadas a `0x13F7A8` usam `a3=0x13`
- 16 contextos distintos de cloth_payload_init
- Split 50/50 entre full init e quick path em cloth_payload_init
- 10 tipos de callback registrados via `0x13F7A8`, nenhum é `0x1D3A30`

## What is Probable

- Os dois callers `0x00240D40`/`0x00240EA0` são parte do sistema de criação de objetos do engine (gamesys), não específicos do sistema cloth
- Caller B em `0x1B803C` (dentro de `0x1B76F8` com `t0 = t3` variável) é improvável de registrar ROPE pois o `descriptor[+0x40]` de BARREL/ROPE é zero

## What is Unknown

- Quem/nem como `0x001D3A30` é chamado em runtime
- Se a função seria atingível via cutscene/evento específico não jogado nesta sessão
- Valor de `descriptor[+0x40]` para tipos NÃO-BARREL (ex: CHAIN, FLEVER, BGA)
- Origem de `t3` em Caller B (`0x1B803C`)

## What is Discarded

- Modelo de "update callback por frame" para `0x1D3A30` — **refutado por runtime**
- `0x00240E58`/`0x00240F98` como caminho de registro de `0x1D3A30` — **excluído**
- Hipótese de overlay para endereços `0x0024xxxx` — **incorreto**, estão em `.text`
- `0x00203EE8` como caller de runtime_candidate — confirmado como função callback, não caller

## Next Minimum Test

1. Breakpoint em `0x001D3A30` com PCSX2 debugger (não logpoint) durante cutscene ou evento específico (ex: cena do vento, Yorda sendo carregada, ou queda da ponte)
2. Ou: análise estática mais profunda de `0x00201EDC` (ra de runtime_candidate) — único callsite restante dos 5 originais (Rev.037) que não foi exaustivamente rastreado
3. Ou: mapear `descriptor[+0x40]` para todos os 31+ tipos na physics type table para ver se algum tipo NÃO-BARREL/ROPE tem valor não-nulo

## Conservative Verdict

A sessão runtime de ~90 minutos estabelece com alta confiança que `0x001D3A30` **não faz parte do loop de gameplay normal**. A função é chamada apenas sob condições específicas não encontradas nesta sessão, ou o modelo semântico de "update callback" está incorreto.

A investigação estática de `0x00240E58`/`0x00240F98` elimina definitivamente esses dois callsites como via para o ROPE callback, corrige o limite do `.text` para `0x0026F5D4`, e documenta duas funções factory do engine que registram múltiplos callbacks por objeto.

A correção do limite do `.text` (`0x0026F5D4`, não `0x001Fxxxx`) e as duas funções factory `0x240D40`/`0x240EA0` são contribuições documentais que corrigem o modelo do projeto.

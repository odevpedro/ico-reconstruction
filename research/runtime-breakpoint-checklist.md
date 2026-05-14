# Runtime Breakpoint Checklist

## Preparado para PCSX2 Debugger (v2.6.3+)

Objetivo: capturar evidência runtime para resolver o gap de registro do callback `0x001d3a30` em `node + 0x1c`.

### Setup

- BIN: qualquer ISO funcional (original ou mod)
- PCSX2: flatpak run net.pcsx2.PCSX2 -debugger -- "/path/to/game.bin"
- Symbols: carregar `.local/pcsx2-symbols.sym` (File > Load Symbols no debugger)
- Debugger: layout R5900 (EE), guia Breakpoints

---

## Breakpoint 1 — Registration capture

**Endereço**: `0x0013f7a8` (`callback_registration_wrapper`)

**Condição**: break on execute; se PCSX2 suportar condição, usar `a3 == 0x13`

**Campos para capturar**:

| Registrador | O que representa | Anotar |
|---|---|---|
| `a0` | handle/objeto alvo | endereço |
| `a1` | **callback sendo registrado** | **comparar com 0x001d3a30** |
| `a2` | byte flag | valor |
| `a3` | tipo de registro | deve ser 0x13 |
| `ra` | **endereço de retorno** | **mapear para callsite** |
| `t0` | argumento auxiliar | valor |

**Perguntas**:
- `a1 == 0x001d3a30`? Se sim, gap resolvido.
- `ra` cai em qual dos 5 callsites conhecidos? (0x1b7ab0, 0x1b7acc, 0x201ed4, 0x240e50, 0x240f90)
- Quão frequente é esse breakpoint? (a cada frame? uma vez só?)

---

## Breakpoint 2 — Cloth dispatcher state distribution

**Endereço**: `0x001d37c8` (`cloth_domain_dispatcher_candidate`)

**Condição**: break on execute (sem condição inicialmente)

**Campos para capturar**:

| Expressão | O que representa | Anotar |
|---|---|---|
| `a0` | context pointer | endereço |
| `[a0 + 0x15c]` | entity pointer | endereço |
| `[[a0 + 0x15c] + 0x800]` | state block pointer | endereço |
| `[[[a0 + 0x15c] + 0x800] + 0x48]` | **state ID** | **valor (0-4)** |

**Perguntas**:
- Quais state IDs aparecem durante gameplay?
- O state ID muda ou fica fixo?
- O dispatcher é chamado a cada frame?
- Qual state ID aparece durante cenas específicas (menu, morte, cutscene)?

---

## Breakpoint 3 — Jump table load

**Endereço**: `0x001d380c` (`cloth_dispatcher_jumptable` load)

**Condição**: break on execute

**Campos para capturar**:

| Registrador | O que representa | Anotar |
|---|---|---|
| `$v1` | endereço na jump table | deve estar próximo de 0x00618fb0 |
| `$a0` **após** `lw a0,($v1)` | **handler alvo** | **qual state block será executado** |

**Perguntas**:
- O handler carregado corresponde ao state ID esperado?
- Algum state ID leva a um handler inesperado?

---

## Breakpoint 4 — Cloth update callback entry

**Endereço**: `0x001d3a30` (`cloth_domain_update_callback_candidate`)

**Condição**: break on execute

**Campos para capturar**:

| Registrador | O que representa | Anotar |
|---|---|---|
| `a0` | context pointer | endereço |
| `ra` | **quem chamou este callback** | **mapear caller** |

**Perguntas**:
- Quem chama `0x001d3a30`? (`ra` revela o caller real)
- O caller é `0x0013fb70` (node_callback_dispatcher)? Se sim, confirma a cadeia.
- O callback é chamado a cada frame?

**Captura adicional (Rev.041 — variant table)**:
| Expressão | O que representa | Anotar |
|---|---|---|
| `state_block + 0x04` | **variant/mode index** | valor (0-7) |
| `0x004d4188 + ([state_block+0x04] * 0x14)` | **table entry** | 20 bytes |
| `[table_entry + 0x00]` | entry field 0 | word |
| `[table_entry + 0x04]` | entry field 4 | word |
| `[table_entry + 0x08]` | entry field 8 | word (possível callback) |
| `[table_entry + 0x0c]` | entry field c | word |
| `[table_entry + 0x10]` | entry field 10 | word |

---

## Breakpoint 5 — Node callback dispatcher

**Endereço**: `0x0013fb70` (`node_callback_dispatcher`)

**Condição**: break on execute

**Campos para capturar**:

| Registrador | O que representa | Anotar |
|---|---|---|
| `a0` | objeto/contexto | endereço |
| `v0` (após load de node+0x1c) | **callback sendo chamado** | **deve ser 0x001d3a30** |

**Perguntas**:
- O callback carregado de `node+0x1c` é `0x001d3a30`?
- Quantos callbacks diferentes passam por este dispatcher?

---

## Breakpoint 6 — Variant field writer (confirmation)

**Endereço**: `0x001d2858`

**Condição**: break on execute

**Campos para capturar**:

| Registrador | O que representa | Anotar |
|---|---|---|
| `a0` | initializer_arg (payload source) | endereço |
| `[a0 + 0x30]` | **variant value being copied** | **0-7** |
| `[payload + 0x04]` destino | escrita realizada | deve ser igual a [a0+0x30] |

**Perguntas**:
- Qual é o valor de `[a0+0x30]`? É sempre 0-7?
- O valor muda entre chamadas?
- O valor corresponde aos índices da tabela 0x004d4188?

---

## Breakpoint 7 — Variant field zeroing (candidate setter)

**Endereço**: `0x001d1b18` (store inside `0x001d1ad8`)

**Condição**: break on write (memory), ou break on execute se mem write não for suportado

**Campos para capturar**:

| Registrador | O que representa | Anotar |
|---|---|---|
| `ra` | **quem chamou o setter** | **pode ser fumi/src/way_llf?** |
| `a0` | target payload | endereço |

**Perguntas**:
- Quem chama `0x001d1ad8`? (`ra` revela)
- É chamado com o mesmo payload do cloth animation?
- Qual o contexto de gameplay (fase, transição, morte)?

---

## Breakpoint 8 — Dispatcher +0x48 (slot dispatch, no a1)

**Endereço**: `0x0013fc44` (inside 0x0013fc00, onde callback +0x48 é carregado)

**Condição**: break on execute

**Campos para capturar**:

| Registrador | O que representa | Anotar |
|---|---|---|
| `a0` | **único argumento preparado** | endereço do objeto |
| `a1` | **NÃO preparado** | lixo/zero? |
| `callback` (v0 após load) | função sendo chamada | é 0x001d27a8? |

**Pergunta**: quem chama +0x48 sem preparar a1? Isso confirma que a1 chega de outra forma.

---

## Breakpoint 9 — Dispatcher +0x48 alternative dispatch point

**Endereço**: `0x0013fcb8` (ponto alternativo de dispatch de +0x48)

**Condição**: break on execute

**Campos para capturar**: mesmos do Breakpoint 8.

---

## Logging pattern

Para cada breakpoint, registrar:

```csv
timestamp,breakpoint_addr,breakpoint_name,a0,a1,a2,a3,ra,state_id,callback
```

Exemplo:
```
00:01:23,0x0013f7a8,registration_wrapper,0x1a3f00,0x001d3a30,0x01,0x13,0x001b7ab0,-
00:01:23,0x001d37c8,cloth_dispatcher,0x1a3f00,-,-,-,-,2
```

---

## Próximo passo após captura

Com os dados coletados:

1. Se `a1 == 0x001d3a30` no breakpoint 1 e `ra` aponta para `0x001b7ab0` → o registro é via `entry[+0x24]` em `0x001b76f8`. Confirmar programaticamente: examinar `entry[+0x46]` e `entry[+0x24]` em runtime.
2. Se `ra` aponta para `0x00201ed4` → o registro é via `0x00201e70`, candidato runtime-dependente.
3. Se `a1 != 0x001d3a30` → o gap permanece. Investigar se o callback já está registrado antes do primeiro frame.

---

## Referências

- Rev.037: mapeamento dos 5 callsites de 0x0013f7a8
- Rev.039: correção de domínio cloth
- Rev.040: reinterpretação estática do cluster cloth
- Rev.041: variant table 0x004d4188 (8 entries stride 0x14, indexada por state_block+0x04)
- Rev.042: variant field writer 0x001d2858, candidate setter 0x001d1ad8
- Rev.043: initializer arg source — 0x001d27a8 needs a1, [a1+0x30] origin open
- `.local/pcsx2-symbols.sym`: símbolos para o debugger

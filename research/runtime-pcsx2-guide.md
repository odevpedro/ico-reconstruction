# PCSX2 Runtime Capture Guide

## Date

2026-05-15

## Objetivo

Capturar em runtime a origem do argumento `a1` passado para `0x001d27a8`
e confirmar o fluxo de registro do callback `0x001d3a30`.

## Pré-requisitos

- PCSX2 com debugger habilitado
- `.sym` file em `splat/SCUS_971.13.sym`
- ISO do ICO USA (`SCUS_971.13`)
- Conhecimento básico do debugger do PCSX2

## Setup inicial

### 1. Carregar símbolos

No PCSX2 debugger, carregar:

```
splat/SCUS_971.13.sym
```

Isso nomeia 191 funções do kernel PS2 (CreateThread, SignalSema,
FlushCache, sceSifSetDma, etc.), facilitando a navegação.

### 2. Breakpoints prioritários

```
Breakpoint 1: 0x001d27a8  (cloth_payload_init) — PRIORIDADE MÁXIMA
Breakpoint 2: 0x0013f7a8  (callback_register) — confirmação
Breakpoint 3: 0x001d37c8  (cloth_dispatcher) — opcional
```

## Breakpoint 1 — 0x001d27a8 (PRIORIDADE MÁXIMA)

### O que a função faz

```asm
0x001d27a8: daddu s5, a0, zero     ; s5 = a0 (context/entity)
0x001d27cc: daddu s4, a1, zero     ; s4 = a1 (initializer_arg ← QUEREMOS SABER)
0x001d27e8: jal func_0013A0F8       ; request payload
0x001d2804: sw s6, 0x800(v1)       ; install payload at [entity + 0x800]
0x001d2850: lw v1, 0x30(s4)        ; READ [a1 + 0x30]
0x001d2858: sw v1, 0x4(s6)         ; WRITE [payload + 0x04]
```

### O que capturar quando o breakpoint disparar

| Registrador | O que é | Por que capturar |
|---|---|---|
| `a0` | Context/entity pointer | Identificar qual entidade está sendo inicializada |
| `a1` | **Initializer arg (ALVO)** | **Saber de onde vem este ponteiro** |
| `ra` | Return address | Quem chamou esta função |
| `[a1+0x30]` | Valor copiado para variant | Qual variante está sendo configurada |
| `a1+0x00..0x60` | Dump do início do descriptor | Entender estrutura do descriptor |

### Procedimento

1. Ativar breakpoint em `0x001d27a8`
2. Iniciar o jogo (qualquer fase — o breakpoint deve disparar durante gameplay/loading)
3. Quando disparar, capturar:
   - `printf "a0=0x%08X a1=0x%08X ra=0x%08X a1_30=0x%08X\n", a0, a1, ra, [a1+0x30]`
   - Dump de 0x40 bytes de `a1`
4. Deixar o jogo continuar e verificar se o breakpoint dispara novamente
5. Repetir 3-5 vezes para ver se `a1` muda

### O que esperar

Hipóteses para a origem de `a1`:

| Hipótese | Probabilidade | Evidência |
|---|---|---|
| Aponta para `.data` (ROPE descriptor) | Média | ROPE em +0x48, estrutura conhecida |
| Aponta para stack (variável local) | Média | Rev.044 mostrou padrão similar em 0x29660 |
| Aponta para heap (alloc) | Média | Payload é allocado via func_0013A0F8 |
| Aponta para struct global | Baixa | Sem evidência |
| `a1 = a0 + offset` (mesmo objeto) | Média | Possível se o initializer_arg parte da entidade |

## Breakpoint 2 — 0x0013f7a8 (callback_register)

### Quando usar

Se o Breakpoint 1 não der resultado claro, ou para confirmar o fluxo
de registro do callback `0x001d3a30`.

### O que capturar

| Registrador | O que é |
|---|---|
| `a0` | Handle/context |
| `a1` | **Callback pointer** (deve ser 0x001d3a30 para ROPE) |
| `a2` | Slot index (0xFF para ROPE? Ver Rev.037) |
| `a3` | Record/descriptor ID (0x13 para ROPE) |

### Procedimento

1. Ativar breakpoint em `0x0013f7a8` com condição `a3 == 0x13`
   (só dispara para o descritor ROPE)
2. Quando disparar, verificar:
   - `a1 == 0x001d3a30`? (confirma que ROPE registra o callback cloth)
   - Se não, qual callback está sendo registrado?
   - `ra` — quem chamou o registro?
3. Subir a pilha (`ra` - 8) para ver o callsite real

## Breakpoint 3 — 0x001d37c8 (cloth_dispatcher) — Opcional

### Quando usar

Depois dos breakpoints 1 e 2, para observar o comportamento em runtime
do dispatcher.

### O que capturar

| Registrador | O que é |
|---|---|
| `v1` (após `lw v1, 8(s1)`) | **state_id** (0-4) |
| `s2` | Context/entity |
| `s3` | Payload pointer |

### Perguntas

- Qual state_id aparece mais? (0 = init, 1 = wind, etc.)
- A distribuição muda durante o jogo?
- O dispatcher é chamado uma vez por frame ou em resposta a eventos?

## Roteiro passo-a-passo

### Fase 1 — Breakpoint em 0x001d27a8 (30 min)

```
1. Carregar .sym file
2. Set breakpoint em 0x001d27a8
3. Iniciar jogo (New Game ou Continue)
4. Breakpoint dispara → capturar a0, a1, ra, [a1+0x30]
5. Salvar dump de 64 bytes de a1
6. Continuar jogo
7. Repetir 4-6 até breakpoint parar de disparar ou dados suficientes
8. Se não disparar durante gameplay normal, testar em tela de título
```

### Fase 2 — Breakpoint em 0x0013f7a8 (15 min)

```
1. Se Fase 1 não resolveu, set breakpoint em 0x0013f7a8 cond: a3==0x13
2. Reiniciar jogo
3. Breakpoint dispara → capturar a0, a1, a2, a3, ra
4. Verificar se a1 == 0x001d3a30
5. Anotar ra para identificar callsite
```

### Fase 3 — Análise dos dados (offline)

```
1. Com a1 conhecido da Fase 1:
   - a1 aponta para .data? Verificar faixa de endereço (~0x002xxxxx)
   - a1 aponta para heap? Faixa ~0x00xxxxxx
   - a1 aponta para stack? Próximo de $sp
   - a1 == a0 + offset? Calcular diferença
2. Com [a1+0x30] conhecido:
   - Qual variante está sendo configurada?
   - Tabela 0x004d41888 (8 entries) indexada por este valor
```

## Checklist

- [ ] PCSX2 instalado e configurado
- [ ] ISO do ICO USA disponível
- [ ] `.sym` file carregado (`splat/SCUS_971.13.sym`)
- [ ] Breakpoints configurados
- [ ] Log format definido para captura
- [ ] Terminal/dump preparado para salvar outputs

## Referências

- `research/runtime-capture-automation-plan.md` (plano detalhado, 475 linhas)
- `splat/SCUS_971.13.sym` (191 símbolos do kernel PS2)
- `research/elf/ghidra-rev045-runtime-plan-for-tomorrow.md`
- `research/external/first-scratch-func-001d3d70-results.md`

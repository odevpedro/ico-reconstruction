# rev.024 — Internal State Block Semantics

## Resumo executivo

Esta revisão analisa os cinco basic blocks internos alcançados pelo dispatcher confirmado em `0x001d37c8`.

O resultado conservador é:

| Estado | Alvo | Leitura curta |
|---:|---|---|
| 0 | `0x001d3818` | bloco de entrada/default que chama uma rotina de teste/consulta e, se ela retorna não zero, prepara o estado 1 |
| 1 | `0x001d3844` | bloco temporizado/de atualização numérica; decrementa `[candidate_state_block_ptr + 0x44]` e troca para o estado 2 ao chegar a zero |
| 2 | `0x001d391c` | bloco de inicialização/configuração mais pesado; cria/configura recursos e troca para o estado 3 |
| 3 | `0x001d39e0` | bloco de espera/finalização; consulta um recurso em `[candidate_state_block_ptr + 0x60]` e troca para o estado 4 quando a consulta retorna não zero |
| 4 | `0x001d3a10` | epílogo comum; não faz trabalho próprio além de restaurar registradores e retornar |

Não há evidência direta suficiente para atribuir nomes de gameplay. A classificação acima é semântica estrutural, baseada em instruções, chamadas, stores e transições internas.

## Escopo

Escopo incluído:

- dispatcher `0x001d37c8`;
- jump table `0x00618fb0`;
- blocos internos `0x001d3818`, `0x001d3844`, `0x001d391c`, `0x001d39e0`, `0x001d3a10`;
- bytes e instruções no intervalo `0x001d37c8..0x001d3a2c`;
- leituras, stores, chamadas e transições observáveis nesse intervalo.

Escopo excluído:

- `DATA.DF`;
- `.gcm`;
- strings de Yorda, capture, shadow, Continue menu ou TM2;
- overlays;
- nomes de gameplay sem evidência direta;
- revisões antigas além do uso como contexto.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `AGENTS.md` | regras de escopo, cautela e nomenclatura |
| `codex_task_rev024.md` | objetivo e formato obrigatório desta revisão |
| `.local/key-concepts.md` | orientação conceitual do projeto |
| `research/elf/ghidra-rev023-dispatcher-table-resolution.md` | fonte de verdade atual para dispatcher e jump table |
| `research/elf/ghidra-rev022-dispatcher-ground-truth.md` | histórico da contradição corrigida por Rev.023 |
| `research/elf/ghidra-rev021-continue-menu-pivot.md` | contexto anterior, usado com cautela e subordinado à Rev.023 |
| `research/elf/ghidra-rev018-state-transition-dispatch.md` | contexto do sistema de estados/resolvers |
| `research/elf/ghidra-rev019-state-resolver-caller-context.md` | contexto de entity/state offsets |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções reextraídos localmente |
| PCSX2 debugger, R5900 EE, sessão manual de 2026-05-13 | validação runtime parcial do caller, dispatcher e entrada de jump table |

Arquivos de contexto que foram procurados mas não estavam presentes como arquivos legíveis:

| Arquivo | Status |
|---|---|
| `.local/ai-context.md` | não encontrado |
| `key-concepts.md` | não encontrado |

## Dispatcher confirmado

Rev.023 é a fonte de verdade atual.

Sequência confirmada:

| VA | Instrução | Interpretação conservadora |
|---|---|---|
| `0x001d37e4` | `lw $v0,348($s2)` | carrega ponteiro de entidade/contexto via `s2 + 0x15c` |
| `0x001d37e8` | `lw $s3,2048($v0)` | `candidate_state_block_ptr = [entity + 0x800]` |
| `0x001d37ec` | `addiu $s1,$s3,64` | `candidate_state_base = candidate_state_block_ptr + 0x40` |
| `0x001d37f0` | `lw $v1,8($s1)` | `candidate_state_id = [candidate_state_block_ptr + 0x48]` |
| `0x001d37f4` | `sltiu $v0,$v1,5` | bounds check `candidate_state_id < 5` |
| `0x001d37f8` | `beq $v0,$zero,0x001d3818` | estado fora do intervalo cai no bloco `0x001d3818` |
| `0x001d37fc` | `lui $v0,0x0062` | parte alta da base da tabela |
| `0x001d3800` | `sll $v1,$v1,2` | índice `candidate_state_id * 4` |
| `0x001d3804` | `addiu $v0,$v0,-28752` | base efetiva `0x00618fb0` |
| `0x001d3808` | `addu $v1,$v1,$v0` | endereço da entrada |
| `0x001d380c` | `lw $a0,0($v1)` | carrega alvo |
| `0x001d3810` | `jr $a0` | salto indireto |

Observação importante: o caminho out-of-bounds (`candidate_state_id >= 5`) entra em `0x001d3818`, o mesmo alvo da entrada 0 da tabela. Isso torna `state_0_block` também um provável bloco default/reset-like, mas o nome forte deve esperar runtime.

## State table

Verificação byte-level local em `.local/extracted/SCUS_971.13.elf`:

| Entry | VA da entrada | Valor little-endian | Alvo | Primeira palavra no alvo |
|---:|---|---|---|---|
| 0 | `0x00618fb0` | `0x001d3818` | `state_0_block` | `0x0c07c852` |
| 1 | `0x00618fb4` | `0x001d3844` | `state_1_block` | `0x3c10004c` |
| 2 | `0x00618fb8` | `0x001d391c` | `state_2_block` | `0xae600064` |
| 3 | `0x00618fbc` | `0x001d39e0` | `state_3_block` | `0xae600008` |
| 4 | `0x00618fc0` | `0x001d3a10` | `state_4_block` | `0xdfbf0050` |

## Validação runtime parcial — 2026-05-13

Sessão manual no PCSX2 debugger, aba `R5900 (EE)`, usando execute breakpoints.

### Observações confirmadas

| Evento | Observação runtime | Interpretação conservadora |
|---|---|---|
| Hit em caller estático | PC parou em `0x001d3a30`, instrução `addiu sp,sp,-0x50` (`27bdffb0`) | O caller estático é executado durante gameplay/load observado |
| Hit posterior no dispatcher | PC parou em `0x001d37c8`, instrução `addiu sp,sp,-0x60` (`27bdffa0`) | O fluxo runtime alcançou o dispatcher confirmado |
| Hit em leitura de state id | PC parou em `0x001d37f0`, instrução `lw v1,0x8(s1)` | O dispatcher executou a leitura de `candidate_state_id` |
| Hit em entrada da jump table | PC parou em `0x001d380c`, instrução `lw a0,0x0(v1)` | O dispatcher calculou uma entrada de tabela e estava prestes a carregar o alvo |
| Valor de `$v1` em `0x001d380c` | `$v1 = 0x00618fc0` | Entrada usada = `0x00618fb0 + 4 * 4`; portanto o state id observado nesse hit é `4` |

### Registradores observados

Em `0x001d37f0`, screenshot `prints/s3?.png`:

| Registrador | Valor observado |
|---|---|
| `$s1` | `0x0063c8d0` |
| `$s2` | `0x008b3990` |
| `$s3` | `0x00831c58` |
| `$ra` | `0x001d3b0c` |
| `$sp` | `0x0063c8d0` |

Em `0x001d380c`, screenshot `prints/s4?.png`:

| Registrador | Valor observado |
|---|---|
| `$v1` | `0x00618fc0` |
| `$s1` | `0x0063c8d0` |
| `$s2` | `0x008b3990` |
| `$s3` | `0x00831c58` |
| `$ra` | `0x001d3b0c` |
| `$sp` | `0x0063c8d0` |

### Limites da evidência

O alvo carregado em `$a0` após `lw a0,0($v1)` não foi capturado de forma confiável nesta sessão. Como a tabela está verificada byte-level e a entrada runtime observada foi `0x00618fc0`, o alvo esperado é `0x001d3a10`, mas esta frase permanece uma inferência da tabela estática combinada com o endereço runtime da entrada, não uma captura direta de `$a0`.

Não foi usado log automático de registradores. O `emulog.txt` registrou pausas/continuações do VMManager, mas não registrou valores de registradores nem hits detalhados de breakpoints.

## State 0 — `0x001d3818`

Endereço final estimado: `0x001d3840`, seguido pelo início de `state_1_block` em `0x001d3844`.

Instruções principais:

| VA | Instrução | Efeito |
|---|---|---|
| `0x001d3818` | `jal 0x001f2148` | chamada direta com argumento no delay slot |
| `0x001d381c` | `lw $a0,0($s1)` | argumento: `[candidate_state_block_ptr + 0x40]` |
| `0x001d3820` | `beq $v0,$zero,0x001d3a10` | se retorno for zero, vai ao epílogo |
| `0x001d3824` | `addiu $s0,$zero,1` | prepara constante `1` |
| `0x001d3828` | `daddu/addu $a0,$s2,$zero` | argumento: contexto em `$s2` |
| `0x001d382c` | `jal 0x001d2538` | chamada direta |
| `0x001d3830` | `sw $s0,8($s1)` | delay slot: escreve `candidate_state_id = 1` |
| `0x001d3834` | `jal 0x001d2540` | chamada direta |
| `0x001d3838` | `daddu/addu $a0,$s2,$zero` | argumento: contexto em `$s2` |
| `0x001d383c` | `beq $zero,$zero,0x001d3a10` | salto incondicional ao epílogo |
| `0x001d3840` | `sw $s0,100($s3)` | delay slot: escreve `1` em `[candidate_state_block_ptr + 0x64]` |

Chamadas diretas:

| Chamada | Argumento observado | Observação |
|---|---|---|
| `0x001f2148` | `[candidate_state_block_ptr + 0x40]` | retorno controla se o bloco avança |
| `0x001d2538` | `$s2` | executada apenas se `0x001f2148` retorna não zero |
| `0x001d2540` | `$s2` | executada após escrever state id 1 |

Chamadas indiretas: nenhuma dentro do bloco.

Loads relevantes:

| Offset | Base | Uso |
|---:|---|---|
| `+0x40` efetivo | `candidate_state_block_ptr` via `lw $a0,0($s1)` | argumento de `0x001f2148` |

Stores relevantes:

| Offset | Base | Valor |
|---:|---|---|
| `+0x48` | `candidate_state_block_ptr` | `1`, altera `candidate_state_id` |
| `+0x64` | `candidate_state_block_ptr` | `1` |

Constantes: `1`.

Branches:

| VA | Condição | Destino |
|---|---|---|
| `0x001d3820` | retorno de `0x001f2148 == 0` | epílogo `0x001d3a10` |
| `0x001d383c` | incondicional | epílogo `0x001d3a10` |

Classificação:

| Item | Avaliação |
|---|---|
| Altera `candidate_state_id`? | sim, para `1`, quando `0x001f2148` retorna não zero |
| Acessa `candidate_state_block_ptr`? | sim, offsets efetivos `+0x40`, `+0x48`, `+0x64` |
| Acessa entity/context? | sim, passa `$s2` para `0x001d2538` e `0x001d2540` |
| Retorna ao epílogo comum? | sim |
| Papel semântico provável | bloco de entrada/default que testa uma condição/recurso e inicializa transição para estado 1 |
| Confiança | média para a estrutura; baixa para o significado externo das chamadas |

## State 1 — `0x001d3844`

Endereço final estimado: `0x001d3918`, seguido pelo início de `state_2_block` em `0x001d391c`.

Instruções principais:

| VA | Instrução | Efeito |
|---|---|---|
| `0x001d3844` | `lui $s0,0x004c` | prepara ponteiro/constante base |
| `0x001d384c` | `addiu $s0,$s0,18256` | `s0 = 0x004c4750` |
| `0x001d3850` | `jal 0x00105f00` | chamada direta com `$a0=$sp`, `$a1=0x004c4750` |
| `0x001d3858..0x001d38f0` | sequência aritmética/FPU/COP | cálculo numérico usando globals, constantes float e `[s1 + 4]` |
| `0x001d38f4` | `lw $a0,348($v0)` | carrega ponteiro via objeto lido de `[s1]` |
| `0x001d38f8` | `jal 0x00105f00` | chamada direta com argumento ajustado no delay slot |
| `0x001d38fc` | `addiu $a0,$a0,160` | argumento efetivo: `[ [s1] + 0x15c ] + 0xa0` |
| `0x001d3900` | `lw $v0,4($s1)` | lê contador/timer em `[candidate_state_block_ptr + 0x44]` |
| `0x001d3904` | `addiu $v0,$v0,-1` | decrementa contador |
| `0x001d3908` | `bne $v0,$zero,0x001d3a10` | se ainda não zerou, termina |
| `0x001d390c` | `sw $v0,4($s1)` | delay slot: grava contador decrementado |
| `0x001d3910` | `addiu $v0,$zero,2` | prepara state id 2 |
| `0x001d3914` | `beq $zero,$zero,0x001d3a10` | salto ao epílogo |
| `0x001d3918` | `sw $v0,8($s1)` | delay slot: escreve `candidate_state_id = 2` |

Chamadas diretas:

| Chamada | Argumentos observados | Observação |
|---|---|---|
| `0x00105f00` | `$a0=$sp`, `$a1=0x004c4750` | chamada de preparação/cálculo; sem semântica externa confirmada |
| `0x00105f00` | `$a0=[ [s1] + 0x15c ] + 0xa0` | segunda chamada com ponteiro derivado de `[s1]` |

Chamadas indiretas: nenhuma dentro do bloco.

Loads relevantes:

| Offset/endereço | Base | Uso |
|---|---|---|
| `0x00274ec0` | absoluto via `lui 0x0027` | carrega palavra global |
| `0x00274ec4` | absoluto via `lui 0x0027` | carrega palavra global |
| `+0x44` | `candidate_state_block_ptr` | contador/timer decrementado |
| `+0x40` | `candidate_state_block_ptr` | objeto/base usado para derivar argumento da segunda chamada |
| `+0x15c` | objeto lido de `[s1]` | ponteiro usado antes de somar `0xa0` |
| `0x004c4754` | via `s0=0x004c4750` | valor float/constante lida por `lwc1` |

Stores relevantes:

| Offset | Base | Valor |
|---:|---|---|
| `+0x44` | `candidate_state_block_ptr` | contador decrementado |
| `+0x48` | `candidate_state_block_ptr` | `2`, apenas quando contador chega a zero |
| `+0x04` | `$sp` | valor float temporário |

Constantes:

| Valor | Uso observado |
|---|---|
| `0x004c4750` | ponteiro/constante passada para `0x00105f00` |
| `0x00274ec0` | global lido |
| `0x0a` | fator/divisor em cálculo inteiro |
| `0x3c` | constante decimal 60 |
| `0x42700000` | float 60.0 provável |
| `0x43960000` | float 300.0 provável |
| `0x3f800000` | float 1.0 provável |
| `0x3f000000` | float 0.5 provável |
| `0xa0` | offset adicionado a ponteiro derivado |
| `2` | próximo `candidate_state_id` |

Branches:

| VA | Condição | Destino |
|---|---|---|
| `0x001d3908` | contador decrementado != 0 | epílogo `0x001d3a10` |
| `0x001d3914` | incondicional | epílogo `0x001d3a10` |

Classificação:

| Item | Avaliação |
|---|---|
| Altera `candidate_state_id`? | sim, para `2`, quando `[candidate_state_block_ptr + 0x44] - 1 == 0` |
| Acessa `candidate_state_block_ptr`? | sim, offsets efetivos `+0x40`, `+0x44`, `+0x48` |
| Acessa entity/context? | indiretamente via objeto em `[s1] + 0x15c`; não usa `$s2` como argumento direto |
| Retorna ao epílogo comum? | sim |
| Papel semântico provável | bloco de update temporizado/interpolação; aguarda contador chegar a zero antes de ir ao estado 2 |
| Confiança | média para timer/transição; baixa para o significado dos cálculos FPU/COP |

## State 2 — `0x001d391c`

Endereço final estimado: `0x001d39dc`, seguido pelo início de `state_3_block` em `0x001d39e0`.

Instruções principais:

| VA | Instrução | Efeito |
|---|---|---|
| `0x001d391c` | `sw $zero,100($s3)` | zera `[candidate_state_block_ptr + 0x64]` |
| `0x001d3928` | `jal 0x0012abe0` | chamada com `$a0=0x1b8`, `$a1=0` |
| `0x001d392c` | `addiu $s0,$s1,16` | delay slot: `s0 = candidate_state_block_ptr + 0x50` |
| `0x001d3934` | `jal 0x0012ac28` | chamada com `$a0=0x1b8`, `$a1=1` |
| `0x001d3940` | `jal 0x00104508` | chamada com `$a0=s0`, `$a1=s2` |
| `0x001d395c` | `jal 0x00181bf8` | chamada com `$a0=s2`, `$a1=0x11`, `$a2=s0`, `$a3=0` |
| `0x001d3964` | `jal 0x001f19f0` | chamada com `$a0=[s1]` |
| `0x001d3974` | `sw $zero,364($v0)` | zera `[ [s1] + 0x16c ]` |
| `0x001d3978` | `jal 0x001d12d8` | chamada com `$a0=s2` |
| `0x001d397c` | `sw $zero,8($s3)` | delay slot: zera `[candidate_state_block_ptr + 0x08]` |
| `0x001d3980` | `jal 0x001d2548` | chamada com `$a0=s2` |
| `0x001d398c` | `jal 0x0012ade8` | chamada com `$a0=0x1b8`, `$a1=1` |
| `0x001d3994` | `jal 0x0012a618` | chamada com `$a0=0x1b8` |
| `0x001d39ac` | `sw $v0,32($s1)` | salva retorno em `[candidate_state_block_ptr + 0x60]` |
| `0x001d39b0` | `jal 0x00118460` | chamada com `$a0=ret+0x20`, `$a1=s0` |
| `0x001d39c4` | `jal 0x0010d830` | chamada com `$a0=[s1+0x20]+0x30`, `$a1=0x00276140` |
| `0x001d39cc` | `addiu $v0,$zero,3` | prepara state id 3 |
| `0x001d39d0` | `sw $v0,8($s1)` | escreve `candidate_state_id = 3` |
| `0x001d39d4` | `lw $v1,348($s2)` | carrega entidade/contexto via `$s2 + 0x15c` |
| `0x001d39d8` | `beq $zero,$zero,0x001d3a10` | salto ao epílogo |
| `0x001d39dc` | `sw $zero,116($v1)` | delay slot: zera `[entity + 0x74]` |

Chamadas diretas:

| Chamada | Argumentos observados |
|---|---|
| `0x0012abe0` | `0x1b8`, `0` |
| `0x0012ac28` | `0x1b8`, `1` |
| `0x00104508` | `candidate_state_block_ptr + 0x50`, `$s2` |
| `0x00181bf8` | `$s2`, `0x11`, `candidate_state_block_ptr + 0x50`, `0` |
| `0x001f19f0` | `[candidate_state_block_ptr + 0x40]` |
| `0x001d12d8` | `$s2` |
| `0x001d2548` | `$s2` |
| `0x0012ade8` | `0x1b8`, `1` |
| `0x0012a618` | `0x1b8` |
| `0x00118460` | retorno de `0x0012a618 + 0x20`, `candidate_state_block_ptr + 0x50` |
| `0x0010d830` | `[candidate_state_block_ptr + 0x60] + 0x30`, `0x00276140` |

Chamadas indiretas: nenhuma dentro do bloco.

Loads relevantes:

| Offset/endereço | Base | Uso |
|---|---|---|
| `+0x40` | `candidate_state_block_ptr` | argumento de `0x001f19f0`; também base para store `+0x16c` |
| `+0x60` | `candidate_state_block_ptr` | retorno de `0x0012a618`, depois usado como recurso/objeto |
| `+0x15c` | `$s2` | carrega entity/context |
| `0x00276140` | absoluto | argumento de `0x0010d830` |

Stores relevantes:

| Offset | Base | Valor |
|---:|---|---|
| `+0x64` | `candidate_state_block_ptr` | `0` |
| `+0x08` | `candidate_state_block_ptr` | `0` |
| `+0x16c` | objeto em `[candidate_state_block_ptr + 0x40]` | `0` |
| `+0x60` | `candidate_state_block_ptr` | retorno de `0x0012a618` |
| `+0x04` | retorno de `0x0012a618` | float zero |
| `+0x48` | `candidate_state_block_ptr` | `3` |
| `+0x74` | entity/context carregado de `$s2 + 0x15c` | `0` |

Constantes:

| Valor | Uso observado |
|---|---|
| `0x1b8` | argumento repetido para chamadas `0x0012ab*` |
| `1` | argumento de chamadas e flags |
| `0x11` | argumento de `0x00181bf8` |
| `0x42c80000` | float 100.0 provável |
| `0x3f800000` | float 1.0 provável |
| `0x00276140` | ponteiro/estrutura global passada para `0x0010d830` |
| `3` | próximo `candidate_state_id` |

Branches:

| VA | Condição | Destino |
|---|---|---|
| `0x001d39d8` | incondicional | epílogo `0x001d3a10` |

Classificação:

| Item | Avaliação |
|---|---|
| Altera `candidate_state_id`? | sim, para `3` |
| Acessa `candidate_state_block_ptr`? | sim, muitos offsets: `+0x08`, `+0x40`, `+0x48`, `+0x50`, `+0x60`, `+0x64` |
| Acessa entity/context? | sim, usa `$s2` em várias chamadas e zera `[entity + 0x74]` |
| Retorna ao epílogo comum? | sim |
| Papel semântico provável | bloco de setup/configuração de recurso ou subobjeto, seguido de transição para estado 3 |
| Confiança | alta para ser bloco de setup/transição; baixa para o domínio do recurso configurado |

## State 3 — `0x001d39e0`

Endereço final estimado: `0x001d3a0c`, seguido pelo epílogo `0x001d3a10`.

Instruções principais:

| VA | Instrução | Efeito |
|---|---|---|
| `0x001d39e0` | `sw $zero,8($s3)` | zera `[candidate_state_block_ptr + 0x08]` |
| `0x001d39e4` | `jal 0x0012a7f8` | chamada com argumento no delay slot |
| `0x001d39e8` | `addiu $a0,$s1,32` | argumento: `candidate_state_block_ptr + 0x60` |
| `0x001d39ec` | `beq $v0,$zero,0x001d3a10` | se retorno zero, termina sem transição |
| `0x001d39f0` | `addiu $v1,$zero,4` | prepara state id 4 |
| `0x001d39f4` | `addiu $a0,$zero,1` | prepara flag 1 |
| `0x001d39f8` | `sw $v1,8($s1)` | escreve `candidate_state_id = 4` |
| `0x001d39fc` | `lw $v0,348($s2)` | carrega entity/context |
| `0x001d3a00` | `lw $v1,2048($v0)` | carrega state block de entity/context |
| `0x001d3a04` | `sw $zero,116($v0)` | zera `[entity + 0x74]` |
| `0x001d3a08` | `sw $a0,0($v1)` | escreve `1` em `[entity_state_block + 0x00]` |
| `0x001d3a0c` | `sw $zero,364($s2)` | zera `[$s2 + 0x16c]` |

Chamadas diretas:

| Chamada | Argumento observado | Observação |
|---|---|---|
| `0x0012a7f8` | `candidate_state_block_ptr + 0x60` | retorno controla transição para estado 4 |

Chamadas indiretas: nenhuma dentro do bloco.

Loads relevantes:

| Offset | Base | Uso |
|---:|---|---|
| `+0x15c` | `$s2` | carrega entity/context |
| `+0x800` | entity/context | carrega state block associado à entity/context |

Stores relevantes:

| Offset | Base | Valor |
|---:|---|---|
| `+0x08` | `candidate_state_block_ptr` | `0` |
| `+0x48` | `candidate_state_block_ptr` | `4`, se `0x0012a7f8` retorna não zero |
| `+0x74` | entity/context | `0` |
| `+0x00` | `[entity + 0x800]` | `1` |
| `+0x16c` | `$s2` | `0` |

Constantes: `4`, `1`.

Branches:

| VA | Condição | Destino |
|---|---|---|
| `0x001d39ec` | retorno de `0x0012a7f8 == 0` | epílogo `0x001d3a10` |

Classificação:

| Item | Avaliação |
|---|---|
| Altera `candidate_state_id`? | sim, para `4`, quando `0x0012a7f8` retorna não zero |
| Acessa `candidate_state_block_ptr`? | sim, offsets efetivos `+0x08`, `+0x48`, `+0x60` |
| Acessa entity/context? | sim, carrega `[s2 + 0x15c]`, `[entity + 0x800]` e escreve flags |
| Retorna ao epílogo comum? | sim, por fall-through para `0x001d3a10` ou branch |
| Papel semântico provável | bloco de espera/checagem de conclusão, seguido de finalização e transição para estado 4 |
| Confiança | média-alta para espera/finalização; baixa para o significado da flag `[entity_state_block + 0] = 1` |

## State 4 — `0x001d3a10`

Endereço final estimado: `0x001d3a28`.

Instruções principais:

| VA | Instrução | Efeito |
|---|---|---|
| `0x001d3a10` | `ld $ra,0x50($sp)` | restaura registrador salvo |
| `0x001d3a14` | `ld $s3,0x40($sp)` | restaura registrador salvo |
| `0x001d3a18` | `ld $s2,0x30($sp)` | restaura registrador salvo |
| `0x001d3a1c` | `ld $s1,0x20($sp)` | restaura registrador salvo |
| `0x001d3a20` | `ld $s0,0x10($sp)` | restaura registrador salvo |
| `0x001d3a24` | `jr $ra` | retorna |
| `0x001d3a28` | `addiu $sp,$sp,96` | delay slot: desfaz stack frame |

Chamadas diretas: nenhuma.

Chamadas indiretas: nenhuma.

Loads relevantes:

| Offset | Base | Uso |
|---:|---|---|
| `+0x10..+0x50` | `$sp` | restauração de registradores |

Stores relevantes: nenhuma.

Constantes: `96`.

Branches:

| VA | Condição | Destino |
|---|---|---|
| `0x001d3a24` | retorno de função | `$ra` |

Classificação:

| Item | Avaliação |
|---|---|
| Altera `candidate_state_id`? | não |
| Acessa `candidate_state_block_ptr`? | não |
| Acessa entity/context? | não |
| Retorna ao epílogo comum? | é o próprio epílogo comum |
| Papel semântico provável | estado terminal/no-op de saída ou alvo usado para encerrar o processamento |
| Confiança | alta para epílogo; baixa para motivo de existir como entrada de tabela |

## Tabela comparativa dos estados

| Estado | Range estimado | Chamadas diretas | Stores de estado | Papel provável | Confiança |
|---:|---|---:|---|---|---|
| 0 | `0x001d3818..0x001d3840` | 3 | `candidate_state_id = 1` | entrada/default/teste inicial | média |
| 1 | `0x001d3844..0x001d3918` | 2 | `candidate_state_id = 2` quando contador zera | update temporizado/interpolação | média |
| 2 | `0x001d391c..0x001d39dc` | 10 | `candidate_state_id = 3` | setup/configuração de recurso | média-alta |
| 3 | `0x001d39e0..0x001d3a0c` | 1 | `candidate_state_id = 4` quando consulta retorna não zero | espera/finalização | média-alta |
| 4 | `0x001d3a10..0x001d3a28` | 0 | nenhuma | epílogo/saída | alta para mecânica, baixa para semântica |

## Writes de estado encontrados

| VA | Instrução | Campo | Valor | Condição |
|---|---|---|---|---|
| `0x001d3830` | `sw $s0,8($s1)` | `[candidate_state_block_ptr + 0x48]` | `1` | `0x001f2148` retornou não zero |
| `0x001d3918` | `sw $v0,8($s1)` | `[candidate_state_block_ptr + 0x48]` | `2` | contador `[candidate_state_block_ptr + 0x44]` chegou a zero |
| `0x001d39d0` | `sw $v0,8($s1)` | `[candidate_state_block_ptr + 0x48]` | `3` | sempre que `state_2_block` executa até o fim |
| `0x001d39f8` | `sw $v1,8($s1)` | `[candidate_state_block_ptr + 0x48]` | `4` | `0x0012a7f8` retornou não zero |

Outros stores relevantes:

| VA | Campo | Valor | Observação |
|---|---|---|---|
| `0x001d3840` | `[candidate_state_block_ptr + 0x64]` | `1` | flag/campo auxiliar em state 0 |
| `0x001d391c` | `[candidate_state_block_ptr + 0x64]` | `0` | limpa flag/campo auxiliar em state 2 |
| `0x001d397c` | `[candidate_state_block_ptr + 0x08]` | `0` | limpa campo auxiliar em state 2 |
| `0x001d39e0` | `[candidate_state_block_ptr + 0x08]` | `0` | limpa campo auxiliar em state 3 |
| `0x001d39dc` | `[entity + 0x74]` | `0` | limpa campo de entity/context em state 2 |
| `0x001d3a04` | `[entity + 0x74]` | `0` | limpa campo de entity/context em state 3 |
| `0x001d3a08` | `[[entity + 0x800] + 0x00]` | `1` | flag no state block da entity/context |
| `0x001d3a0c` | `[$s2 + 0x16c]` | `0` | limpa campo do contexto original |

## Chamadas externas relevantes

| Função | Estados | Papel local observado | Semântica externa |
|---|---|---|---|
| `0x001f2148` | 0 | consulta/teste que controla avanço para estado 1 | desconhecida |
| `0x001d2538` | 0 | chamada após decisão de avanço | desconhecida |
| `0x001d2540` | 0 | chamada após escrever state 1 | desconhecida |
| `0x00105f00` | 1 | chamada duas vezes em contexto de cálculo/ponteiros | desconhecida |
| `0x0012abe0` | 2 | chamada com ID/constante `0x1b8` | desconhecida |
| `0x0012ac28` | 2 | chamada com `0x1b8`, `1` | desconhecida |
| `0x00104508` | 2 | chamada com `candidate_state_block_ptr + 0x50` e `$s2` | desconhecida |
| `0x00181bf8` | 2 | chamada com `$s2`, `0x11`, `candidate_state_block_ptr + 0x50` | desconhecida |
| `0x001f19f0` | 2 | chamada com `[candidate_state_block_ptr + 0x40]` | desconhecida |
| `0x001d12d8` | 2 | chamada com `$s2` | desconhecida |
| `0x001d2548` | 2 | chamada com `$s2` | desconhecida |
| `0x0012ade8` | 2 | chamada com `0x1b8`, `1` | desconhecida |
| `0x0012a618` | 2 | retorna ponteiro salvo em `[candidate_state_block_ptr + 0x60]` | Rev.021 associou a display/validation; aqui a semântica exata fica aberta |
| `0x00118460` | 2 | configura retorno de `0x0012a618` com state sub-struct | desconhecida |
| `0x0010d830` | 2 | chamada com retorno de `0x0012a618 + 0x30` e `0x00276140` | desconhecida |
| `0x0012a7f8` | 3 | consulta sobre `[candidate_state_block_ptr + 0x60]`; retorno controla state 4 | desconhecida |

## Pseudocódigo conservador

```c
void candidate_state_dispatcher(context *ctx) {
    entity = *(ctx + 0x15c);
    candidate_state_block_ptr = *(entity + 0x800);
    state_base = candidate_state_block_ptr + 0x40;
    candidate_state_id = *(state_base + 0x8);

    if (candidate_state_id >= 5) {
        goto state_0_block;
    }

    switch (candidate_state_id) {
    case 0:
state_0_block:
        if (FUN_001f2148(*(state_base + 0x0)) != 0) {
            *(state_base + 0x8) = 1;
            FUN_001d2538(ctx);
            FUN_001d2540(ctx);
            *(candidate_state_block_ptr + 0x64) = 1;
        }
        return;

    case 1:
        FUN_00105f00(stack_temp, 0x004c4750);
        /* numeric/FPU/COP calculation omitted; exact semantics unknown */
        FUN_00105f00(derived_pointer_from_state_base_plus_0x15c_plus_0xa0);
        counter = *(state_base + 0x4) - 1;
        *(state_base + 0x4) = counter;
        if (counter == 0) {
            *(state_base + 0x8) = 2;
        }
        return;

    case 2:
        *(candidate_state_block_ptr + 0x64) = 0;
        sub = state_base + 0x10;
        FUN_0012abe0(0x1b8, 0);
        FUN_0012ac28(0x1b8, 1);
        FUN_00104508(sub, ctx);
        FUN_00181bf8(ctx, 0x11, sub, 0);
        FUN_001f19f0(*(state_base + 0x0));
        *(*(state_base + 0x0) + 0x16c) = 0;
        *(candidate_state_block_ptr + 0x08) = 0;
        FUN_001d12d8(ctx);
        FUN_001d2548(ctx);
        FUN_0012ade8(0x1b8, 1);
        resource = FUN_0012a618(0x1b8);
        *(state_base + 0x20) = resource;
        *(resource + 0x04) = 0.0f;
        FUN_00118460(resource + 0x20, sub);
        FUN_0010d830(resource + 0x30, 0x00276140);
        *(state_base + 0x8) = 3;
        *(*(ctx + 0x15c) + 0x74) = 0;
        return;

    case 3:
        *(candidate_state_block_ptr + 0x08) = 0;
        if (FUN_0012a7f8(state_base + 0x20) != 0) {
            *(state_base + 0x8) = 4;
            entity = *(ctx + 0x15c);
            entity_state_block = *(entity + 0x800);
            *(entity + 0x74) = 0;
            *(entity_state_block + 0x00) = 1;
            *(ctx + 0x16c) = 0;
        }
        return;

    case 4:
        return;
    }
}
```

Notas sobre o pseudocódigo:

- `state_base` corresponde a `$s1 = candidate_state_block_ptr + 0x40`.
- O campo documentado como `candidate_state_id` está em `[state_base + 0x8]`, equivalente a `[candidate_state_block_ptr + 0x48]`.
- O pseudocódigo não tenta nomear recursos, gameplay ou renderização além do que as instruções sustentam.

## O que fica confirmado

1. A jump table real usada pelo dispatcher é `0x00618fb0`.
2. Os cinco alvos são basic blocks internos, não entry points independentes.
3. `state_0_block` também é o alvo do caminho out-of-bounds (`candidate_state_id >= 5`).
4. O campo `candidate_state_id` é lido de `[candidate_state_block_ptr + 0x48]`.
5. Há writes explícitos para `candidate_state_id` com valores `1`, `2`, `3` e `4`.
6. Todos os estados retornam pelo epílogo comum em `0x001d3a10`.
7. `state_4_block` é o epílogo comum e não contém lógica própria de estado.
8. Em runtime, durante a sessão de 2026-05-13, o caller `0x001d3a30` alcançou o dispatcher `0x001d37c8`.
9. Em runtime, o dispatcher alcançou `0x001d37f0` e `0x001d380c`.
10. Em runtime, um hit em `0x001d380c` usou `$v1 = 0x00618fc0`, isto é, a entrada de tabela correspondente a `candidate_state_id = 4`.

## O que fica provável

1. `state_0_block` é um bloco de entrada/default/reset-like, porque também recebe IDs fora do intervalo e só avança se uma chamada de teste retorna não zero.
2. `state_1_block` é um bloco temporizado/update-like, porque decrementa `[candidate_state_block_ptr + 0x44]` e só muda para estado 2 quando o contador zera.
3. `state_2_block` é setup-like, porque executa a maior sequência de chamadas e cria/configura um ponteiro salvo em `[candidate_state_block_ptr + 0x60]`.
4. `state_3_block` é wait/cleanup-like, porque consulta `[candidate_state_block_ptr + 0x60]` e só finaliza a transição quando a consulta retorna não zero.
5. A sequência estrutural provável é `0/default -> 1 -> 2 -> 3 -> 4`, com estado 4 funcionando como saída/terminal.

## O que fica possível

1. Os campos `[candidate_state_block_ptr + 0x08]`, `+0x64`, `[entity + 0x74]`, `[ctx + 0x16c]` e `[[entity + 0x800] + 0x00]` podem ser flags auxiliares de ciclo/transição, mas isso ainda não está demonstrado.
2. A chamada `0x0012a618` em `state_2_block` pode estar ligada a criação/configuração de recurso visual ou display, coerente com Rev.021, mas este arquivo não eleva isso a conclusão.
3. O valor `0x1b8` pode ser um ID de recurso/subsistema usado por várias chamadas `0x0012ab*`, mas seu significado permanece desconhecido.
4. Os cálculos FPU/COP em `state_1_block` podem indicar interpolação, temporização ou ajuste espacial, mas a decodificação semântica exige análise das chamadas e runtime.

## O que ainda precisa de runtime

1. Capturar diretamente o valor de `$a0` logo após `lw a0,0($v1)` em `0x001d380c`.
2. Confirmar quais outros `candidate_state_id` aparecem em gameplay real.
3. Confirmar se IDs fora de `0..4` ocorrem e se caem intencionalmente em `state_0_block`.
4. Capturar valores de:
   - `$s2`;
   - `[s2 + 0x15c]`;
   - `candidate_state_block_ptr`;
   - `[candidate_state_block_ptr + 0x44]`;
   - `[candidate_state_block_ptr + 0x48]`;
   - `[candidate_state_block_ptr + 0x60]`;
   - `[candidate_state_block_ptr + 0x64]`.
5. Confirmar o significado de retorno de:
   - `0x001f2148`;
   - `0x0012a618`;
   - `0x0012a7f8`.
6. Confirmar se `state_4_block` é usado como estado persistente, estado terminal ou apenas saída da função.

## Próximo teste mínimo recomendado

Teste mínimo recomendado para runtime:

1. Substituir o fluxo manual de screenshots por um método menos oneroso:
   - cópia textual do painel de registradores, se o PCSX2 permitir;
   - ou patch/instrumentação temporária que grave os valores em RAM;
   - ou trace/debug externo se houver uma opção prática.
2. Capturar `$a0` após `0x001d380c` para confirmar diretamente o alvo da entrada `0x00618fc0`.
3. Breakpoints em writes de estado:
   - `0x001d3830`;
   - `0x001d3918`;
   - `0x001d39d0`;
   - `0x001d39f8`.
4. Para cada hit, registrar:
   - `$s2`;
   - `[s2 + 0x15c]`;
   - `$s3`;
   - `$s1`;
   - valor antigo e novo de `[s1 + 0x8]`;
   - contador `[s1 + 0x4]`;
   - recurso `[s1 + 0x20]`.

Esse teste valida a sequência real de estados sem exigir investigação de assets, overlays ou strings.

## Veredito

Rev.024 confirma, em nível de instrução, que os cinco alvos da jump table formam uma pequena máquina de estados interna com transições explícitas:

```txt
state_0_block -> state_1_block -> state_2_block -> state_3_block -> state_4_block
```

A semântica estrutural é bem sustentada: entrada/default, update temporizado, setup, espera/finalização e epílogo. A semântica de gameplay continua desconhecida. Não há base suficiente para nomes fortes como Yorda, capture, continue, menu, death, final ou animation state.

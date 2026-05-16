# PCSX2 Recompiler Logpoints Session

## Date

2026-05-16

## Objective

Testar uma build local do PCSX2 com logpoints internos no caminho do EE
Recompiler, mantendo a emulacao jogavel, para substituir o teste anterior em
EE Interpreter que ficou lento demais para navegacao normal.

O alvo pratico era confirmar se a captura automatica poderia observar:

```txt
0x001d27a8  cloth_payload_init candidate
0x0013f7a8  callback registration, filtered by a3 == 0x13
0x001b7a74  descriptor iteration context
```

## Scope

Incluido:

- execucao local do PCSX2 modificado em modo EE Recompiler;
- captura JSONL em `.local/runtime-captures/ico-probe/events.jsonl`;
- validacao de hook por sentinela de entrada ELF;
- resumo dos callsites observados via `ra - 8`;
- correlacao conservadora com Rev.037.

Excluido:

- distribuicao de binarios, ISO, savestates ou assets proprietarios;
- conclusao semantica sobre gameplay;
- prova negativa definitiva para `0x001d27a8`;
- prova de origem final para callback `0x001d3a30`.

## Sources Used

| Fonte | Uso |
|---|---|
| `research/runtime/dynamic-logpoints-poc-evaluation.md` | plano original do POC de logpoints |
| `research/runtime/next-runtime-session-after-rev046.md` | alvos runtime seguintes |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | mapa dos cinco callsites de `0x0013f7a8` |
| `research/elf/ghidra-rev046-runtime-a1-source-resolved.md` | contexto runtime anterior de `0x001d27a8` |
| `research/elf/ghidra-rev047-descriptor-callback-runtime-model.md` | modelo consolidado de descriptor/callback |
| `.local/runtime-captures/ico-probe/events.jsonl` | log bruto da sessao |
| `/home/peter/Documentos/repos/pcsx2-ico-logpoints` | build local instrumentada do PCSX2 |

## Evidence Used

| Evidencia | Resultado |
|---|---|
| `pc == 0x00100008` no log | sentinela confirma que o hook no Recompiler executou |
| 640 eventos JSONL | captura automatica funcionou em modo jogavel |
| `pc == 0x0013f7a8`, `a3 == 0x13` | registro de callback filtrado foi observado |
| `ra == 0x00201edc` em um hit | callsite provavel `0x00201ed4`, candidato runtime de Rev.037 |
| ausencia de `pc == 0x001d27a8` | alvo cloth_payload_init nao foi observado nesta sessao |
| varredura de `s0_words_32` nos hits de `0x001b7a74` | nao encontrou `0x001d27a8`, `0x001d3a30` ou `0x001d3b28` nos dumps capturados |

## Byte-Level Or Instruction-Level Findings

Esta sessao nao adicionou nova verificacao de bytes do ELF. A base
instrucional usada permanece a de Rev.037 para o callsite `0x00201ed4`:

```asm
0x00201ecc: daddu   a0,s0,zero
0x00201ed0: daddu   a2,zero,zero
0x00201ed4: jal     0x0013f7a8
0x00201ed8: addiu   a3,zero,+0x13
```

O achado runtime desta sessao e que um evento em `0x0013f7a8` retornou com:

```txt
ra       = 0x00201edc
callsite = 0x00201ed4
a3       = 0x00000013
```

Isso vincula a captura ao callsite descrito acima, usando a convencao
`callsite = ra - 8` para `jal` com delay slot.

## Runtime Results

Resumo agregado do log:

| Campo | Valor |
|---|---:|
| eventos JSONL | 640 |
| hits com `a1 == sp` | 0 |
| hits em `0x001d27a8` | 0 |
| hits em `0x0013f7a8` com `a3 == 0x13` | presentes |
| modo de CPU pratico | EE Recompiler |

Callsites provaveis observados:

| Callsite provavel | Contagem | Interpretacao conservadora |
|---|---:|---|
| `0x001b7a6c` | 393 | caminho de iteracao/registro de descriptor; ruidoso nesta instrumentacao |
| `0x001b7acc` | 186 | fallback `descriptor_label+0x40` descrito em Rev.037 |
| `0x001b7ab0` | 39 | registro `entry[+0x24]` descrito em Rev.037 |
| `0x00240f90` | 20 | caminho literal `t0` descrito em Rev.037 |
| `0x00082618` | 1 | chamada associada a sentinela de entrada ELF |
| `0x00201ed4` | 1 | candidato runtime `0x00201e70` descrito em Rev.037 |

Evento mais relevante da sessao:

| Campo | Valor |
|---|---|
| `pc` | `0x0013f7a8` |
| label | `callback_register_a3_13` |
| `ra` | `0x00201edc` |
| callsite provavel | `0x00201ed4` |
| `a0` | `0x0083467c` |
| `a1` | `0x002342c8` |
| `a2` | `0x00000000` |
| `a3` | `0x00000013` |
| `sp` | `0x0063c8f0` |
| `s0` | `0x0083467c` |
| `s3` | `0x00000001` |
| ciclo | `122504820803` |

## Confirmed

1. A instrumentacao no EE Recompiler funciona em modo jogavel. A sentinela em
   `0x00100008` apareceu no log.
2. O breakpoint/logpoint condicional de `0x0013f7a8` com `a3 == 0x13`
   capturou eventos reais durante gameplay.
3. O caminho runtime-dependente `0x00201e70` / callsite `0x00201ed4`, listado
   em Rev.037 como nao resolvido estaticamente, foi atingido pelo menos uma
   vez nesta sessao.
4. No hit observado em `0x00201ed4`, o callback passado em `a1` foi
   `0x002342c8`, nao `0x001d3a30`.
5. `0x001d27a8` nao apareceu no log desta sessao.

## Probable

1. A captura em Recompiler e o caminho correto para continuar os testes
   jogaveis; o Interpreter e lento demais para esta investigacao sem savestate
   muito proximo do evento.
2. O log em `0x001b7a74` esta amplo demais e deve ser filtrado ou substituido
   por alvos mais especificos.
3. O callsite `0x00201ed4` e runtime-real, mas a instancia capturada nao e a
   instancia que registra o callback `0x001d3a30`.

## Possible

1. Outro contexto de sala, objeto ou momento pode atingir `0x00201ed4` com
   `a1 == 0x001d3a30`.
2. `0x001d27a8` pode exigir uma cena, objeto cloth ou savestate anterior
   diferente do trecho jogado nesta sessao.
3. O alvo `0x001b7a74` pode precisar de filtro por descriptor, `s4`, `s7`,
   `a1` ou faixa de callbacks para virar evidencia util.

## Unknown

1. Qual evento runtime registra efetivamente `0x001d3a30` como callback.
2. Se `0x00201ed4` alguma vez recebe `a1 == 0x001d3a30`.
3. Quem escreve o valor que eventualmente leva a `node + 0x1c == 0x001d3a30`.
4. Qual cena ou asset minimiza o caminho ate `0x001d27a8`.

## Discarded

1. Continuar usando apenas EE Interpreter para navegacao normal: a lentidao
   tornou o teste impraticavel.
2. Tratar ausencia de hit em `0x001d27a8` nesta sessao como prova negativa
   definitiva. A sessao valida o hook, mas nao cobre todos os contextos.
3. Tratar os hits ruidosos de `0x001b7a74` como evidencia sem classificacao
   adicional.

## Next Minimum Test

Reduzir o ruido antes de pedir nova sessao jogavel:

| Prioridade | Mudanca minima | Motivo |
|---|---|---|
| 1 | logar `0x00201ed4` diretamente, com `a1`, `a0`, `s0`, `s1`, indice e palavras proximas | confirmar variacao do candidato runtime sem depender so de `ra` |
| 2 | manter `0x0013f7a8`, mas destacar `a1 == 0x001d3a30` e `ra == 0x00201edc` | separar evento raro de ruido comum |
| 3 | reduzir ou condicionar `0x001b7a74` | evitar logs grandes sem ganho de evidencia |
| 4 | preservar `0x00100008` apenas como teste de smoke opcional | confirmar build/hook sem poluir sessao longa |
| 5 | manter `0x001d27a8` como alvo, mas considerar savestate/cena mais especifica | ausencia atual nao resolve o alvo |

## Follow-Up Patch Prepared

O patch exportado em `research/runtime/pcsx2-ico-recompiler-probe.patch`
implementa a mudanca minima seguinte:

| Mudanca | Estado |
|---|---|
| alvo direto `0x00201ed4` no EE Recompiler | preparado e compilado localmente |
| filtro de `0x0013f7a8` para `a1 == 0x001d3a30` ou `ra == 0x00201edc` | preparado e compilado localmente |
| remocao do alvo amplo `0x001b7a74` no Recompiler | preparado e compilado localmente |
| manutencao de `0x001d27a8` | preparado e compilado localmente |

Essa preparacao ainda nao e uma nova evidencia runtime. Ela apenas reduz o
ruido esperado para a proxima sessao jogavel.

## Conservative Verdict

A sessao foi positiva para a infraestrutura: a build instrumentada do PCSX2 em
EE Recompiler consegue capturar eventos JSONL sem tornar o jogo injogavel.

O achado tecnico mais importante e runtime, mas limitado: o callsite
`0x00201ed4`, antes marcado como candidato dependente de runtime em Rev.037,
foi observado chamando `0x0013f7a8` com `a3 == 0x13`. Nesse evento especifico,
`a1 == 0x002342c8`, portanto ele nao registrou `0x001d3a30`.

O proximo passo deve ser refinar os logpoints para perseguir o candidato
`0x00201ed4` com menos ruido, antes de outra sessao longa de gameplay.

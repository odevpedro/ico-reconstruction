# PCSX2 Recompiler Focused Logpoints Session

## Date

2026-05-16

## Objective

Executar uma segunda sessao jogavel com a build local do PCSX2 instrumentada
no EE Recompiler, agora com logpoints reduzidos e focados nos alvos que
restaram apos a sessao anterior:

```txt
0x00201ed4  runtime-dependent registration candidate
0x0013f7a8  callback registration, filtered
0x001d27a8  cloth payload initializer
```

## Scope

Incluido:

- uso do patch exportado em `research/runtime/pcsx2-ico-recompiler-probe.patch`;
- captura JSONL em `.local/runtime-captures/ico-probe/events.jsonl`;
- monitoramento do log com `tools/runtime-probe-analyzer/runtime_probe_analyzer.py`;
- validacao de que os logpoints funcionam em EE Recompiler com emulacao
  jogavel;
- correlacao dos hits com Rev.037 e Rev.046.

Excluido:

- distribuicao de binarios, ISO, savestates, memoria bruta ou assets;
- conclusao semantica sobre gameplay;
- prova de que `0x001d3a30` nunca e registrado;
- proposta upstream final para PCSX2.

## Sources Used

| Fonte | Uso |
|---|---|
| `research/runtime/pcsx2-recompiler-logpoints-session-2026-05-16.md` | sessao anterior e motivacao do filtro |
| `research/runtime/pcsx2-ico-recompiler-probe.patch` | patch PCSX2 usado nesta sessao |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | caller `0x00203ea0` e callsite `0x00201ed4` |
| `research/elf/ghidra-rev046-runtime-a1-source-resolved.md` | baseline manual para `0x001d27a8` |
| `research/elf/ghidra-rev047-descriptor-callback-runtime-model.md` | modelo descriptor/callback consolidado |
| `.local/runtime-captures/ico-probe/events.jsonl` | log bruto local da sessao |
| `tools/runtime-probe-analyzer/runtime_probe_analyzer.py` | sumarizacao e classificacao dos callsites |

## Evidence Used

| Evidencia | Resultado |
|---|---|
| `pc == 0x00100008` | sentinela confirma hook ativo no Recompiler |
| `pc == 0x00201ed4` | candidato runtime foi observado diretamente |
| `ra == 0x00203ea8` em `0x00201ed4` | caller provavel `0x00203ea0`, um dos callers runtime-dependentes de Rev.037 |
| `pc == 0x0013f7a8`, `ra == 0x00201edc` | `0x00201ed4` chamou o registrador de callback |
| `pc == 0x001d27a8`, `ra == 0x001b7a88` | initializer cloth foi observado em Recompiler via caller `0x001b7a80` |
| `a1 == sp` nos hits de `0x001d27a8` | reproduz a evidencia manual de Rev.046 |
| `[a1+0x30]` em `0x001d27a8` | valores `0` e `1` capturados |

## Byte-Level Or Instruction-Level Findings

Esta sessao nao adicionou nova verificacao byte-level do ELF. Ela usa as
instrucoes ja mapeadas em Rev.037 para `0x00201ed4`:

```asm
0x00201ecc: daddu   a0,s0,zero
0x00201ed0: daddu   a2,zero,zero
0x00201ed4: jal     0x0013f7a8
0x00201ed8: addiu   a3,zero,+0x13
```

E usa a convencao runtime:

```txt
callsite = ra - 8
```

para atribuir:

| Runtime `ra` | Callsite provavel | Interpretacao |
|---|---|---|
| `0x00203ea8` | `0x00203ea0` | caller de `0x00201e70`, usando `[s0+4]` como callback |
| `0x00201edc` | `0x00201ed4` | chamada de `0x0013f7a8` dentro de `0x00201e70` |
| `0x001b7a88` | `0x001b7a80` | `jalr` via descriptor `+0x58` para `0x001d27a8` |

## Runtime Results

Resumo agregado final da sessao:

| Campo | Valor |
|---|---:|
| eventos JSONL | 26 |
| sentinelas `0x00100008` | 1 |
| hits diretos em `0x00201ed4` | 5 |
| hits em `0x0013f7a8` via `0x00201ed4` | 5 |
| hits em `0x001d27a8` | 15 |
| hits com `a1 == sp` | 15 |
| hits com `[a1+0x30] == 0` | 9 |
| hits com `[a1+0x30] == 1` | 6 |

Callsites observados:

| Callsite | Contagem | Classificacao |
|---|---:|---|
| `0x001b7a80` | 15 | `descriptor +0x58 jalr` para `0x001d27a8` |
| `0x00203ea0` | 5 | caller de `0x00201e70`, fonte `[s0+4]` |
| `0x00201ed4` | 5 | candidato runtime de registro |
| `0x00082618` | 1 | sentinela de entry ELF |

Valores de `a1` observados no caminho `0x00203ea0 -> 0x00201ed4`:

| `a1` | Contagem | Observacao |
|---|---:|---|
| `0x002342c8` | 2 | callback registrado em eventos repetidos |
| `0x002342f8` | 1 | callback registrado |
| `0x00232350` | 1 | callback registrado |
| `0x00234f38` | 1 | callback registrado em outro `a0` |

Nenhum desses valores e `0x001d3a30`.

Valores unicos de `a0` observados por alvo:

| `pc` | Valores unicos de `a0` | Observacao |
|---|---:|---|
| `0x00201ed4` | 2 | dois objetos/contextos passaram pelo candidato runtime |
| `0x0013f7a8` | 2 | corresponde aos mesmos objetos/contextos do registro via `0x00201ed4` |
| `0x001d27a8` | 5 | cinco objetos/contextos inicializaram payload cloth |

Valores de `[a1+0x30]` nos hits de `0x001d27a8`:

| Valor | Contagem | Observacao |
|---|---:|---|
| `0x00000000` | 9 | variant/mode zero observado em Recompiler |
| `0x00000001` | 6 | variant/mode um observado em Recompiler |

## Confirmed

1. A instrumentacao focada no EE Recompiler e jogavel e reduz o ruido de forma
   significativa.
2. O caminho `0x00203ea0 -> 0x00201e70/0x00201ed4 -> 0x0013f7a8` foi observado
   diretamente em runtime.
3. O caller `0x00203ea0`, descrito estaticamente em Rev.037 como dependente de
   runtime, passa callbacks variados via `a1`.
4. Nos hits capturados desse caminho, `a1` nao foi `0x001d3a30`.
5. `0x001d27a8` foi capturado em modo Recompiler com `a1 == sp`, reproduzindo
   a conclusao manual de Rev.046 sem depender do Interpreter lento.
6. `[a1+0x30]` em `0x001d27a8` foi observado com pelo menos dois valores:
   `0` e `1`.

## Probable

1. `[a1+0x30]` funciona como campo runtime de variant/mode do initializer
   cloth, coerente com Rev.043 e Rev.046.
2. Os grupos de hits em `0x001d27a8` representam inicializacao de multiplos
   objetos/instancias relacionados ao mesmo descriptor `s0 == 0x002a3924`.
3. O caminho `0x00203ea0` registra callbacks de objetos ou entradas carregadas
   dinamicamente, mas os valores observados nesta sessao nao correspondem ao
   callback cloth `0x001d3a30`.

## Possible

1. Outra area, sala, carregamento ou estado pode fazer `0x00203ea0` passar
   `a1 == 0x001d3a30`.
2. Outros valores de `[a1+0x30]` alem de `0` e `1` podem aparecer em cenas com
   cloth diferente.
3. O upstream do PCSX2 poderia aceitar uma versao generica dessa ideia como
   logpoints/debug tracepoints, mas isso ainda requer estabilizacao e design
   separado do POC hardcoded.

## Unknown

1. Qual chamada registra efetivamente `0x001d3a30`.
2. Se `0x00203ea0` alguma vez passa `a1 == 0x001d3a30`.
3. O significado semantico exato dos valores `0` e `1` em `[a1+0x30]`.
4. Se outros descriptors alem de `0x002a3924` atingem `0x001d27a8` nesta mesma
   fase de jogo.

## Discarded

1. A ideia de que `0x00201ed4` era apenas um candidato estatico sem alcance
   runtime: ele foi observado diretamente.
2. A necessidade de usar EE Interpreter para capturar `0x001d27a8`: o
   Recompiler instrumentado capturou o alvo.
3. Tratar o logpoint amplo em `0x001b7a74` como necessario para esta etapa:
   a sessao focada produziu evidencia melhor com menos ruido.

## Next Minimum Test

| Prioridade | Teste | Motivo |
|---|---|---|
| 1 | manter o filtro `a1 == 0x001d3a30` em `0x0013f7a8` durante uma progressao maior | buscar o registro do callback alvo sem inundar o log |
| 2 | capturar tambem o caller `0x00203080` de `0x00201e70` se ele aparecer em outra rota | cobrir o outro caller runtime-dependente de Rev.037 |
| 3 | capturar mais cenas com cloth visivel e salvar o trecho de jogo correspondente em nota textual | relacionar variant `0/1` a contexto observavel sem afirmar semantica |
| 4 | depois de mais uma sessao, escrever proposta separada de logpoints genericos para PCSX2 | manter upstream separado do POC hardcoded |

## Offline Analyzer Follow-Up

Apos a sessao, `tools/runtime-probe-analyzer/runtime_probe_analyzer.py` foi
atualizado para listar:

- valores unicos de `a1` por callsite;
- valores unicos de `a0` por `pc`.

Esse refinamento nao adiciona nova evidencia runtime, mas torna a comparacao
entre sessoes mais auditavel. No log desta sessao, o resumo confirmou que
`0x00203ea0` e `0x00201ed4` compartilham os mesmos quatro valores unicos de
`a1`, todos diferentes de `0x001d3a30`.

## Conservative Verdict

A sessao estabiliza o POC: logpoints no EE Recompiler conseguem capturar os
alvos relevantes sem tornar o jogo impraticavel e sem gerar ruido excessivo.

O resultado mais forte e a reproducao de `0x001d27a8` em modo jogavel:
`a1 == sp`, caller `0x001b7a80`, descriptor `s0 == 0x002a3924`, e
`[a1+0x30]` variando entre `0` e `1`.

O caminho `0x00203ea0 -> 0x00201ed4 -> 0x0013f7a8` tambem esta confirmado em
runtime, mas os callbacks observados ainda nao incluem `0x001d3a30`.

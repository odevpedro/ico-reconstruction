# Dynamic Logpoints POC Evaluation for PCSX2 / ICO Reconstruction

## Date

2026-05-15

## Objective

Decidir o menor experimento implementavel para substituir parte dos
breakpoints manuais do PCSX2 por logpoints dinamicos confiaveis, com foco
inicial em:

```txt
0x001d27a8
```

O objetivo pratico e capturar automaticamente registradores e memoria
sincronamente quando o EE/R5900 entra nesse endereco.

## Scope

Incluido:

- avaliacao de PINE, BREAK/SYSCALL, hook interno e arquitetura hibrida;
- POC hardcoded para um unico endereco;
- estrategia de savestate;
- dados minimos de runtime para o cluster cloth;
- caminho de evolucao para JSONL, SQLite e relatorios Markdown.

Excluido:

- implementar ou compilar um fork local do PCSX2;
- automatizar UI do debugger;
- inferir novos significados sem captura runtime;
- distribuir binarios, assets, savestates ou dumps proprietarios.

## Sources Used

| Fonte | Uso |
|---|---|
| `prompt-logpoints-dinamicos-ico-reconstruction-v3-1.md` | Especificacao da avaliacao |
| `research/runtime-capture-automation-plan.md` | Campos e breakpoints existentes |
| `research/runtime-pcsx2-guide.md` | Fluxo manual atual |
| `research/elf/ghidra-rev046-runtime-a1-source-resolved.md` | Estado runtime mais recente para `a1` |
| `research/elf/ghidra-rev047-descriptor-callback-runtime-model.md` | Modelo runtime consolidado |
| PCSX2 upstream `pcsx2/Interpreter.cpp` | Validacao das ancoras `intExecute`, `execI`, `cpuRegs.pc`, `cpuRegs.GPR` |

## Evidence Used

| Evidencia | Resultado |
|---|---|
| Rev.046 runtime | `a1 == sp` no hit observado em `0x001d27a8`; `ra == 0x001b7a88` |
| Rev.047 runtime | callback vem de `[descriptor + 0x58]`; caller em `0x001b7a74` |
| PCSX2 upstream atual | `execI()` e chamado por instrucao; `intExecute()` contem o loop do EE Interpreter; `cpuRegs.pc` e `cpuRegs.GPR.n.<reg>.UL[0]` continuam presentes |
| Ausencia de checkout local do PCSX2 | Esta nota nao afirma build/teste compilado do patch |

## 1. Decisao tecnica

```txt
Mecanismo primario recomendado:
Hook/logpoint interno minimo no PCSX2 EE Interpreter, inicialmente hardcoded
para pc == 0x001d27a8.

Mecanismos auxiliares:
PINE para validacao auxiliar, leitura manual fora do evento critico, patches
pontuais e comparacao antes/depois. Analyzer Python para transformar JSONL em
SQLite e Markdown.

Mecanismos descartados:
Polling externo de PC via PINE como mecanismo primario. BREAK/SYSCALL via PINE
como fluxo primario para logs repetiveis.

Motivo da decisao:
A evidencia forte exige captura sincronica no momento exato da entrada da
funcao. O Interpreter executa uma instrucao por vez e ja possui `cpuRegs.pc` e
registradores em memoria do processo; isso elimina miss por polling e permite
dumpar `[a1+0x30]` antes do estado avancar.
```

## 2. Avaliacao das abordagens

### Abordagem A - Polling externo via PINE

```txt
Veredito:
Inviavel como primario; util apenas como auxiliar.

Justificativa tecnica:
Polling externo depende de latencia IPC/socket e da frequencia de consulta. A
entrada em `0x001d27a8` pode durar uma unica instrucao antes de `pc` avancar,
entao a chance de miss e estrutural. Mesmo quando capturar `pc`, leituras
posteriores de memoria podem ja ocorrer depois do evento relevante.

Risco principal:
Falso negativo: concluir que a funcao nao executou quando o polling apenas
perdeu a janela.

Recomendacao:
Usar PINE para sanity checks, leitura de memoria fora do ponto critico,
patches pontuais e comparacao de estado, nao para detectar entrada exata.
```

### Abordagem B - Injecao de BREAK/SYSCALL via PINE

```txt
Veredito:
Viavel apenas para casos pontuais; nao recomendado como primario.

Justificativa tecnica:
Substituir a instrucao em `0x001d27a8` por BREAK/SYSCALL pode gerar uma pausa
sincronica, mas depende de como o PCSX2 expõe o evento externamente, de
restaurar a instrucao original corretamente e de invalidar cache/JIT quando o
Recompiler estiver ativo. Para um unico teste manual pode funcionar; para logs
repetiveis e multiplos hits, vira frágil.

Risco principal:
Interacao ruim com blocos recompilados, skip/restore incorreto e degradacao da
execucao quando o alvo e chamado muitas vezes.

Recomendacao:
Manter como fallback experimental para um endereco especifico se o hook interno
nao puder ser compilado a tempo.
```

### Abordagem C - Hook/logpoint interno minimo no PCSX2

```txt
Veredito:
Recomendado como primario para a POC.

Justificativa tecnica:
No EE Interpreter, `execI()` observa `cpuRegs.pc` antes de incrementar o PC e
executar a instrucao. Um teste `if (pc == 0x001d27a8)` nesse ponto captura a
entrada exata da funcao, com acesso direto a `cpuRegs.GPR.n.a0/a1/...` e a
memoria EE via helpers internos como `memRead32`/leituras seguras equivalentes.

Risco principal:
O Interpreter pode ser lento demais para navegar ate a cena relevante sem
savestate. Tambem ha risco de logging sincrono pesado travar a emulacao.

Recomendacao:
Mirar Interpreter primeiro, usar savestate antes da cena ou antes do fluxo que
dispara cloth, e escrever JSONL curto com dumps pequenos. Recompiler fica para
fase posterior se o Interpreter nao for praticavel.
```

### Abordagem D - Hibrido PINE + Hook interno

```txt
Veredito:
Arquitetura recomendada apos a POC.

Justificativa tecnica:
O hook interno deve ser a fonte primaria dos eventos sincronicos. PINE continua
valioso para inspecao e automacao de apoio. O analyzer Python fica responsavel
por transformar logs brutos em evidencia humana: SQLite, correlacao de `ra`,
classificacao de regioes e relatorio Markdown.

Risco principal:
Generalizar cedo demais e atrasar a prova minima.

Recomendacao:
Fase 1 hardcoded. So depois introduzir `probe_targets.json`, SQLite e relatorio
automatico.
```

## 3. Menor POC possivel

Tempo alvo: menos de 4 horas, assumindo um checkout compilavel do PCSX2.

| Item | Decisao |
|---|---|
| Savestate | Usar como pre-condicao pratica, carregado pouco antes de uma cena/fluxo que inicialize cloth |
| Endereco hardcoded | `0x001d27a8` |
| CPU | EE Interpreter primeiro |
| Registradores minimos | `pc`, `a0`, `a1`, `a2`, `a3`, `sp`, `ra` |
| Memoria minima | 128 bytes em `a1`, word `[a1+0x30]`, 64 bytes em `sp`, instrucoes em `ra-8`, `ra-4`, `ra` |
| Log minimo | JSONL em `.local/runtime-captures/<session>/events.jsonl` |
| Validacao positiva | JSONL contem hit com `pc=0x001d27a8`, registradores coerentes e dumps sincronicos |
| Validacao negativa | Savestate roda por janela definida sem hit; testar breakpoint manual/debugger no mesmo estado antes de concluir ausencia |

Teste minimo para separar falhas:

| Falha observada | Teste separador |
|---|---|
| Sem hits apos savestate | Confirmar com breakpoint manual em `0x001d27a8` no mesmo savestate |
| Breakpoint manual bate, logpoint nao | Hook esta no ponto errado ou nao foi compilado/ativado |
| Nem breakpoint manual bate | Savestate pode estar depois da inicializacao; criar savestate anterior |
| Bate em Recompiler mas nao em Interpreter | Testar boot direto em Interpreter e testar outro alvo conhecido |
| Valores de memoria inconsistentes | Mover dump para dentro do hook antes de qualquer continuacao externa |

O savestate deve restaurar registradores, memoria, PC, stack e contexto do EE
em condicao suficiente para a POC. A incerteza pratica nao e se o savestate
preserva memoria basica, mas se o ponto salvo ainda esta antes de uma chamada
futura a `0x001d27a8`.

## 4. Menor patch conceitual no PCSX2

Ponto de partida obrigatorio:

```txt
Arquivo candidato: pcsx2/Interpreter.cpp
Loop candidato: intExecute()
PC atual: cpuRegs.pc
Registradores: cpuRegs.GPR.n.<reg>.UL[0]
```

Essas ancoras continuam plausiveis no PCSX2 upstream atual. A funcao
`intExecute()` contem o loop do EE Interpreter; `execI()` e o ponto mais
especifico porque roda por instrucao e captura `const u32 pc = cpuRegs.pc`
antes de `cpuRegs.pc += 4`.

Arquivos reais/provaveis a investigar:

| Arquivo | Motivo |
|---|---|
| `pcsx2/Interpreter.cpp` | Hook da POC no EE Interpreter |
| `pcsx2/R5900.cpp` / headers associados | Definicao de `cpuRegs` e helpers de memoria |
| `pcsx2/DebugTools/Breakpoints.*` | Referencia para enderecos padronizados e pausa/debugger |
| `pcsx2/x86/iR5900.cpp` ou equivalente atual | Futuro hook no Recompiler, se necessario |

Ponto provavel do hook:

```cpp
static void execI()
{
    const u32 pc = cpuRegs.pc;

    IcoProbeMaybeLog(pc);

    cpuRegs.pc += 4;
    cpuRegs.code = memRead32(pc);
    const OPCODE& opcode = GetCurrentInstruction();
    ...
    opcode.interpret();
}
```

Pseudocodigo minimo:

```cpp
static bool IcoProbeRead32(u32 addr, u32* out)
{
    // Na POC, manter simples: validar faixa basica e usar helper interno.
    // Na versao reutilizavel, centralizar traducao/validacao de endereco EE.
    if (!out)
        return false;
    *out = memRead32(addr);
    return true;
}

static void IcoProbeMaybeLog(u32 pc)
{
    if (pc != 0x001d27a8)
        return;

    const u32 a0 = cpuRegs.GPR.n.a0.UL[0];
    const u32 a1 = cpuRegs.GPR.n.a1.UL[0];
    const u32 a2 = cpuRegs.GPR.n.a2.UL[0];
    const u32 a3 = cpuRegs.GPR.n.a3.UL[0];
    const u32 sp = cpuRegs.GPR.n.sp.UL[0];
    const u32 ra = cpuRegs.GPR.n.ra.UL[0];

    u32 a1_plus_30 = 0;
    const bool have_a1_30 = IcoProbeRead32(a1 + 0x30, &a1_plus_30);

    // Escrever uma linha JSONL curta, preferencialmente em buffer/ring buffer.
    // Dumps binarios pequenos podem ser hex no JSONL na POC.
}
```

Como ler memoria EE com seguranca:

- usar helpers internos do PCSX2 (`memRead8/16/32` ou equivalente atual);
- validar ranges basicos antes de dumps longos;
- em caso de excecao/invalidade, registrar erro no JSONL em vez de parar a VM;
- capturar `[a1+0x30]` dentro do hook, nao por processo externo depois.

Estrategia de logging:

- POC: abrir arquivo uma vez por sessao e escrever JSONL curto;
- evitar flush por hit quando o alvo puder disparar muito;
- limitar dumps a 64/128 bytes;
- se houver volume alto, usar ring buffer em memoria e drenar fora do hot path;
- nunca escrever dumps grandes sincronicamente em toda instrucao.

Interpreter primeiro. Recompiler depois apenas se o jogo nao for navegavel ou
se o savestate em Interpreter nao reproduzir o evento.

## 5. Dados a capturar

Para a POC em `0x001d27a8`:

| Campo | Obrigatorio | Motivo |
|---|---:|---|
| `session_id` | Sim | Agrupar logs |
| `timestamp_ns` | Sim | Ordenacao |
| `frame` | Se disponivel | Correlacao com execucao |
| `pc` | Sim | Confirmar alvo |
| `label` | Sim | `cloth_payload_init` / `cloth_initializer_candidate` |
| `a0`, `a1`, `a2`, `a3` | Sim | Argumentos |
| `sp`, `ra` | Sim | Stack e caller |
| `v0`, `v1` | Recomendado | Contexto de retorno/intermediarios |
| `s0`, `s3` | Recomendado | Rev.047 mostra descriptor/context relevantes no caller |
| `[a1+0x30]` | Sim, se valido | Variant copiado para payload |
| dump `a1+0x00..0x7f` | Sim | Initializer stack struct |
| dump `sp+0x00..0x3f` | Sim | Verificar `a1 == sp` e contexto de stack |
| instrucao `ra-8` | Sim | Provavel `jal`/`jalr` caller |
| instrucao `ra-4` | Sim | Delay slot |
| instrucao `ra` | Sim | Pos-call |

Capturar so `a1` nao e suficiente para evidencia forte. O valor de
`[a1+0x30]` e o dump ao redor de `a1` precisam ser sincronicos com o hit.

## 6. Como transformar em ferramenta reutilizavel

Evolucao recomendada:

```txt
POC hardcoded -> probe_targets.json -> JSONL -> SQLite -> relatorio Markdown
```

Organizacao sugerida:

| Caminho | Conteudo |
|---|---|
| `.local/runtime-captures/<session>/events.jsonl` | Log bruto, ignorado pelo git |
| `.local/runtime-captures/<session>/dumps/` | Dumps binarios opcionais, ignorados pelo git |
| `tools/runtime-probe-analyzer/` | Importador JSONL -> SQLite e Markdown |
| `research/runtime/*.md` | Planos e resultados documentaveis |

`probe_targets.json` futuro:

```json
[
  {
    "addr": "0x001d27a8",
    "label": "cloth_payload_init",
    "subsystem": "cloth",
    "regs": ["a0", "a1", "a2", "a3", "sp", "ra", "v0", "v1", "s0", "s3"],
    "memory": [
      {"base": "a1", "offset": 0, "size": 128, "purpose": "initializer_stack"},
      {"base": "a1", "offset": 48, "size": 4, "purpose": "variant_word"},
      {"base": "sp", "offset": 0, "size": 64, "purpose": "stack_context"}
    ]
  }
]
```

SQLite e adequado para fase 2/3. Manter JSONL bruto sempre, porque ele e
auditavel e simples de arquivar localmente. Dumps devem ficar em tabela separada
ou arquivos com indice; inserir em batch e usar WAL quando o importador crescer.

Schema-base:

| Tabela | Papel |
|---|---|
| `probe_events` | Uma linha por hit |
| `probe_memory_dumps` | Dumps associados por `event_id` |
| `probe_sessions` | Metadados: PCSX2 commit, modo CPU, jogo, savestate, alvo |

## 7. Riscos restantes

| Risco | Impacto | Mitigacao |
|---|---|---|
| Build do PCSX2 no Linux/Ubuntu 24 falhar | Atrasa POC | Primeiro compilar upstream sem patch; depois aplicar diff minimo |
| Interpreter lento demais | Nao chegar ao evento manualmente | Usar savestate antes do evento |
| Savestate salvo depois da inicializacao | Nenhum hit | Criar savestate anterior; testar breakpoint manual |
| Diferenca Interpreter/Recompiler | Hit some em um modo | Testar alvo conhecido; considerar hook no Recompiler |
| Logging sincrono pesado | Stutter/travamento | JSONL curto, buffering, dumps pequenos |
| Enderecos virtuais vs fisicos | Hook nunca bate | Confirmar convencao `0x001d27a8` no PCSX2/debugger |
| BREAK/JIT invalidation | Instabilidade no fallback B | Evitar como primario |
| Ponteiros para heap/stack sem tipo | Interpretacao fraca | Classificar regiao e preservar dump bruto |
| Caller via `ra` impreciso | Chamada indireta/tail call | Registrar `ra-8`, `ra-4`, `ra`; tratar como provavel |
| Delay slots | Caller mal identificado | Analyzer deve conhecer `ra = jal_addr + 8` |
| `jalr`/callbacks indiretos | `ra-8` nao revela alvo carregado | Capturar registradores no caller quando necessario |
| MTVU/VU1 | Confusao de dominio | `0x001d27a8` e ELF MIPS EE; hook EE basta para esse alvo |
| ICO USA-specific | Logs nao portaveis | Registrar build `SCUS_971.13` e hash local fora do git |

Sobre MTVU/VU1: o alvo `0x001d27a8` vem do ELF MIPS principal, entao e codigo
EE/R5900. MTVU pode afetar temporizacao de microcode VU1, mas nao deve impedir
um hook EE de capturar a entrada nessa funcao. Instrumentar VU1 so vira
necessario se o alvo futuro for microcode ou estado produzido exclusivamente na
VU1 sem reflexo imediato em memoria EE.

## 8. Veredito final

```txt
Eu implementaria primeiro:
Um hook hardcoded no EE Interpreter em `pcsx2/Interpreter.cpp`, no caminho
`execI()`, logando quando `pc == 0x001d27a8`.

Eu evitaria primeiro:
Polling de PC via PINE como detector primario e BREAK/SYSCALL auto-restore como
pipeline de logs.

O sucesso minimo seria:
Gerar automaticamente um JSONL com `pc=0x001d27a8`, `a0/a1/a2/a3/sp/ra`,
`[a1+0x30]`, dump de 128 bytes de `a1` e instrucoes em `ra-8/ra-4/ra`.

O proximo passo apos sucesso seria:
Transformar o alvo hardcoded em `probe_targets.json`, importar JSONL para
SQLite e expandir para `0x001b7a74`, `0x0013f7a8`, `0x001d37c8` e os demais
alvos do cluster cloth.
```

## Conservative Verdict

O melhor caminho para logs automatizados e assertivos e um hook interno minimo
no PCSX2, nao PINE como detector. A Rev.046/047 ja resolveu o primeiro `a1`
manual, mas a POC continua valiosa porque transforma essa captura em evidencia
repetivel e escalavel para os proximos alvos do sistema cloth.

## Implementation Status

### Created after this evaluation

| Artefato | Estado |
|---|---|
| `/tmp/pcsx2-ico-logpoints` | Checkout local temporario do PCSX2 com hook aplicado |
| `research/runtime/pcsx2-ico-probe-poc.patch` | Patch aplicavel ao PCSX2 upstream para a POC hardcoded |
| `tools/runtime-probe-analyzer/runtime_probe_analyzer.py` | Parser/monitor JSONL inicial |
| `tools/runtime-probe-analyzer/README.md` | Uso minimo do parser |

### Current build blocker

Resolvido em 2026-05-16. O PCSX2 patched compilou com sucesso em:

```sh
cd /tmp/pcsx2-ico-logpoints
cmake --preset clang-devel -DUSE_BACKTRACE=OFF
cmake --build build --config Devel
```

Binario gerado:

```txt
/tmp/pcsx2-ico-logpoints/build/bin/pcsx2-qt
```

Observacoes:

- `Interpreter.cpp` compilou com o hook aplicado.
- O link final de `bin/pcsx2-qt` terminou sem erro.
- FFmpeg nao foi instalado por conflito de pacotes no Nobara; o CMake usou
  headers bundled, conforme fallback upstream.
- `USE_BACKTRACE=OFF` foi usado porque `libbacktrace-devel` nao estava
  disponivel como pacote direto.

### First runtime command

Para iniciar uma sessao de captura:

```sh
mkdir -p /home/peter/Documentos/repos/ico-reconstruction/.local/runtime-captures/ico-probe

ICO_PCSX2_PROBE_LOG=/home/peter/Documentos/repos/ico-reconstruction/.local/runtime-captures/ico-probe/events.jsonl \
  /tmp/pcsx2-ico-logpoints/build/bin/pcsx2-qt
```

Em outro terminal, monitorar os hits:

```sh
cd /home/peter/Documentos/repos/ico-reconstruction
python3 tools/runtime-probe-analyzer/runtime_probe_analyzer.py \
  --input .local/runtime-captures/ico-probe/events.jsonl \
  --follow
```

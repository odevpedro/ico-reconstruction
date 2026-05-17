# rev.065 — Live Dispatch Follow-up Plan

## Resumo executivo

A Rev.064 desloca o modelo de dispatch vivo que antes estava associado a `0x001B76F8` / `0x001B7D00` para o conjunto:

```txt
0x00167230 / 0x00167258 / 0x00167270 -> 0x00166E10
```

Essa correção é estrutural: `0x00167230` não deve ser analisado como função independente, e `0x00166E10` passa a ser o melhor candidato atual para o corpo principal vivo de iteração/dispatch de inicialização de cena ou entidade. A interpretação antiga da Rev.059 deve ser preservada como histórico, mas substituída neste ponto específico por um modelo conservador: `0x001B76F8` e `0x001B7D00` estão dead/unreached under current evidence, enquanto `0x00166E10` e seus cold paths exigem validação runtime direta.

## Fonte de verdade atual

Arquivos mais confiáveis para este ponto do projeto:

| Arquivo | Papel atual |
|---|---|
| `research/elf/ghidra-rev064-cold-paths-and-live-dispatch.md` | Fonte primária da correção: cold paths, `0x00166E10`, `gp-25904`, `0x006AAC00` |
| `docs/data-model.md` | Modelo de dados vivo com `scene_init_context`, `desc_array_0x006AAC00` e default state |
| `docs/backlog.md` | Registro operacional de que Rev.064 supersede o modelo Rev.059 para este ponto |

A Rev.059 deve ser tratada como histórica/superseded para a afirmação de que `0x001B76F8` é o iterador real de descritores e `0x001B7D00` é o scene loader vivo. Ela não deve ser apagada, porque registra o caminho de análise que levou ao erro e ainda pode conter evidência útil em outros tópicos, desde que revalidada.

## Correção principal da Rev.064

`0x00167230 is not a standalone function. It is a compiler-split cold path that rejoins the main body at 0x00166E10.`

Em português: `0x00167230` não é uma função independente. É um cold path gerado pelo compilador que salta para o corpo principal em `0x00166E10`.

O erro anterior era tratar um endereço de entrada frio como se fosse a unidade funcional completa. A Rev.064 mostra que a unidade real precisa incluir o corpo principal em `0x00166E10` e os cold paths que preparam campos antes de saltar para esse corpo.

## Cold paths identificados

| Cold path | GP slot associado | Comportamento observado | Destino |
|---|---:|---|---|
| `0x00167230` | `gp-25856` | limpa +176/+148/+136, carrega default state, muitos JALRs | `0x00166E10` |
| `0x00167258` | `gp-25852` | limpa +148, menos JALRs | `0x00166E10` |
| `0x00167270` | `gp-25848?` | terceiro path, precisa validar | `0x00166E10?` |

O padrão provável é de entradas especializadas por estado inicial. Elas compartilham o mesmo corpo principal, mas diferem nos campos do contexto que são limpos antes da entrada comum.

## Função principal `0x00166E10`

`0x00166E10` é o melhor candidato atual para live iterator/dispatch principal. A Rev.064 descreve um corpo grande, com stack frame de cerca de 400 bytes e mais de 250 instruções, que itera o array em `0x006AAC00`, acessa campos do contexto e executa callback indireto por `JALR`.

Isso ainda não deve ser chamado de confirmação runtime total. A evidência estática e estrutural é forte, mas o próximo passo precisa capturar hits em runtime para confirmar que:

| Item | O que confirmar |
|---|---|
| Entrada | cold paths realmente batem em sessão relevante |
| Corpo comum | os cold paths saltam para `0x00166E10` |
| Dados | `0x00166E10` processa descriptors reais de `0x006AAC00` |
| Dispatch | `0x00167020` executa `JALR` para callbacks vivos |

## Por que `0x00166E10` não aparece como caller estático

`0x00166E10` pode não ter referências estáticas diretas porque não é chamado por `JAL` externo. O modelo atual é:

```txt
caller externo
-> carrega ponteiro de função em GP slot
-> JALR para cold path
-> cold path prepara estado
-> J para 0x00166E10
```

Nesse padrão, a referência funcional externa aponta para `0x00167230`, `0x00167258` ou possivelmente `0x00167270`, não para `0x00166E10`. O corpo principal aparece como destino de `J` interno vindo dos cold paths, então buscas por `JAL 0x00166E10` ou ponteiros diretos para `0x00166E10` podem retornar zero sem significar que o corpo é morto.

## Estado padrão em `gp-25904`

| Campo | Valor |
|---|---|
| Address | `0x006323C0` |
| GP slot | `gp-25904` |
| Bytes | `00 00 00 00 FF FF FF FF` |
| Interpretação provisória | null pointer + `-1` flag |
| Modo de carga | par não alinhado `LDL` / `LDR` |

A interpretação provisória é que os 8 bytes representam dois campos de 32 bits: um ponteiro nulo e uma flag ou índice `-1`. A Rev.064 indica que esses bytes são carregados via `LDL` / `LDR` e armazenados em campos do contexto por stores não alinhados.

Perguntas mínimas para runtime:

| Pergunta | Evidência necessária |
|---|---|
| Esse estado é copiado para campos do contexto? | dump de `context +0x00..+0xB0` antes/depois do cold path |
| É usado para reset de descriptor? | correlação entre cold path, descriptor atual e campos +0x80/+0x84 |
| É usado para invalidar/limpar callback state? | observar callback target antes/depois e campos limpos +0x88..+0x98 |

## Modelo de dados relacionado

A Rev.064 introduziu ou consolidou estes elementos:

| Elemento | Endereço/base | Estado atual |
|---|---:|---|
| `scene_init_context` | `a0` dos cold paths / `0x00166E10` | struct operacional em validação |
| `desc_array_0x006AAC00` | `0x006AAC00` | array iterado pelo candidato vivo |
| sub-struct em `+0x15C` | `[descriptor + 0x15C]` | subestrutura acessada pelo fluxo |
| default state | `gp-25904` / `0x006323C0` | bytes confirmados, semântica provisória |

Mapa mínimo de offsets que precisa ser preservado na próxima captura:

| Área | Faixa/campos |
|---|---|
| Context | `+0x00..+0xB0`, com atenção a `+0x0C`, `+0x80`, `+0x84`, `+0x88`, `+0x98`, `+0xB0` |
| Descriptor array | base `0x006AAC00`, índice atual, ponteiro do descriptor |
| Descriptor | `+0x00`, `+0x08`, `+0x0C`, `+0x70`, `+0x74`, `+0x78`, `+0x80`, `+0x15C` |
| Sub-struct | campos lidos a partir de `[descriptor + 0x15C]`, especialmente `+0x08`, `+0x0C`, `+0x70..+0x80` |

## Função utilitária `0x00105F00`

Tratar `0x00105F00` como `atomic_or_synchronization_candidate`.

Motivos:

| Evidência | Interpretação conservadora |
|---|---|
| Cerca de 665 `JALs` | uso amplo em `.text`, não específico deste pipeline |
| Uso de `LL` / Load Linked | provável operação atômica, lock-free ou sincronização |
| Chamada múltipla dentro de `0x00166E10` | utilitário usado pelo fluxo, não prova de semântica scene-specific |

Não renomear definitivamente ainda. O nome provisório deve lembrar que a evidência forte é instrucional (`LL`) e de frequência de chamadas, não semântica final.

## O que a Rev.059 deixou de explicar

A Rev.059 tentou tratar `0x001B76F8` como iterador real de descritores e `0x001B7D00` como scene loader de quatro fases. A Rev.064 mostra que esse modelo não explica o live dispatch atual porque o caminho estruturalmente ativo passa por GP slots que entram em cold paths e convergem em `0x00166E10`.

O ponto fraco da Rev.059, agora visível, é que ela elevou um caminho estático plausível para papel operacional sem prova runtime suficiente. A substituição correta não é apagar a Rev.059, mas marcar esse ponto como superseded e exigir validação runtime do novo candidato.

## Como tratar `0x001B76F8` e `0x001B7D00`

Usar a linguagem:

```txt
dead/unreached under current evidence
```

Evitar:

```txt
definitivamente morto
```

Estado recomendado:

| Endereço | Tratamento atual |
|---|---|
| `0x001B76F8` | modelo de iterador real superseded; dead/unreached under current evidence |
| `0x001B7D00` | modelo de scene loader vivo superseded; dead/unreached under current evidence |
| Rev.059 | manter como histórico, com nota de superseded para este ponto |

Se runtime futuro mostrar hits reais em `0x001B76F8`, o modelo deve ser reaberto sem assumir automaticamente que a Rev.059 inteira estava correta. O critério será o fluxo observado, os argumentos, os descriptors processados e a relação com `0x00166E10`.

## Hipótese atual de live dispatch

Hipótese operacional:

```txt
GP function pointer slot
-> cold path específico
-> limpeza/reset parcial do context
-> cópia do default state de gp-25904
-> salto para 0x00166E10
-> iteração de desc_array_0x006AAC00
-> dispatch por callback indireto em 0x00167020
```

Classificação:

| Parte | Confiança |
|---|---|
| Cold paths saltam para corpo comum | forte por disassembly |
| `gp-25904` contém `00 00 00 00 FF FF FF FF` | confirmado por bytes |
| `0x00166E10` é live iterator/dispatch principal | melhor candidato atual |
| `0x00167020` é callback vivo em runtime | precisa captura runtime |
| `0x001B76F8` nunca participa | não confirmado; apenas dead/unreached under current evidence |

## Plano de runtime probe

Não executar runtime nesta revisão. O objetivo é preparar uma captura pequena, auditável e comparável.

Prioridades:

| Alvo | Objetivo |
|---|---|
| `0x00167230` | capturar `a0` como context pointer, confirmar cold path ativado, capturar campos limpos `+176`, `+148`, `+136` |
| `0x00167258` | capturar `a0`, confirmar se limpa apenas `+148`, comparar com `0x00167230` |
| `0x00167270` | validar se é terceiro cold path real, capturar GP slot associado |
| `0x00166E10` | capturar `a0`, `a1`, `s2`, índice de array, descriptor pointer, confirmar corpo principal vivo |
| `0x00167020` | capturar `JALR` callback, callback target, argumentos e `RA`/caller |

O probe deve registrar poucos eventos por hit, mas com dumps estruturais suficientes para reconstituir o caminho. Se o volume ficar alto, priorizar `0x00167020` filtrado por callback target e os cold paths.

## Breakpoints prioritários

| Prioridade | PC | Motivo |
|---:|---:|---|
| 1 | `0x00167230` | cold path primário observado em `gp-25856`, muitos `JALRs` |
| 2 | `0x00167258` | cold path secundário observado em `gp-25852` |
| 3 | `0x00166E10` | corpo principal candidato a live iterator/dispatch |
| 4 | `0x00167020` | ponto provável de `JALR` callback vivo |
| 5 | `0x00167270` | terceiro cold path provável, precisa validação |

Campos específicos por breakpoint:

| PC | Captura mínima |
|---|---|
| `0x00167230` | `a0`, `ra`, `gp`, dump `context +0x00..+0xB0`, bytes `gp-25904` |
| `0x00167258` | `a0`, `ra`, `gp`, dump `context +0x00..+0xB0`, comparação de `+0x94/+0x98/+0xB0` |
| `0x00167270` | `a0`, `ra`, `gp`, GP slot de origem se recuperável, destino efetivo |
| `0x00166E10` | `a0`, `a1`, `s0-s7`, índice de array, `descriptor_ptr`, `descriptor_array_base` |
| `0x00167020` | callback target, `a0-a3`, `ra`, caller, descriptor atual, contexto atual |

## Campos obrigatórios para captura

| Campo | Observação |
|---|---|
| `hit_id` | contador monotônico local |
| `timestamp/frame` | frame, ciclo ou timestamp do probe |
| `pc` | endereço do breakpoint/logpoint |
| `ra` | retorno; usar `ra - 8` com cautela para `jal`, não para todo caso |
| `sp` | stack pointer |
| `gp` | confirmar base `0x006388F0` em sessão |
| `a0-a3` | argumentos imediatos |
| `v0-v1` | retornos/intermediários |
| `s0-s7` | estado salvo do corpo principal |
| `t0-t9` | temporários relevantes para índice/callback |
| `cold_path_id` | `0x00167230`, `0x00167258`, `0x00167270` ou none |
| `context_ptr` | provável `scene_init_context` |
| `descriptor_array_base` | esperado `0x006AAC00` |
| `descriptor_index` | índice ativo no loop |
| `descriptor_ptr` | ponteiro de descriptor atual |
| `callback_target` | alvo efetivo de `JALR` |
| `callback_args` | argumentos passados ao callback |
| `memory dump context +0x00..+0xB0` | antes/depois se possível nos cold paths |
| `memory dump descriptor +0x00..+0x180` | cobre campos até `+0x15C` e margem |
| `gp-25904 bytes` | esperado `00 00 00 00 FF FF FF FF` |

## Critérios de confirmação

Confirmar a hipótese atual se:

| Critério | Evidência esperada |
|---|---|
| Cold paths batem em runtime | hits em `0x00167230`, `0x00167258` e/ou `0x00167270` durante sessão relevante |
| Todos saltam para `0x00166E10` | sequência observada ou `RA`/controle compatível com entrada no corpo comum |
| `0x00166E10` processa descriptors reais | `descriptor_array_base == 0x006AAC00`, índice plausível, descriptor pointer válido |
| `0x00167020` faz `JALR` para callbacks vivos | callback target em `.text`, argumentos coerentes, retorno observado |
| `a0` ou outro registrador aponta para `scene_init_context` | dump de contexto mostra campos esperados e alterações dos cold paths |
| `desc_array_0x006AAC00` aparece no fluxo | base ou ponteiros derivados aparecem nos registradores/memória |

## Critérios de descarte

Enfraquecer ou descartar a hipótese se:

| Critério | Consequência |
|---|---|
| Breakpoints nunca batem em sessão relevante | reduzir confiança de live path; testar cobertura da sessão antes de concluir |
| `0x00166E10` não processa descriptors | reclassificar como init auxiliar ou outro corpo comum |
| `0x00167020` não é `JALR` de callback | revisar leitura instrucional e offsets |
| Cold paths não compartilham contexto | abandonar hipótese de entradas especializadas para mesmo corpo operacional |
| `0x001B76F8` volta a aparecer em runtime como caminho real | reabrir Rev.059 parcialmente, comparando fluxo com `0x00166E10` |

Ausência de hits em uma sessão curta não é descarte definitivo. Ela deve ser registrada como cobertura insuficiente ou evidência negativa limitada.

## O que atualizar depois do runtime

Atualizar somente depois de evidência runtime:

| Arquivo/área | Atualização possível |
|---|---|
| `docs/data-model.md` | consolidar `scene_init_context`, `desc_array_0x006AAC00`, offsets confirmados por dumps |
| `docs/backlog.md` | mover probe para concluído ou abrir próximo item mínimo |
| `AGENTS.md` | atualizar fonte de verdade operacional se Rev.065 + runtime supersederem Rev.064 |
| `src/entity/structs.h` | apenas se a validação justificar mudança de struct; não nesta tarefa |
| `research/elf/ghidra-rev059-table-reader-and-callback-chain.md` | adicionar nota de superseded para o ponto `0x001B76F8`/`0x001B7D00`, sem apagar histórico |
| runtime guide/probe patch | adicionar alvos `0x00167230`, `0x00167258`, `0x00167270`, `0x00166E10`, `0x00167020` |

Também revisar documentos secundários que ainda tratem `0x001B76F8` como iterador real, mas somente após captura runtime ou por nota explícita de superseded.

## Riscos

| Risco | Mitigação |
|---|---|
| Repetir o erro da Rev.059, promovendo hipótese estática a conclusão | manter `0x00166E10` como melhor candidato atual até runtime |
| Logpoint gerar ruído excessivo | começar com cold paths e `0x00167020`; filtrar por frame/callback se necessário |
| Interpretar `RA` errado em contexto de `J`, `JALR` e delay slot | registrar `pc`, `ra`, instrução e caller provável separadamente |
| Confundir default state com semântica final | registrar bytes e efeitos nos campos, sem nomear gameplay |
| Tratar `0x001B76F8` como definitivamente morto | usar dead/unreached under current evidence |
| Atualizar `src/` prematuramente | aguardar dumps runtime e confirmação estrutural |

## Veredito

A Rev.064 corrige um erro estrutural importante: o projeto não deve mais tratar `0x00167230` como função standalone nem usar `0x001B76F8` / `0x001B7D00` como modelo operacional atual de live dispatch sem nova evidência.

O modelo operacional provisório é que os cold paths `0x00167230`, `0x00167258` e possivelmente `0x00167270` preparam/resetam partes de um `scene_init_context` e convergem em `0x00166E10`, que é o melhor candidato atual para o corpo principal de iteração/dispatch sobre `desc_array_0x006AAC00`. A confirmação mínima agora é runtime: capturar os cold paths, o corpo comum, o `JALR` em `0x00167020`, os campos do contexto e os descriptors processados.

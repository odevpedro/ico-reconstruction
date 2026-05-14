# Term Audit Report — Termos Antigos que Precisam de Cuidado

Gerado em 2026-05-14. Baseado nas correções Rev.039 e Rev.040.

## Regra

Termos como "entity state", "Yorda", "capture", "AI state", "iosThreadStart" foram usados em notas antigas
(Rev.001-037) como hipóteses legítimas para a época. A partir de Rev.039, o domínio foi corrigido para cloth
physics. Notas antigas **não devem ser reescritas** — elas registram a evolução do pensamento. Mas ao citá-las,
é necessário indicar que aquelas interpretações foram descartadas.

## Ocorrências por arquivo

### `research/elf/ghidra-rev021-continue-menu-pivot.md`
- Yorda/capture/shadow: 5 ocorrências (linhas 153-244)
- entity state dispatcher: 4 ocorrências (linhas 49-178)
- **Cuidado**: este arquivo foi onde o dispatcher 0x001d37c8 foi identificado pela primeira vez. Os nomes "entity
  state dispatcher" são históricos e devem ser tratados como hipótese inicial, não conclusão.

### `research/elf/ghidra-rev024-internal-state-block-semantics.md`
- Yorda/capture/menu/death: 2 ocorrências (linhas 33, 709)
- entity_state_block: 4 ocorrências (linhas 401-612)
- **Cuidado**: a nomenclatura `entity_state_block` está no decompilado Ghidra. Não reescrever o .md, mas ao citar
  este arquivo, indicar que o domínio foi corrigido para cloth.

### `research/elf/ghidra-rev039-cloth-domain-correction.md`
- Yorda/capture/menu/death/AI: 4 ocorrências (linhas 45, 95, 136)
- **OK**: este é o próprio arquivo de correção. As menções estão no contexto de "descartado".

### `research/elf/ghidra-rev040-static-cloth-domain-reinterpretation.md`
- Yorda/capture/menu/death/AI: 2 ocorrências (linhas 368, 437)
- iosThreadStart: 7 ocorrências (linhas 84-97, 381, 399, 439)
- **OK**: este é o arquivo de reinterpretação. As menções estão no contexto de correção/descarte.

### `research/ico-decomp-cross-reference-2026-05-14.md`
- entity state: 1 ocorrência (linha 188)
- iosThreadStart: 2 ocorrências (linhas 43-44)
- **Cuidado**: a correção de "entity state" para "cloth physics" está no texto. `iosThreadStart` como símbolo
  vizinho é factual, mas não implica equivalência funcional.

### `docs/architecture-log.md`
- Yorda: 1 ocorrência (linha 26)
- entity state: 1 ocorrência (linha 782)
- **Cuidado**: documentos de arquitetura antigos. As referências a Yorda/entity state são sobre o escopo geral
  do jogo, não sobre funções específicas. Não precisam de correção imediata.

### `docs/architectural-analysis-a-d.md`
- Yorda behavior: 1 ocorrência (linha 31)
- **Cuidado**: análise arquitetural de alto nível. A referência a Yorda é sobre o subsistema de IA, não sobre
  as funções em clothAnimation.c. Não precisa de correção.

### `docs/backlog.md`
- entity/AI state: 1 ocorrência (linha 39)
- iosThreadStart: 1 ocorrência (linha 25)
- **OK**: ambos no contexto da correção Rev.039-040.

## O que NÃO fazer

- Não reescrever Rev.021, Rev.024 ou qualquer nota antiga.
- Não apagar menções a Yorda/capture/menu — elas são registro histórico legítimo.
- Não renomear nomenclatura Ghidra em notas antigas.

## O que fazer ao citar notas antigas

Incluir um disclaimer como:

> "Esta nota foi escrita antes da correção de domínio Rev.039. O dispatcher 0x001d37c8 era tratado como entity
> state machine na época, mas desde Rev.039 sabe-se que ele reside em clothAnimation.c (simulação de pano).
> As referências a Yorda/capture/menu devem ser lidas como hipóteses históricas descartadas."

# Codex Task — Rev.024 Internal State Block Semantics

Leia o `AGENTS.md` primeiro.

Dê continuidade ao projeto `ico-reconstruction` a partir do estado atual.

A prioridade é executar a próxima etapa:

```txt
Rev.024 — Internal State Block Semantics
```

Crie ou atualize somente este arquivo:

```txt
research/elf/ghidra-rev024-internal-state-block-semantics.md
```

Use como fontes principais:

- `research/elf/ghidra-rev023-dispatcher-table-resolution.md`
- `research/elf/ghidra-rev022-dispatcher-ground-truth.md`
- `research/elf/ghidra-rev021-continue-menu-pivot.md`
- `research/elf/ghidra-rev018-state-transition-dispatch.md`
- `research/elf/ghidra-rev019-state-resolver-caller-context.md`
- `.local/key-concepts.md`, se existir
- `.local/ai-context.md`, se existir

---

## Objetivo

Analisar os cinco basic blocks internos alcançados pelo dispatcher confirmado em `0x001d37c8`:

```txt
state 0 -> 0x001d3818
state 1 -> 0x001d3844
state 2 -> 0x001d391c
state 3 -> 0x001d39e0
state 4 -> 0x001d3a10
```

Para cada bloco, identifique:

- endereço inicial;
- endereço final estimado;
- instruções principais;
- chamadas diretas;
- chamadas indiretas;
- loads relevantes;
- stores relevantes;
- offsets acessados;
- constantes;
- branches;
- possível alteração de `candidate_state_id`;
- acesso a `candidate_state_block_ptr`;
- acesso a entity/context;
- se retorna ao epílogo comum;
- papel semântico provável;
- nível de confiança.

---

## Restrições

Não investigue nesta tarefa:

- `DATA.DF`;
- `.gcm`;
- Yorda;
- capture;
- shadow strings;
- TM2;
- Continue menu;
- overlay extraction;
- unrelated asset formats;
- broad ISO scanning.

Não use nomes fortes como:

- “Yorda state”;
- “capture state”;
- “continue state”;
- “menu state”;
- “death state”;
- “final state”.

Use esses nomes somente se houver evidência direta.

Separe claramente:

- confirmado;
- provável;
- possível;
- desconhecido;
- precisa de runtime.

Não escreva fora do arquivo solicitado.

---

## Formato obrigatório do arquivo

```md
# rev.024 — Internal State Block Semantics

## Resumo executivo

## Escopo

## Fontes usadas

## Dispatcher confirmado

## State table

## State 0 — `0x001d3818`

## State 1 — `0x001d3844`

## State 2 — `0x001d391c`

## State 3 — `0x001d39e0`

## State 4 — `0x001d3a10`

## Tabela comparativa dos estados

## Writes de estado encontrados

## Chamadas externas relevantes

## Pseudocódigo conservador

## O que fica confirmado

## O que fica provável

## O que fica possível

## O que ainda precisa de runtime

## Próximo teste mínimo recomendado

## Veredito
```

---

## Critério de sucesso

A tarefa é bem-sucedida se o arquivo `research/elf/ghidra-rev024-internal-state-block-semantics.md` permitir responder, com cautela:

```txt
O que cada um dos 5 estados internos parece fazer?
```

Mesmo que a resposta ainda seja parcial, ela deve deixar claro:

- quais evidências existem;
- quais interpretações são prováveis;
- quais pontos continuam desconhecidos;
- quais testes mínimos vêm depois.

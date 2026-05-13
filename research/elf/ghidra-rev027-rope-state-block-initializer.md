# rev.027 — ROPE State Block Initializer

## Data

2026-05-13

## Objetivo

Analisar estaticamente o callback `0x001d27a8`, apontado pelo slot `+0x48` do record `.data` rotulado `ROPE`, para confirmar como ele inicializa o ponteiro usado depois por `0x001d3a30` e pelo dispatcher `0x001d37c8`.

## Escopo

Incluído:

- callback `0x001d27a8`;
- record `ROPE` de Rev.026;
- escrita em `[entity + 0x800]`;
- template copiado de `0x004c46b0`;
- campos do state block que se conectam a Rev.024.

Excluído:

- runtime adicional;
- gameplay;
- assets e `DATA.DF`;
- semântica visual/física do label `ROPE`;
- análise completa das chamadas externas.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `AGENTS.md` | regras de cautela |
| `.local/key-concepts.md` | orientação conceitual |
| `research/elf/ghidra-rev024-internal-state-block-semantics.md` | layout do dispatcher/state block |
| `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | caller `0x001d3a30` |
| `research/elf/ghidra-rev026-rope-record-table-context.md` | record `ROPE` e slot `+0x48` |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |

## Resumo

O callback `0x001d27a8` instala diretamente o ponteiro usado pela máquina de estados:

```txt
[context + 0x15c] -> entity
[entity + 0x800] = provided/copied_state_block
```

Ele obtém/resolve um bloco via chamada a `0x0013a0f8`, copia um template de `0x90` bytes a partir de `0x004c46b0`, e depois grava o ponteiro retornado em `[entity + 0x800]`.

Isso fecha uma cadeia estática importante:

```txt
ROPE record +0x48
-> 0x001d27a8
-> writes [entity + 0x800]
-> later 0x001d3a30 reads [entity + 0x800]
-> 0x001d3a30 may call 0x001d37c8
-> 0x001d37c8 dispatches on [state_block + 0x48]
```

## Record `ROPE` relevante

De Rev.026:

| Record offset | VA | Valor | Papel observado |
|---:|---|---:|---|
| `+0x38` | `0x002a396c` | `0x001d3b28` | callback relacionado, papel exato pendente |
| `+0x40` | `0x002a3974` | `0x001d3a30` | callback runtime-confirmado que pode chamar o dispatcher |
| `+0x48` | `0x002a397c` | `0x001d27a8` | callback analisado nesta revisão |
| `+0x54` | `0x002a3988` | `ROPE` | label interno do record |

## Instruções-chave de `0x001d27a8`

| VA | Instrução | Interpretação conservadora |
|---|---|---|
| `0x001d27e4` | `lw a0,-0x68e0(gp)` | argumento para chamada de obtenção/resolução |
| `0x001d27e8` | `jal 0x0013a0f8` | chamada que retorna ponteiro em `$v0` |
| `0x001d27ec` | `lw s0,0x15c(s5)` | delay slot: lê entity/context interno |
| `0x001d27f0` | `daddu s6,v0,zero` | preserva ponteiro retornado como state block candidato |
| `0x001d27f4` | `lw v1,0x15c(s5)` | relê entity/context interno |
| `0x001d27fc` | `addiu v0,v0,0x46b0` | prepara fonte do template em `0x004c46b0` |
| `0x001d2804` | `sw s6,0x800(v1)` | instala state block em `[entity + 0x800]` |
| `0x001d2808` | `addiu v1,v0,0x80` | fim parcial do loop de cópia |
| `0x001d280c..0x001d283c` | sequência `ld/sd` com incremento de `0x20` | copia `0x80` bytes do template |
| `0x001d2840..0x001d284c` | `ld/ld/sd/sd` | copia mais `0x10` bytes |
| `0x001d2850` | `lw v1,0x30(s4)` | lê campo de argumento externo |
| `0x001d2858` | `sw v1,0x04(s6)` | inicializa `[state_block + 0x04]` |
| `0x001d2944` | `sw s0,0x40(s3)` | escreve ponteiro em `[state_block + 0x40]` |
| `0x001d2978` | `swc1 f1,0x44(s3)` | escreve valor calculado em `[state_block + 0x44]` |

## Tamanho e fonte do template

O callback copia:

```txt
template source = 0x004c46b0
copy size       = 0x90 bytes
destination     = pointer returned by 0x0013a0f8
```

Evidência:

| Instruções | Efeito |
|---|---|
| `v0 = 0x004c46b0` | fonte do template |
| `v1 = v0 + 0x80` | limite do loop principal |
| loop `ld/sd` em blocos de `0x20` | copia `0x80` bytes |
| cópia final de dois `ld` | copia mais `0x10` bytes |

Total:

```txt
0x80 + 0x10 = 0x90
```

## Campos relevantes do template `0x004c46b0`

Valores observados nos offsets que se conectam diretamente a Rev.024:

| Offset | Valor inicial no template | Relação com Rev.024 |
|---:|---:|---|
| `+0x04` | `0x00000000` | sobrescrito por `0x001d2858` com valor de `[arg + 0x30]` |
| `+0x2c` | `0x3f800000` | float `1.0`; significado pendente |
| `+0x40` | `0x00000000` | sobrescrito por `0x001d2944` |
| `+0x44` | `0x0000012c` | campo temporizador/contador usado por `state_1_block` em Rev.024 |
| `+0x48` | `0x00000000` | `candidate_state_id` inicial; Rev.024 lê este campo como state id |
| `+0x60` | `0x00000000` | campo usado por estados posteriores em Rev.024 |
| `+0x64` | `0x00000000` | campo escrito por `state_0_block` em Rev.024 |

Interpretação conservadora:

- o template inicializa `candidate_state_id` como `0`;
- o valor `0x12c` em `+0x44` é consistente com o campo decrementado por `state_1_block`;
- a função `0x001d27a8` conecta diretamente o record `ROPE` à pequena máquina de estados de Rev.024.

## Comparação curta com callbacks `+0x48` vizinhos

Foi feita uma varredura curta dos callbacks `+0x48` de records vizinhos:

| Label | Callback `+0x48` | Chama `0x0013a0f8` no trecho inicial? | Store direto em `+0x800` observado no trecho inicial? |
|---|---|---|---|
| `BARREL` | `0x001e9f08` | sim | não |
| `ROPE` | `0x001d27a8` | sim | sim, `sw s6,0x800(v1)` |
| `CHAIN` | `0x001e8f38` | sim | não |
| `FLEVER` | `0x0018e5b0` | sim | não |
| `FLEVER_TRISTATE` | `0x001c09c8` | sim | não |
| `WLEVER2` | `0x001c0c40` | sim | não |

Isso não prova que o slot `+0x48` sempre significa a mesma coisa em todos os records. O que fica confirmado é mais estreito: no record `ROPE`, o callback `+0x48` instala o state block em `[entity + 0x800]`.

## Relação com o dispatcher

O campo `candidate_state_id` de Rev.024 é:

```txt
[candidate_state_block_ptr + 0x48]
```

O template copiado por `0x001d27a8` inicializa esse offset com zero:

```txt
template[0x48] = 0
```

Portanto, a sequência estática provável ao inicializar o record `ROPE` é:

```txt
0x001d27a8
-> obtém/resolve state block
-> copia template com candidate_state_id = 0
-> grava state block em [entity + 0x800]
```

Depois, quando `0x001d3a30` roda e seu gate permite:

```txt
0x001d3a30
-> 0x001d37c8
-> lê [state_block + 0x48]
-> dispatch por jump table
```

## O que fica confirmado

1. `0x001d27a8` é o callback `+0x48` do record `ROPE`.
2. `0x001d27a8` chama `0x0013a0f8` e preserva o retorno como `$s6`.
3. `0x001d27a8` grava `$s6` em `[entity + 0x800]`.
4. `0x001d27a8` copia `0x90` bytes de template a partir de `0x004c46b0` para o ponteiro retornado.
5. O template inicializa `[state_block + 0x48]` com `0`, que é o campo `candidate_state_id` usado por Rev.024.
6. O template contém `0x0000012c` em `[state_block + 0x44]`, campo decrementado pelo `state_1_block` em Rev.024.

## O que fica provável

1. `0x001d27a8` é um inicializador/construtor do state block associado ao record `ROPE`.
2. O estado inicial do dispatcher para esse state block é `state_0_block`, porque `candidate_state_id` começa em `0`.
3. `[state_block + 0x44]` começa em `0x12c` antes de ser sobrescrito ou decrementado por estados posteriores.

## O que fica possível

1. O callback `+0x48` pode ser um slot de inicialização em parte dos records, mas a comparação curta não sustenta isso como regra global.
2. `0x0013a0f8` fornece/resolve um ponteiro de bloco estruturado; Rev.028 mostra que chamá-la definitivamente de alocador seria forte demais.
3. O valor `0x1b2` passado perto da chamada a `0x0013a0f8` pode ser um ID/tipo/tamanho auxiliar, mas isso permanece não confirmado.

## O que permanece desconhecido

1. O significado exato de `0x0013a0f8`.
2. O significado dos argumentos passados para `0x0013a0f8`.
3. Se `[state_block + 0x04]` é flag de ativação, modo, estágio ou outro tipo de gate.
4. Quem chama o callback `0x001d27a8` via record `ROPE`.

## O que é descartado

1. Tratar o state block de Rev.024 como estrutura abstrata sem origem. Para o record `ROPE`, sua origem está ligada ao callback `0x001d27a8`.
2. Generalizar automaticamente o papel do slot `+0x48` para todos os records vizinhos.

## Próximo teste mínimo

Sem gameplay:

1. Analisar `0x0013a0f8` apenas o suficiente para entender o contrato de retorno usado por `0x001d27a8`.
2. Procurar quem chama callbacks a partir dos slots `+0x38`, `+0x40`, `+0x48` dos records.
3. Comparar `ROPE +0x40` com callbacks `+0x40` de `BARREL` e `CHAIN` para entender se esse slot é update-like.

## Veredito conservador

Rev.027 confirma a origem estática do state block usado pelo dispatcher para o record `ROPE`: o callback `0x001d27a8` copia um template de `0x90` bytes, instala o ponteiro em `[entity + 0x800]`, e esse template já contém `candidate_state_id = 0`.

Isso fortalece a cadeia de evidência sem precisar de novo teste de gameplay.

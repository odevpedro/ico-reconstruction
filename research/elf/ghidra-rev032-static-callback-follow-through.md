# rev.032 — Static Callback Follow-through

## Data

2026-05-13

## Objetivo

Executar os próximos passos definidos após Rev.031 sem gameplay:

1. aprofundar `0x0013fc00`;
2. rastrear `0x0013f7a8`;
3. fazer busca dataflow mais ampla para `0x001d3a30`;
4. comparar callbacks `+0x40` de records vizinhos;
5. preparar o conjunto para commit.

## Escopo

Incluído:

- análise estática de `0x0013fc00`;
- análise estática de `0x0013f7a8` e `0x0013f3f0`;
- nova busca por referência direta a `0x001d3a30`;
- comparação curta de `+0x40` para `BARREL`, `ROPE`, `CHAIN`, `FLEVER` e `FLEVER_TRISTATE`;
- consolidação do estado atual após Rev.026-Rev.031.

Excluído:

- runtime;
- gameplay;
- screenshots;
- assets;
- nomeação definitiva de slots de lifecycle.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `AGENTS.md` | ordem de prioridade e disciplina de evidência |
| `research/elf/ghidra-rev026-rope-record-table-context.md` | records vizinhos e slots `+0x40` |
| `research/elf/ghidra-rev031-record-callback-dispatchers.md` | dispatchers de `+0x38` e `+0x48` |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |

## 1. Aprofundamento de `0x0013fc00`

Rev.031 confirmou que `0x0013fc00` chama callbacks `+0x48`.

O trecho completo mostra duas camadas:

```asm
0x0013fc08: lw    s2,-0x671c(gp)
...
0x0013fc38: lw    v0,+0x48(s2)
0x0013fc44: jalr  ra,v0
0x0013fc48: daddu a0,s2,zero
...
0x0013fc68: lw    v0,+0x4c(s2)
...
0x0013fc80: lw    s0,+0x0(v0)
...
0x0013fc8c: lw    v0,+0x16c(s0)
0x0013fc94: lw    s0,+0x34(s0)
0x0013fc98: lw    v1,+0x50(s2)
0x0013fc9c: lw    v0,+0x50(s0)
0x0013fca0: and   v1,v1,v0
...
0x0013fcac: lw    v0,+0x48(s0)
0x0013fcb8: jalr  ra,v0
0x0013fcbc: daddu a0,s0,zero
...
0x0013fce4: lw    s2,+0x34(s2)
```

Interpretação conservadora:

- `s2` vem de uma lista global via `gp - 0x671c`;
- `s2` também parece encadeado por `+0x34`;
- a função percorre até `0x20` buckets/índices usando base `0x00281ab0`;
- cada bucket fornece um `s0`;
- `s0` também é encadeado por `+0x34`;
- o filtro usa campos `+0x16c` e máscara `+0x50`;
- quando aprovado, o callback `+0x48(s0)` é chamado com `a0 = s0`.

Isso torna `0x0013fc00` um dispatcher estático forte para callbacks `+0x48`, mas ainda não identifica qual lista contém o record `ROPE` em runtime.

## 2. Rastreamento de `0x0013f7a8`

Rev.031 apontou `0x001b7ac0` como pista para o slot `+0x40`:

```asm
0x001b7ac0: lw    a1,+0x40(s7)
0x001b7ac4: beq   a1,zero,0x001b7ad4
0x001b7acc: jal   0x0013f7a8
0x001b7ad0: addiu a3,zero,+0x13
```

`0x0013f7a8` não chama `a1` diretamente. Ele rearranja argumentos e chama `0x0013f3f0`:

```asm
0x0013f7a8: daddu v0,a3,zero
0x0013f7b0: andi  a3,a2,0xff
0x0013f7b4: daddu t1,t0,zero
0x0013f7b8: daddu a2,a1,zero
0x0013f7c0: daddu t0,v0,zero
0x0013f7c4: jal   0x0013f3f0
0x0013f7c8: daddu a1,a0,zero
```

Para o caller de `0x001b7ac0`, isso significa:

| Valor no caller | Em `0x0013f7a8` | Em `0x0013f3f0` |
|---|---|---|
| `a0 = s3` | vira `a1` | `s5 = a1 = s3` |
| `a1 = [s7 + 0x40]` | vira `a2` | `s4 = a2 = [s7 + 0x40]` |
| `a2 = 0` | vira `a3 & 0xff` | `s6 = 0` |
| `a3 = 0x13` | vira `t0` | `s3 = 0x13` |
| `t0` anterior | vira `t1` | `s7 = t1` |

`0x0013f3f0` trata esse valor como dado de registro/lista, não como callback chamado imediatamente:

```asm
0x0013f42c: bne   s4,zero,0x0013f43c
...
0x0013f550: sw    s4,+0x1c(s1)
```

Quando `s6 == 0`, há caminho que inicializa uma estrutura auxiliar e chama:

```asm
0x0013f530: daddu a0,s0,zero
0x0013f534: addiu a1,zero,+0x1
0x0013f538: jal   0x0013d1b0
0x0013f53c: daddu t2,s3,zero
0x0013f540: jal   0x0013d3c8
```

Interpretação conservadora:

- `0x0013f7a8` é wrapper de registro/enfileiramento;
- `0x0013f3f0` cria ou reutiliza nodes de tamanho aparente `0x94`;
- o valor vindo de `+0x40(s7)` é armazenado em `node + 0x1c`;
- esta trilha não resolve o caller de `0x001d3a30`, mas mostra que um valor de slot `+0x40` pode ser registrado/enfileirado para uso posterior.

## 3. Busca dataflow curta para `0x001d3a30`

Busca por palavra little-endian `0x001d3a30` encontrou uma única ocorrência direta:

| VA | Interpretação |
|---|---|
| `0x002a3974` | `ROPE +0x40` |

Busca pela codificação de `jal 0x001d3a30` não encontrou ocorrência.

Resultado:

- a evidência direta continua sendo `.data`, não chamada direta;
- o hit runtime de Rev.025 continua essencial;
- o caminho estático mais promissor agora é por registro/enfileiramento de slot `+0x40`, não por `jalr` local simples.

## 4. Comparação de callbacks `+0x40` vizinhos

Rev.026 listou os callbacks `+0x40`:

| Record | `+0x40` | Primeira leitura estática |
|---|---|---|
| `BARREL` | `0x001e9950` | função curta que incrementa `[state_block + 0x30]` e pode chamar `0x001ae460` |
| `ROPE` | `0x001d3a30` | função que lê `[entity + 0x800]` e pode chamar dispatcher `0x001d37c8` |
| `CHAIN` | `0x001e9810` | função que chama `0x001e9588`, usa global `gp - 0x6e0c`, acessa `[entity + 0x800]` |
| `FLEVER` | `0x0018ecc8` | função grande, usa `[entity + 0x800]`, campos `+0xcc`, `+0x60`, `+0x64` |
| `FLEVER_TRISTATE` | `0x001bc1a8` | função grande, usa `[entity + 0x800]`, campos `+0x14`, `+0x18`, `+0x04`, `+0x08` |

Padrão comum confirmado:

```txt
a0 -> [a0 + 0x15c] -> entity-ish/context state
entity-ish -> [entity-ish + 0x800] -> auxiliary/state block
```

Diferenças:

- `ROPE +0x40` é o único desses cinco que, até agora, foi ligado ao dispatcher `0x001d37c8`;
- os outros `+0x40` parecem update-like por acessarem o state block e campos internos, mas seus papéis não são iguais;
- `BARREL +0x40` é muito mais curta do que `ROPE`, `FLEVER` e `FLEVER_TRISTATE`;
- `CHAIN +0x40` depende de uma global relacionada a outro objeto/contexto.

Interpretação conservadora:

`+0x40` parece ser um slot comportamental/update-like em vários records vizinhos, porque as funções recebem `a0`, acessam `[a0 + 0x15c]`, e usam `[entity + 0x800]`. O nome exato do slot ainda não está provado.

## Estado consolidado após os cinco passos

| Questão | Estado |
|---|---|
| `0x0013fc00` entendido melhor? | Sim, é dispatcher/list traversal de `+0x48` com filtros por bucket/máscara |
| `0x0013f7a8` rastreado? | Sim, ele repassa para `0x0013f3f0` e não chama o valor diretamente |
| `0x001d3a30` tem nova referência direta? | Não, segue apenas em `ROPE +0x40` |
| `+0x40` vizinhos comparados? | Sim, há padrão comum de acesso a `[entity + 0x800]` |
| Commit preparado? | Sim, os documentos Rev.026-Rev.032 e persona podem ser commitados juntos |

## O que fica confirmado

1. `0x0013fc00` percorre uma estrutura/lista e chama `+0x48` com `a0` igual ao item/record percorrido.
2. `0x0013fc00` usa até `0x20` buckets/índices baseados em `0x00281ab0`.
3. `0x0013fc00` filtra candidatos por campos `+0x16c`, `+0x34` e máscara `+0x50`.
4. `0x0013f7a8` não chama diretamente o valor recebido em `a1`.
5. `0x0013f7a8` passa o valor recebido em `a1` para `0x0013f3f0` como `a2`.
6. Em `0x0013f3f0`, esse valor pode ser armazenado em `node + 0x1c`.
7. `0x001d3a30` continua tendo uma única referência direta por palavra: `0x002a3974`.
8. Os callbacks `+0x40` vizinhos acessam `[a0 + 0x15c]` e frequentemente `[entity + 0x800]`.

## O que fica provável

1. `0x0013fc00` é parte de um sistema amplo de dispatch/iteration de records ou entidades.
2. `+0x48` é callback de fase inicial/registro/ativação em parte dos records.
3. `+0x40` é callback comportamental/update-like em vários records vizinhos.
4. O caminho que chama `ROPE +0x40 = 0x001d3a30` pode passar por node/list registration antes do dispatch real.

## O que fica possível

1. `0x0013f3f0` pode registrar callbacks ou payloads para execução posterior por outro loop.
2. O valor em `node + 0x1c` pode ser uma função, payload ou descriptor usado por callbacks posteriores.
3. O caller de `0x001d3a30` pode estar em um dispatcher que lê `node + 0x1c`, não diretamente `[record + 0x40]`.

## O que permanece desconhecido

1. O dispatcher exato que invoca `0x001d3a30`.
2. Se `node + 0x1c` é chamado por `jalr` em algum fluxo posterior.
3. Qual lista contém `ROPE` no momento runtime.
4. O nome original dos slots `+0x38`, `+0x40` e `+0x48`.
5. Se `+0x40` é sempre update-like ou apenas parece update-like nos records comparados.

## O que é descartado

1. Tratar `0x0013f7a8` como caller direto do callback `+0x40`.
2. Tratar a ausência de `jal 0x001d3a30` como evidência de código morto.
3. Tratar o slot `+0x40` como resolvido apenas porque ele pode ser passado para `0x0013f7a8`.
4. Nomear `+0x40` definitivamente como update.

## Próximo teste mínimo

Sem gameplay:

1. Rastrear usos de `node + 0x1c` em estruturas de stride `0x94` criadas por `0x0013f3f0`.
2. Procurar `jalr` que use valor carregado de `+0x1c` desses nodes.
3. Mapear quem escreve a global `gp - 0x671c`, usada por `0x0013fc00` como head de lista.
4. Mapear quem escreve a base/buckets `0x00281ab0`.

## Veredito conservador

Os cinco passos avançaram o projeto sem gameplay. O principal ganho é que o caminho de callbacks deixou de ser apenas uma suspeita por `.data`: `+0x48` tem dispatcher claro em `0x0013fc00`, `+0x40` tem padrão comportamental consistente nos records vizinhos, e a pista `0x0013f7a8 -> 0x0013f3f0` mostra que valores vindos de `+0x40` podem ser registrados em nodes para uso posterior. O dispatcher exato de `ROPE +0x40 = 0x001d3a30` ainda não está identificado.

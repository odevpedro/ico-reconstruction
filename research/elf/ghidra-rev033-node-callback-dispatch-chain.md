# rev.033 — Node Callback Dispatch Chain

## Data

2026-05-13

## Objetivo

Executar a rodada "1-2-3-4-5" definida após Rev.032:

1. rastrear usos de `node + 0x1c`;
2. procurar `jalr` derivado de `node + 0x1c`;
3. mapear quem escreve a global `gp - 0x671c`;
4. mapear quem escreve/inicializa os buckets em torno de `0x00281ab0`;
5. reavaliar se isso explica o caller estático de `ROPE +0x40 = 0x001d3a30`.

## Escopo

Incluído:

- varredura estática de loads `+0x1c` seguidos por `jalr`;
- análise do cluster `0x0013f3f0..0x0013fc00`;
- análise dos buckets `0x00281a70`, `0x00281a90`, `0x00281ab0`, `0x00281ad0`;
- análise da lista global `gp - 0x671c`;
- cadeia provável de chamada para callbacks registrados a partir de slots `+0x40`.

Excluído:

- runtime;
- gameplay;
- prova de que o record `ROPE` específico percorreu esse caminho numa sessão real;
- nomeação definitiva dos subsistemas.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev031-record-callback-dispatchers.md` | slot `+0x48` e lacuna do `+0x40` |
| `research/elf/ghidra-rev032-static-callback-follow-through.md` | pista `0x0013f7a8 -> 0x0013f3f0` |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |

## Resultado curto

A cadeia estática provável para callbacks `+0x40` agora é:

```txt
record +0x40
-> loaded as callback candidate
-> 0x0013f7a8
-> 0x0013f3f0
-> stored into node +0x1c
-> later 0x0013f9d0 loop
-> 0x0013fb64 loads node +0x1c
-> 0x0013fb70 jalr node_callback
```

Para `ROPE`, Rev.026 já mostrou:

```txt
ROPE +0x40 = 0x001d3a30
```

Portanto, o mecanismo estático provável para chamar `0x001d3a30` é:

```txt
ROPE +0x40
-> registered into node +0x1c
-> dispatched by 0x0013fb70
```

Isto ainda não prova runtime específico para `ROPE`, mas reduz a lacuna de Rev.031/Rev.032: agora existe um caller indireto estático concreto para valores armazenados em `node + 0x1c`.

## 1. Usos de `node + 0x1c`

A varredura encontrou:

```txt
159 loads de +0x1c
5 padrões rastreados de lw +0x1c -> jalr
```

Hits rastreados:

| Load | Call | Observação |
|---|---|---|
| `0x0013fb64: lw v0,+0x1c(s0)` | `0x0013fb70: jalr ra,v0` | relevante; mesmo cluster de `0x0013f3f0` |
| `0x001bc15c: lw v0,+0x1c(v1)` | `0x001bc16c: jalr ra,v0` | outro subsistema/state block |
| `0x001bc17c: lw v0,+0x1c(v1)` | `0x001bc18c: jalr ra,v0` | outro subsistema/state block |
| `0x001bc394: lw v1,+0x1c(s0)` | `0x001bc3ac: jalr ra,v1` | outro subsistema/state block |
| `0x00245f64: lw v0,+0x1c(s0)` | `0x00245f70: jalr ra,v0` | outro subsistema |

O hit relevante para esta linha de investigação é `0x0013fb64 -> 0x0013fb70`, porque:

- fica dentro do mesmo cluster que cria/itera nodes;
- percorre nodes ligados a objetos `s2`;
- chama o valor armazenado em `node + 0x1c`.

## 2. Dispatcher de `node + 0x1c` em `0x0013f9d0`

Trecho relevante:

```asm
0x0013fa94: lui   v0,0x28
0x0013fa9c: addiu fp,v0,+0x1a70
...
0x0013fab8: lw    s2,+0x0(v0)
...
0x0013faf8: lw    s0,+0x2c(s2)
...
0x0013fb08: lw    v0,+0x14(s0)
...
0x0013fb18: lw    v0,+0x18(s0)
...
0x0013fb24: lw    v0,+0x10(s0)
0x0013fb28: bne   v0,zero,0x0013fb64
...
0x0013fb64: lw    v0,+0x1c(s0)
0x0013fb70: jalr  ra,v0
0x0013fb74: daddu a0,s2,zero
```

Leitura conservadora:

- `s2` vem de buckets baseados em `0x00281a70`;
- `s2 + 0x2c` aponta para uma lista de nodes;
- cada node é `s0`;
- o loop filtra por `node + 0x14`, `node + 0x18` e `node + 0x10`;
- quando segue pelo caminho indireto, carrega `node + 0x1c`;
- chama esse valor por `jalr`;
- passa `a0 = s2`.

Isso é o primeiro dispatcher estático localizado para valores que podem ter sido registrados em `node + 0x1c`.

## 3. Relação com `0x0013f7a8` e `0x0013f3f0`

Rev.032 mostrou que `0x0013f7a8` não chama o valor recebido em `a1`; ele o repassa para `0x0013f3f0` como `a2`.

Trecho de `0x0013f7a8`:

```asm
0x0013f7b8: daddu a2,a1,zero
0x0013f7c4: jal   0x0013f3f0
0x0013f7c8: daddu a1,a0,zero
```

Trecho de `0x0013f3f0`:

```asm
0x0013f414: daddu s4,a2,zero
...
0x0013f550: sw    s4,+0x1c(s1)
```

Assim, o valor passado como `a1` para `0x0013f7a8` pode se tornar:

```txt
node + 0x1c
```

E Rev.033 mostra que esse mesmo offset pode ser chamado depois:

```asm
0x0013fb64: lw    v0,+0x1c(s0)
0x0013fb70: jalr  ra,v0
```

## 4. Registro de callbacks `+0x40` em `0x001b76f8`

O caller investigado em Rev.032 aparece em `0x001b76f8`.

O início da função calcula `s7` a partir de um record table-like em `0x002a31b8`, usando stride `0x64`:

```asm
0x001b7724: addiu a2,zero,+0x64
0x001b7744: addiu a3,s2,+0x31b8
0x001b774c: lbu   a0,+0x46(s4)
0x001b7750: mult  a0,a2
0x001b7758: addu  s7,a2,a3
```

Depois, a função registra callback em duas alternativas:

```asm
0x001b7aa0: lw    a1,+0x24(s4)
...
0x001b7ab0: jal   0x0013f7a8
0x001b7ab4: addiu a3,zero,+0x13
```

ou, se `+0x24(s4)` é zero:

```asm
0x001b7ac0: lw    a1,+0x40(s7)
0x001b7acc: jal   0x0013f7a8
0x001b7ad0: addiu a3,zero,+0x13
```

Leitura conservadora:

- `s7` é um descriptor/record selecionado por índice e stride `0x64`;
- `+0x40(s7)` é usado como callback candidate;
- esse candidate é passado para `0x0013f7a8`;
- pelo caminho de `0x0013f3f0`, pode ser salvo em `node + 0x1c`;
- depois `0x0013fb70` pode chamá-lo.

Para `ROPE`, isso é compatível com:

```txt
s7 = ROPE record
[s7 + 0x40] = 0x001d3a30
node + 0x1c = 0x001d3a30
0x0013fb70 -> jalr 0x001d3a30
```

Essa cadeia ainda é condicional: depende de `s7` ser o record `ROPE` e do caminho runtime passar pela alternativa `+0x40(s7)`.

## 5. Buckets `0x00281a70`, `0x00281a90`, `0x00281ab0`, `0x00281ad0`

Foram encontrados dois pares de buckets parecidos:

| Bucket head | Bucket tail/sorted companion | Inicialização | Uso observado |
|---|---|---|---|
| `0x00281a70` | `0x00281a90` | `0x0013dda0`, `0x0013f2c8` | loop `0x0013f9d0` e callbacks `+0x28` / `node +0x1c` |
| `0x00281ab0` | `0x00281ad0` | `0x0013f2c8` e helpers `0x0013ee60`, `0x0013efd0` | loop `0x0013fc00` e callbacks `+0x48` |

Inicialização de `0x00281ab0/0x00281ad0`:

```asm
0x0013f2c8: lui   v1,0x28
0x0013f2cc: lui   v0,0x28
0x0013f2d0: addiu v0,v0,+0x1ad0
0x0013f2d4: addiu v1,v1,+0x1ab0
...
0x0013f2e0: sw    zero,+0x0(v1)
0x0013f2e8: sw    zero,+0x0(v0)
```

Registro em buckets `0x00281ab0/0x00281ad0`:

```asm
0x0013ee8c: lui   v0,0x28
0x0013ee94: addiu v0,v0,+0x1ab0
...
0x0013eed0: sw    s1,+0x0(a2)
...
0x0013efd4: lui   v0,0x28
0x0013efdc: addiu v0,v0,+0x1ab0
...
0x0013effc: sw    a0,+0x0(v1)
```

Interpretação conservadora:

- `0x00281ab0` é bucket/list head array para o dispatcher `+0x48` de Rev.031;
- `0x00281a70` é bucket/list head array para o dispatcher que inclui `node +0x1c`;
- ambos usam estruturas de lista parecidas, mas parecem alimentar fases diferentes.

## 6. Lista global `gp - 0x671c`

`0x0013fc00` usa:

```asm
0x0013fc08: lw s2,-0x671c(gp)
```

Writers e manipuladores principais encontrados:

| Endereço | Instrução | Papel provável |
|---|---|---|
| `0x001fc020` | `sw v0,-0x671c(gp)` | remove head ao unlinkar |
| `0x001fc090` | `sw s0,-0x671c(gp)` | insere primeiro elemento |
| `0x001fc0cc` | `sw s0,-0x671c(gp)` | insere antes do head |
| `0x001fc2d0` | `sw zero,-0x671c(gp)` | limpa lista |
| `0x001fc318` | `sw s0,-0x671c(gp)` | insere primeiro elemento em outro wrapper |
| `0x001fc33c` | `sw s0,-0x671c(gp)` | insere antes do head em outro wrapper |
| `0x001fc3d8` | `sw a0,-0x671c(gp)` | insere primeiro elemento direto |
| `0x001fc3fc` | `sw a0,-0x671c(gp)` | insere antes do head direto |
| `0x001fc504` | `sw s1,-0x671c(gp)` | insere clone/linked descriptor no head |

O cluster `0x001fbfc8..0x001fc504` também usa:

```txt
+0x34 / +0x38 = links duplamente encadeados
+0x44 = chave/prioridade usada para ordenação
+0x48 / +0x4c / +0x50 = campos copiados/instalados antes da inserção
+0x40 = bucket/index-like byte em alguns clones
```

Interpretação conservadora:

- `gp - 0x671c` é head de uma lista global ordenada por campo `+0x44`;
- essa lista é a fonte de `s2` em `0x0013fc00`;
- ela alimenta o dispatcher `+0x48`, não o dispatcher `node +0x1c` diretamente.

## Cadeia revisada

Modelo estático mais forte agora:

```txt
Object/record creation path:
  0x001b76f8
    selects descriptor/record s7 using stride 0x64
    if object field +0x24 is absent:
      callback = [s7 + 0x40]
      0x0013f7a8(object, callback, 0, 0x13)
        -> 0x0013f3f0
        -> node +0x1c = callback

Per-frame/iteration path:
  0x0013f9d0
    walks buckets 0x00281a70
    walks object nodes via s2 +0x2c
    if node state/flags allow:
      callback = [node +0x1c]
      jalr callback with a0 = s2
```

For `ROPE`:

```txt
ROPE +0x40 = 0x001d3a30
```

So the likely static caller path is:

```txt
0x001b76f8 registers 0x001d3a30 into node +0x1c
0x0013f9d0 / 0x0013fb70 calls node +0x1c
0x001d3a30 may call 0x001d37c8
```

## O que fica confirmado

1. `0x0013fb70` é um `jalr` direto para valor carregado de `node + 0x1c`.
2. `0x0013f3f0` pode armazenar em `node + 0x1c` o valor recebido via `a2`.
3. `0x0013f7a8` passa seu `a1` para `0x0013f3f0` como `a2`.
4. `0x001b7ac0` carrega `+0x40(s7)` em `a1` e chama `0x0013f7a8`.
5. `s7` em `0x001b76f8` é calculado a partir de tabela/records com stride `0x64`.
6. `0x00281a70` alimenta o loop que alcança `node +0x1c`.
7. `0x00281ab0` alimenta o loop `0x0013fc00` que chama callbacks `+0x48`.
8. `gp - 0x671c` é manipulado como head de lista global ordenada por `+0x44`.

## O que fica provável

1. O slot `+0x40` de records pode ser registrado como callback de node.
2. `0x0013fb70` é o dispatcher estático que faltava para callbacks vindos de `+0x40`.
3. Para `ROPE`, o callback registrado por esse caminho provavelmente é `0x001d3a30`.
4. O hit runtime em `0x001d3a30` observado em Rev.025 é compatível com essa cadeia.

## O que fica possível

1. `+0x40` é uma callback phase executada a partir de nodes por `0x0013f9d0`.
2. `+0x48` é outra callback phase executada por `0x0013fc00`.
3. `+0x38` pode ser uma callback phase especializada usada antes/durante criação ou processamento auxiliar.
4. Os buckets `0x00281a70` e `0x00281ab0` podem representar fases distintas do mesmo sistema de entidade/record.

## O que permanece desconhecido

1. Se `s7 == ROPE` no caminho runtime observado em Rev.025.
2. Qual valor exato foi armazenado em `node +0x1c` para o evento runtime observado.
3. Se todos os records usam `+0x40` por esse mesmo caminho.
4. O significado original de `node +0x10`, `+0x14`, `+0x18`, `+0x1c`.
5. O nome original das fases representadas por `+0x38`, `+0x40`, `+0x48`.

## O que é descartado

1. Dizer que não há caller estático plausível para `0x001d3a30`.
2. Tratar `0x0013f7a8` como caller direto do callback. Ele registra/repassa.
3. Confundir o dispatcher `+0x48` de `0x0013fc00` com o dispatcher `node +0x1c` de `0x0013f9d0`.
4. Dizer que `0x0013fb70` prova runtime específico de `ROPE`; ele prova o mecanismo, não o caso concreto.

## Próximo teste mínimo

Sem gameplay:

1. Analisar `0x001d3a30` como callback chamado por `0x0013fb70`, com foco nos argumentos esperados.
2. Comparar assinaturas dos callbacks `+0x40` vizinhos assumindo chamada por `0x0013fb70`.
3. Mapear as funções que chamam `0x001b76f8` para entender quando records são selecionados por stride `0x64`.
4. Rastrear o campo de índice em `s4 + 0x46` que seleciona `s7`.

## Veredito conservador

Rev.033 resolve a principal lacuna estática deixada por Rev.031/Rev.032: existe um dispatcher concreto para valores armazenados em `node +0x1c`, e existe uma cadeia que pode registrar `record +0x40` nesse campo. Para o record `ROPE`, isso fornece o melhor mecanismo estático atual para explicar como `0x001d3a30` é chamado sem `jal` direto. A confirmação específica de que o runtime observado usou `ROPE` por essa cadeia ainda exigiria captura de valores ou trace.

# rev.034 — Callback Signature and Record Selection

## Data

2026-05-13

## Objetivo

Executar a rodada seguinte após Rev.033:

1. analisar `0x001d3a30` assumindo chamada por `0x0013fb70`;
2. comparar assinaturas dos callbacks `+0x40` vizinhos sob o mesmo modelo;
3. mapear callers de `0x001b76f8`;
4. rastrear o índice em `s4 + 0x46` que seleciona o record/descriptor;
5. decidir se o README público já deve receber atualização conservadora.

## Escopo

Incluído:

- assinatura efetiva do dispatcher `0x0013fb70`;
- compatibilidade de `ROPE +0x40 = 0x001d3a30` com essa assinatura;
- comparação curta de callbacks `+0x40` vizinhos;
- callers diretos de `0x001b76f8`;
- seleção de record via índice/stride;
- impacto documental.

Excluído:

- gameplay;
- runtime adicional;
- nomes definitivos de lifecycle;
- prova de que o evento runtime de Rev.025 foi exatamente o record `ROPE`.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev026-rope-record-table-context.md` | record `ROPE` e vizinhos |
| `research/elf/ghidra-rev033-node-callback-dispatch-chain.md` | cadeia `record +0x40 -> node +0x1c -> jalr` |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |

## 1. Assinatura efetiva de `0x0013fb70`

Rev.033 localizou o dispatcher:

```asm
0x0013fb64: lw    v0,+0x1c(s0)
0x0013fb70: jalr  ra,v0
0x0013fb74: daddu a0,s2,zero
```

Portanto, o callback armazenado em `node +0x1c` é chamado com:

```txt
a0 = s2
```

No mesmo loop, `s2` vem dos buckets em torno de `0x00281a70`:

```asm
0x0013fab8: lw    s2,+0x0(v0)
...
0x0013faf8: lw    s0,+0x2c(s2)
```

Interpretação conservadora:

- `s2` é o objeto/entidade/contexto percorrido pelo loop;
- `s2 +0x2c` aponta para a lista de nodes desse objeto;
- cada node pode conter callback em `+0x1c`;
- o callback recebe o próprio `s2` como argumento.

## 2. Compatibilidade de `0x001d3a30`

`ROPE +0x40 = 0x001d3a30`.

Entrada de `0x001d3a30`:

```asm
0x001d3a44: daddu s1,a0,zero
0x001d3a50: lw    v1,+0x15c(s1)
0x001d3a54: lw    s2,+0x800(v1)
```

Esse padrão é compatível com a chamada por `0x0013fb70`, porque o callback espera um ponteiro em `a0` que tenha:

```txt
[a0 + 0x15c] -> entity/context state
[[a0 + 0x15c] + 0x800] -> state block
```

O restante da função continua compatível com o modelo anterior:

```asm
0x001d3af0: lw    a0,+0x800(v0)
0x001d3af4: lw    v1,+0x4(a0)
...
0x001d3b04: jal   0x001d37c8
0x001d3b08: daddu a0,s1,zero
```

Interpretação conservadora:

- se `0x001d3a30` está em `node +0x1c`, `0x0013fb70` passa o tipo de argumento que `0x001d3a30` espera;
- isso fortalece o modelo de Rev.033;
- ainda não prova que o node concreto observado em runtime continha `0x001d3a30`.

## 3. Comparação de callbacks `+0x40` vizinhos

Comparação sob o modelo:

```txt
0x0013fb70 -> callback(a0 = object/context)
```

| Record | Callback `+0x40` | Primeiras ações | Compatível com `a0 = object/context`? |
|---|---|---|---|
| `BARREL` | `0x001e9950` | `lw v0,+0x15c(a0)`; `lw a1,+0x800(v0)` | Sim |
| `ROPE` | `0x001d3a30` | `s1=a0`; `lw v1,+0x15c(s1)`; `lw s2,+0x800(v1)` | Sim |
| `CHAIN` | `0x001e9810` | saves `a0`; calls helper; later `lw v0,+0x15c(s0)`; `lw s0,+0x800(v0)` | Sim |
| `FLEVER` | `0x0018ecc8` | `s1=a0`; `lw v1,+0x15c(s1)`; `lw s3,+0x800(v1)` | Sim |
| `FLEVER_TRISTATE` | `0x001bc1a8` | `s1=a0`; `lw v0,+0x15c(s1)`; `lw s0,+0x800(v0)` | Sim |

Resultado:

- todos os cinco callbacks vizinhos aceitam `a0` como objeto/contexto;
- todos acessam a cadeia `[a0 +0x15c] -> +0x800`, diretamente ou logo após setup inicial;
- isso sustenta que `+0x40` é um slot comportamental/update-like chamado com o objeto/contexto como argumento.

Limite:

- "update-like" ainda é nome provável, não nome confirmado;
- cada callback faz lógica própria e não deve ser fundido semanticamente.

## 4. Callers diretos de `0x001b76f8`

Busca por codificação de:

```asm
jal 0x001b76f8
```

encontrou três calls diretos:

| Callsite | Contexto |
|---|---|
| `0x001b7d90` | loop inicial com `a1 = 2..5` |
| `0x001b7df8` | loop de intervalo dinâmico `[s3, s1)` |
| `0x001b7e6c` | loop sobre tabela em `0x004b3d10`, com decremento de `0xb5` entradas |

Trecho do primeiro loop:

```asm
0x001b7d88: daddu a1,s0,zero
0x001b7d90: jal   0x001b76f8
0x001b7d94: daddu a0,s2,zero
0x001b7d98: addiu s0,s0,+0x1
0x001b7d9c: slti  v0,s0,+0x6
```

Trecho do segundo:

```asm
0x001b7de0: lw    s1,+0x12c(v0)
...
0x001b7df8: jal   0x001b76f8
0x001b7dfc: daddu a0,s2,zero
```

Trecho do terceiro:

```asm
0x001b7e68: lhu   a1,+0x2(s0)
0x001b7e6c: jal   0x001b76f8
0x001b7e70: daddu a0,s2,zero
```

Interpretação conservadora:

- `0x001b76f8` é chamado durante uma rotina maior em `0x001b7ce0`;
- essa rotina parece inicializar/percorrer múltiplas entradas de objeto/record;
- `a0` é mantido como `s2`, contexto maior;
- `a1` é o índice/ID de entrada processada por `0x001b76f8`.

## 5. Seleção por `s4 +0x46`

Dentro de `0x001b76f8`, o índice usado para selecionar `s7` vem de `s4 +0x46`:

```asm
0x001b770c: sd    s7,+0x100(sp)
0x001b771c: addiu v0,v0,+0x4c48
...
0x001b7734: addu  s4,v1,v0
...
0x001b7744: addiu a3,s2,+0x31b8
0x001b774c: lbu   a0,+0x46(s4)
0x001b7750: mult  a0,a2
0x001b7758: addu  s7,a2,a3
```

Com:

```txt
a2 = 0x64
base = 0x002a31b8
s7 = 0x002a31b8 + ([s4 +0x46] * 0x64)
```

Isto combina com Rev.026, que observou records com stride `0x64`, incluindo:

```txt
0x002a3934 = ROPE
```

Derivação para `ROPE`:

```txt
0x002a3934 - 0x002a31b8 = 0x77c
0x77c / 0x64 = 0x13
```

Assim, se:

```txt
[s4 +0x46] == 0x13
```

então:

```txt
s7 = 0x002a3934 = ROPE record
[s7 +0x40] = 0x001d3a30
```

Esse é um avanço importante: o caminho de seleção do record `ROPE` é agora aritmeticamente compatível com o índice `0x13`.

## Modelo revisado

Modelo estático para o caso `ROPE`:

```txt
0x001b7ce0 caller loop
-> 0x001b76f8(context, entry_index)
-> s4 = 0x002a4c48 + entry_index * 0x4c
-> record_index = [s4 +0x46]
-> s7 = 0x002a31b8 + record_index * 0x64
-> if record_index == 0x13, s7 == ROPE
-> callback = [s7 +0x40] == 0x001d3a30
-> 0x0013f7a8 / 0x0013f3f0 stores callback into node +0x1c
-> 0x0013f9d0 / 0x0013fb70 calls node +0x1c with a0 = object/context
-> 0x001d3a30 can call dispatcher 0x001d37c8
```

## README impact

This revision is mature enough for a conservative README update because it changes the public project status from generic planning to a concrete, validated ELF research thread.

The README should not include:

- gameplay names for the internal states;
- claims that `ROPE` runtime was observed through this exact chain;
- copied game data.

It can safely include:

- the existence of a validated dispatcher research thread;
- links to Rev.023 and Rev.033/Rev.034;
- a short statement that recent work mapped callback records and a probable indirect call chain.

## O que fica confirmado

1. `0x001d3a30` expects `a0` to support `[a0 +0x15c] -> +0x800`.
2. `0x0013fb70` calls `node +0x1c` with `a0 = s2`.
3. The compared `+0x40` callbacks are compatible with `a0 = object/context`.
4. `0x001b76f8` has three direct callers in the inspected ELF.
5. `0x001b76f8` selects `s7` from `0x002a31b8 + ([s4+0x46] * 0x64)`.
6. The `ROPE` record at `0x002a3934` corresponds arithmetically to record index `0x13` from base `0x002a31b8`.

## O que fica provável

1. `0x0013fb70` is the practical dispatcher for callbacks registered from record slot `+0x40`.
2. `ROPE +0x40 = 0x001d3a30` is called through `node +0x1c` when an entry selects record index `0x13`.
3. `+0x40` is a behavior/update-like callback slot across these neighboring records.

## O que fica possível

1. The runtime hit in `0x001d3a30` came from this exact chain.
2. Other records in the same stride table use the same `+0x40 -> node +0x1c` registration path.
3. `s4 +0x46` may be a record/type index for object creation.

## O que permanece desconhecido

1. Whether the observed runtime hit had `[s4 +0x46] == 0x13`.
2. Who writes `s4 +0x46`.
3. The original names of the record table and lifecycle slots.
4. Whether all records use `+0x40` through `node +0x1c`.

## O que é descartado

1. Treating `0x001d3a30` as incompatible with the `0x0013fb70` dispatcher.
2. Treating record selection as opaque: the stride/index arithmetic is now explicit.
3. Updating README with gameplay semantics. The mature public update is structural only.

## Próximo teste mínimo

Sem gameplay:

1. Search for writes to `+0x46` in the `0x002a4c48` entry table or its constructors.
2. Map the layout of the `0x4c`-stride entry table around `0x002a4c48`.
3. Compare record index values for neighboring labels from base `0x002a31b8`.
4. If runtime is resumed later, capture `[s4 +0x46]`, `s7`, and `node +0x1c` at the registration path.

## Veredito conservador

Rev.034 strengthens the static chain for `ROPE +0x40 = 0x001d3a30`. The callback signature matches the `0x0013fb70` dispatcher, the neighboring `+0x40` callbacks follow the same object/context convention, and `ROPE` maps cleanly to record index `0x13` from the `0x002a31b8` table base. The exact runtime instance remains unproven, but the structural model is now strong enough to summarize conservatively in the public README.

# rev.030 — Provider Caller Survey

## Data

2026-05-13

## Objetivo

Avançar sem gameplay, comparando o uso de `0x0013a0f8` no callback `ROPE +0x48` com outros callers estáticos, para entender se o caso do state block de `ROPE` é único, comum ou parte de um padrão amplo.

## Escopo

Incluído:

- todos os `jal 0x0013a0f8` encontrados em `.text`;
- inferência simples de argumentos imediatos próximos ao call;
- comparação de calls com tamanho `0x90`;
- comparação de calls com metadados de arquivo em `0x00618xxx`;
- foco especial em `src/item.c`.

Excluído:

- análise completa dos 247 callers;
- runtime;
- gameplay;
- assets;
- nomes fortes de subsistema.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev029-state-block-provider-deeper-static.md` | contrato do provider no caso `ROPE` |
| `.local/extracted/SCUS_971.13.elf` | bytes, strings e instruções locais |
| `.local/reports/*mips-prologue-scan.json` | associação aproximada de caller a função por prologue |

## Método

Foi feita uma busca byte-level pela codificação de:

```asm
jal 0x0013a0f8
```

Para cada callsite, foi feita uma inferência local simples dos registradores:

```txt
a1 = tamanho pedido, quando imediato localmente recuperável
a2 = metadado de arquivo, quando construído por lui/addiu local
a3 = metadado numérico/linha, quando imediato localmente recuperável
```

Limite importante: a inferência é local e simples. Calls com argumentos vindos de registradores, cálculos longos ou valores carregados de memória podem aparecer como `None`.

## Resultado curto

Foram encontrados:

```txt
247 callsites para 0x0013a0f8
```

O caso `ROPE` permanece bem caracterizado:

```txt
callsite = 0x001d27e8
function = 0x001d27a8
a1       = 0x90
a2       = 0x00618f68 -> "src/item.c"
a3       = 0x1b2
```

`src/item.c` não apareceu em outros callsites inferidos nesta varredura.

## Distribuição curta de tamanhos inferidos

Tamanhos `a1` mais comuns quando recuperáveis pela análise simples:

| Tamanho inferido | Count |
|---:|---:|
| `None` | 124 |
| `0x50` | 9 |
| `0x10` | 9 |
| `0x40` | 8 |
| `0xa0` | 5 |
| `0x0c` | 5 |
| `0x20` | 5 |
| `0x898` | 4 |
| `0x17c` | 4 |
| `0x190` | 4 |
| `0x90` | 3 |

Interpretação:

- `0x0013a0f8` é amplamente usado;
- tamanho `0x90` não é único, mas é raro na inferência local;
- muitos calls têm tamanho não inferido por esse método simples.

## Calls com tamanho `0x90`

| Callsite | Função aproximada | `a1` | `a2` | `a3` | Observação |
|---|---|---:|---|---:|---|
| `0x0011cc50` | `0x0011cbf8` | `0x90` | `0x00555490` -> `src/Primitive.c` | `0x244` | call de outro subsistema |
| `0x0011cc78` | `0x0011cbf8` | dinâmico após primeiro call | `0x00555490` -> `src/Primitive.c` | `0x247` | a varredura simples marcou `0x90`, mas o valor real de `a1` vem de cálculo anterior |
| `0x001d27e8` | `0x001d27a8` | `0x90` | `0x00618f68` -> `src/item.c` | `0x1b2` | caso `ROPE +0x48` |

O segundo call em `0x0011cc78` não deve ser tratado como pedido fixo de `0x90`, porque logo antes há:

```asm
sll a1,s1,4
...
mflo a1
```

Ou seja, `a1` é recalculado entre o primeiro e o segundo call.

## Calls inferidos com metadados de arquivo `src/*.c`

A varredura encontrou diversos callsites com `a2` apontando para strings `src/*.c`, por exemplo:

| Callsite | Função aproximada | `a1` inferido | `a2` |
|---|---|---:|---|
| `0x001bbe70` | `0x001bbe50` | `0x0c` | `src/attackCheckBoundary.c` |
| `0x001c0104` | `0x001c00c0` | `0x190` | `src/box.c` |
| `0x001c09fc` | `0x001c09c8` | `0x20` | `src/switch.c` |
| `0x001d1694` | `0x001d1668` | `0x20` | `src/girl.c` |
| `0x001d1ca0` | `0x001d1c78` | `0x0c` | `src/girlForceField.c` |
| `0x001d27e8` | `0x001d27a8` | `0x90` | `src/item.c` |
| `0x001e9f38` | `0x001e9f08` | `0x40` | `src/rotObject.c` |
| `0x001f058c` | `0x001f0568` | `0x30` | `src/stormTest.c` |
| `0x00202174` | `0x00202148` | `0x400` | `src/act.c` |

Interpretação conservadora:

- `0x0013a0f8` é um provider usado por muitos sistemas;
- o par `a2/a3` frequentemente parece metadado de arquivo/linha;
- o caso `ROPE` está em `src/item.c`, coerente com o record `.data` de itens/objetos observado em Rev.026, mas isso ainda não nomeia os estados internos.

## Relação com records vizinhos

Alguns callbacks `+0x48` de records vizinhos também chamam `0x0013a0f8`:

| Label | Callback `+0x48` | Callsite | `a1` | `a2` |
|---|---|---|---:|---|
| `ROPE` | `0x001d27a8` | `0x001d27e8` | `0x90` | `src/item.c` |
| `FLEVER_TRISTATE` / `WLEVER` | `0x001c09c8` | `0x001c09fc` | `0x20` | `src/switch.c` |
| `WLEVER2` | `0x001c0c40` | `0x001c0c74` | `0x20` | `src/switch.c` |
| `BARREL` | `0x001e9f08` | `0x001e9f38` | `0x40` | `src/rotObject.c` |

Isso sugere que slot `+0x48` frequentemente inicia/obtém algum bloco auxiliar via o provider, mas o tamanho, arquivo e uso posterior variam por record.

## O que fica confirmado

1. Existem 247 callsites estáticos para `0x0013a0f8`.
2. O callsite `ROPE +0x48` em `0x001d27e8` passa `a1 = 0x90`.
3. O callsite `ROPE +0x48` passa `a2 = 0x00618f68`, string `src/item.c`.
4. O callsite `ROPE +0x48` passa `a3 = 0x1b2`.
5. `src/item.c` não apareceu em outros callsites inferidos nesta varredura.
6. Outros callbacks `+0x48` de records vizinhos também podem chamar `0x0013a0f8`, mas com tamanhos e metadados diferentes.

## O que fica provável

1. `0x0013a0f8` é um provider allocator-like amplo usado por múltiplos sistemas.
2. `a2/a3` são metadados de origem, provavelmente arquivo e linha.
3. O caso `ROPE` pertence ao domínio interno de `src/item.c`.

## O que fica possível

1. O slot `+0x48` em alguns records pode ser initializer-like, mas a regra não é globalmente comprovada.
2. Records de `item.c`, `switch.c` e `rotObject.c` podem usar o mesmo provider para blocos auxiliares diferentes.
3. Uma análise futura dos callers por `a2` pode revelar agrupamentos por arquivo fonte preservado.

## O que permanece desconhecido

1. Como os records escolhem qual callback chamar em runtime.
2. O papel exato do segundo call em `0x0011cbf8`, que recalcula `a1`.
3. Se todos os metadados `src/*.c` correspondem a arquivo/linha original ou a outra forma de tag.

## O que é descartado

1. Tratar `0x0013a0f8` como específico de `ROPE`.
2. Tratar tamanho `0x90` como exclusivo do record `ROPE`.
3. Usar `src/item.c` para nomear os cinco estados internos do dispatcher.

## Próximo teste mínimo

Sem gameplay:

1. Mapear callers de `0x0013a0f8` agrupados por `a2` (`src/*.c`) em um relatório metadata-only.
2. Comparar callbacks `+0x40` de `ROPE`, `BARREL` e `CHAIN` para testar se `+0x40` é update-like.
3. Procurar quem chama os slots `+0x38`, `+0x40`, `+0x48` por offset.

## Veredito conservador

`0x0013a0f8` é um provider allocator-like amplo. O caso `ROPE` está bem delimitado dentro desse uso amplo: ele pede `0x90` bytes com metadado `src/item.c:0x1b2`, recebe um payload pointer e usa esse payload como state block em `[entity + 0x800]`.

Esse avanço continua totalmente estático.

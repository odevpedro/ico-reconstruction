# rev.028 — State Block Provider Contract

## Data

2026-05-13

## Objetivo

Analisar minimamente `0x0013a0f8`, chamada por `ROPE +0x48` (`0x001d27a8`), para corrigir a interpretação do ponteiro retornado usado como state block.

## Escopo

Incluído:

- função `0x0013a0f8`;
- sua chamada direta a `0x00138e30`;
- comportamento de retorno quando o ponteiro é não zero;
- caminho de falha/diagnóstico em caso de ponteiro zero.

Excluído:

- análise completa de `0x00138e30`;
- nomes definitivos como allocator/factory/cache;
- runtime;
- gameplay e assets.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev027-rope-state-block-initializer.md` | contexto da chamada a `0x0013a0f8` |
| `.local/extracted/SCUS_971.13.elf` | instruções locais |

## Resumo

`0x0013a0f8` não deve ser chamado definitivamente de alocador.

O comportamento confirmado é mais estreito:

```txt
0x0013a0f8
-> chama 0x00138e30
-> se retorno != 0, devolve esse ponteiro
-> se retorno == 0, executa caminho de diagnóstico/assert e retorna zero
```

Portanto, para Rev.027, o termo mais seguro é:

```txt
state_block_provider
```

ou:

```txt
função que fornece/resolve o ponteiro do state block
```

## Instruções principais de `0x0013a0f8`

| VA | Instrução | Interpretação conservadora |
|---|---|---|
| `0x0013a0f8` | `addiu sp,sp,-0x470` | prólogo grande |
| `0x0013a104` | `daddu s4,a3,zero` | preserva argumento 4 |
| `0x0013a10c` | `daddu s3,a2,zero` | preserva argumento 3 |
| `0x0013a114` | `daddu s2,a1,zero` | preserva argumento 2 |
| `0x0013a118` | `daddu s1,a0,zero` | preserva argumento 1 |
| `0x0013a124` | `jal 0x00138e30` | chama função provedora/resolvedora principal |
| `0x0013a12c` | `daddu s5,v0,zero` | preserva retorno |
| `0x0013a130` | `bne s5,zero,0x0013a1dc` | se retorno não zero, pula para epílogo |
| `0x0013a134` | `daddu v0,s5,zero` | prepara retorno |
| `0x0013a138..0x0013a1d4` | chamadas/logs/assert-like | caminho de falha quando retorno é zero |
| `0x0013a1d8` | `daddu v0,s5,zero` | retorna zero após caminho de falha |
| `0x0013a1f8` | `jr ra` | retorno |

## Relação com `0x001d27a8`

Em `0x001d27a8`, o retorno de `0x0013a0f8` é salvo em `$s6`:

```txt
0x001d27e8: jal 0x0013a0f8
0x001d27f0: daddu s6,v0,zero
0x001d2804: sw s6,0x800(v1)
```

Logo, a cadeia confirmada deve ser escrita assim:

```txt
0x001d27a8
-> 0x0013a0f8
-> 0x00138e30
-> returned pointer
-> [entity + 0x800] = returned pointer
```

Não está confirmado se `0x00138e30` sempre aloca memória nova, recicla uma entrada, consulta cache, ou faz outro tipo de resolução.

## O que fica confirmado

1. `0x0013a0f8` chama `0x00138e30`.
2. Se `0x00138e30` retorna ponteiro não zero, `0x0013a0f8` retorna esse ponteiro.
3. Se `0x00138e30` retorna zero, `0x0013a0f8` entra em caminho de diagnóstico/assert-like e retorna zero.
4. `0x001d27a8` grava o ponteiro retornado por `0x0013a0f8` em `[entity + 0x800]`.

## O que fica provável

1. `0x0013a0f8` é um wrapper seguro ao redor de `0x00138e30`.
2. `0x00138e30` é o melhor próximo alvo para entender origem real do state block.

## O que fica possível

1. `0x00138e30` pode ser allocator, resolver de pool, construtor parcial, ou lookup com criação condicional.
2. O caminho de falha pode corresponder a assert/log de desenvolvimento preservado no binário.

## O que é descartado

1. Chamar `0x0013a0f8` definitivamente de alocador.
2. Dizer que `0x001d27a8` aloca diretamente o state block. O que ele faz diretamente é gravar o ponteiro retornado por `0x0013a0f8`.

## Próximo teste mínimo

Sem gameplay:

1. Analisar `0x00138e30` somente até identificar:
   - estrutura de entrada consultada;
   - de onde vem o ponteiro retornado;
   - se existe criação nova ou lookup em lista/pool.
2. Atualizar Rev.027 mentalmente para usar “fornece/resolve ponteiro” em vez de “aloca”.

## Veredito conservador

`0x0013a0f8` é um wrapper/guard em torno de `0x00138e30`. Para a cadeia do record `ROPE`, o fato importante é que ele fornece o ponteiro gravado em `[entity + 0x800]`; a origem exata desse ponteiro ainda depende de `0x00138e30`.

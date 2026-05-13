# rev.026 — ROPE Record Table Context

## Data

2026-05-13

## Objetivo

Mapear estaticamente a região `.data` que contém a referência direta a `0x001d3a30`, sem continuar o fluxo manual de screenshots no PCSX2.

O objetivo mínimo é responder:

```txt
0x001d3a30 pertence a que tipo de estrutura de dados?
```

## Escopo

Incluído:

- região `.data` em torno de `0x002a3934..0x002a3998`;
- referência direta a `0x001d3a30` em `0x002a3974`;
- records vizinhos com stride aparente de `0x64`;
- labels ASCII próximos, especialmente `ROPE`;
- ponteiros de função nos offsets recorrentes dos records.

Excluído:

- análise de assets;
- análise de `DATA.DF`;
- runtime adicional;
- nomes fortes de gameplay;
- semântica de rope/chain/barrel além de rótulos internos observados.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `AGENTS.md` | regras de cautela e evidência |
| `.local/key-concepts.md` | orientação conceitual |
| `research/elf/ghidra-rev024-internal-state-block-semantics.md` | contexto do dispatcher e state block |
| `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | caller `0x001d3a30` e referência inicial em `.data` |
| `.local/extracted/SCUS_971.13.elf` | bytes locais do ELF |

## Método

Foram feitas buscas byte-level e scans estruturais sobre o ELF local:

1. busca por palavras little-endian de endereços relevantes;
2. busca por labels ASCII em `.data`;
3. identificação de possíveis records com:
   - label ASCII em `record + 0x54`;
   - ponteiros para `.text` em slots recorrentes;
   - stride constante de `0x64` bytes entre records vizinhos.

O método não depende de nomes automáticos do Ghidra.

## Resultado curto

`0x001d3a30` aparece dentro de um record de `.data` cujo label local é `ROPE`.

O record começa provavelmente em:

```txt
0x002a3934
```

e contém:

| Offset no record | VA | Valor | Interpretação conservadora |
|---:|---|---:|---|
| `+0x38` | `0x002a396c` | `0x001d3b28` | ponteiro de função/callback |
| `+0x40` | `0x002a3974` | `0x001d3a30` | ponteiro de função/callback runtime-confirmado |
| `+0x48` | `0x002a397c` | `0x001d27a8` | ponteiro de função/callback que inicializa `[entity + 0x800]` |
| `+0x54` | `0x002a3988` | ASCII `ROPE` | label interno do record |

Este layout torna mais forte a interpretação de Rev.025: `0x001d3a30` não é uma função solta; ela é callback de um record estruturado.

## Busca por referências diretas

Busca por palavras little-endian:

| Valor buscado | Resultado |
|---:|---|
| `0x002a396c` | nenhuma palavra direta encontrada |
| `0x002a3974` | nenhuma palavra direta encontrada |
| `0x002a397c` | nenhuma palavra direta encontrada |
| `0x002a3988` | nenhuma palavra direta encontrada |
| `0x001d3b28` | encontrado em `.data` em `0x002a396c` |
| `0x001d3a30` | encontrado em `.data` em `0x002a3974` |
| `0x001d27a8` | encontrado em `.data` em `0x002a397c` |

Interpretação conservadora:

- há ponteiros diretos para as funções dentro do record;
- não foi encontrada referência direta simples ao endereço do próprio record ou ao label `ROPE`;
- isso é compatível com acesso por base/índice/stride, mas ainda não confirma quem itera a tabela.

## Records vizinhos com stride `0x64`

Os labels abaixo aparecem separados por `0x64` bytes:

| Record start provável | Label em `+0x54` | Ponteiros de função observados |
|---|---|---|
| `0x002a38d0` | `BARREL` | `+0x24=0x001ea228`, `+0x28=0x001ea220`, `+0x2c=0x001ea248`, `+0x38=0x001ea030`, `+0x40=0x001e9950`, `+0x48=0x001e9f08` |
| `0x002a3934` | `ROPE` | `+0x38=0x001d3b28`, `+0x40=0x001d3a30`, `+0x48=0x001d27a8` |
| `0x002a3998` | `CHAIN` | `+0x38=0x001e9630`, `+0x40=0x001e9810`, `+0x48=0x001e8f38` |
| `0x002a39fc` | `FLEVER` | `+0x38=0x0018f640`, `+0x40=0x0018ecc8`, `+0x48=0x0018e5b0` |
| `0x002a3a60` | `FLEVER_TRISTATE` | `+0x38=0x001bc438`, `+0x40=0x001bc1a8`, `+0x48=0x001c09c8` |

O stride observado:

```txt
0x002a38d0 -> 0x002a3934 = 0x64
0x002a3934 -> 0x002a3998 = 0x64
0x002a3998 -> 0x002a39fc = 0x64
0x002a39fc -> 0x002a3a60 = 0x64
```

## Região maior de records parecidos

Outros runs de records com stride `0x64` foram encontrados na mesma área de `.data`.

| Range | Count | Primeiro label | Último label |
|---|---:|---|---|
| `0x002a3420..0x002a35b0` | 5 | `SOBJ` | `PARTICLE` |
| `0x002a3678..0x002a3740` | 3 | `LIGHTBIT` | `SPIDER_LAYOUT` |
| `0x002a37a4..0x002a386c` | 3 | `SOFA` | `ROTOBJECT` |
| `0x002a38d0..0x002a3a60` | 5 | `BARREL` | `FLEVER_TRISTATE` |
| `0x002a3ac4..0x002a3b8c` | 3 | `WLEVER` | `NONE` |
| `0x002a3d80..0x002a3f74` | 6 | `SEFFECT` | `DEMO_QSWORD` |
| `0x002a3fd8..0x002a4294` | 8 | `CHANDELIER` | `DYNAMICMOTIONDAT` |
| `0x002a44ec..0x002a4618` | 4 | `CAGEFIX` | `QUEEN_BARRIER` |
| `0x002a4744..0x002a48d4` | 5 | `INTEREST1` | `KYOMI` |

Essa tabela é somente uma amostra estrutural dos runs detectados. Ela não prova que todos os records pertencem à mesma tabela global, mas mostra que o layout do record `ROPE` é parte de um padrão mais amplo.

## Relação com Rev.024 e Rev.025

Rev.024 confirmou a máquina interna:

```txt
[context + 0x15c] -> entity
[entity + 0x800] -> candidate_state_block_ptr
[candidate_state_block_ptr + 0x48] -> candidate_state_id
```

Rev.025 mostrou que:

```txt
0x001d3a30 -> 0x001d37c8
```

e que `0x001d27a8` escreve em:

```txt
[entity + 0x800]
```

Rev.026 acrescenta que `0x001d27a8`, `0x001d3a30` e `0x001d3b28` aparecem como callbacks no mesmo record `ROPE`.

Modelo estrutural revisado:

```txt
ROPE record in .data
  +0x38 -> 0x001d3b28
  +0x40 -> 0x001d3a30 -> may call dispatcher 0x001d37c8
  +0x48 -> 0x001d27a8 -> initializes [entity + 0x800]
```

## O que fica confirmado

1. `0x001d3a30` está em um record `.data` com label ASCII `ROPE`.
2. O record `ROPE` tem stride consistente de `0x64` bytes em relação aos records vizinhos `BARREL`, `CHAIN`, `FLEVER` e `FLEVER_TRISTATE`.
3. Os offsets `+0x38`, `+0x40` e `+0x48` do record `ROPE` contêm ponteiros para `.text`.
4. `0x001d3a30` ocupa o slot `+0x40` do record `ROPE`.
5. `0x001d27a8`, que escreve `[entity + 0x800]`, ocupa o slot `+0x48` do mesmo record.
6. Não foi encontrada referência direta simples aos endereços `0x002a396c`, `0x002a3974`, `0x002a397c` ou `0x002a3988`.

## O que fica provável

1. O record `ROPE` é parte de uma tabela de records de objeto/entidade com stride `0x64`.
2. Os slots `+0x38`, `+0x40` e `+0x48` são callbacks/funções de ciclo de vida ou comportamento, mas seus papéis exatos ainda dependem de comparação entre records.
3. `0x001d3a30` é callback associado ao label `ROPE`, não apenas um caller genérico do dispatcher.

## O que fica possível

1. `0x001d3b28`, `0x001d3a30` e `0x001d27a8` podem corresponder a fases distintas do mesmo record, por exemplo update/finalização/inicialização, mas essa ordem nominal ainda não está provada.
2. Um iterador global pode acessar esses records por base + `0x64 * index`, o que explicaria a ausência de referência direta ao endereço do record `ROPE`.
3. Comparar os slots `+0x38`, `+0x40`, `+0x48` entre `BARREL`, `ROPE`, `CHAIN` e `FLEVER` pode revelar o papel relativo de cada slot.

## O que permanece desconhecido

1. Quem referencia a base exata da tabela ou do run que contém `ROPE`.
2. Qual código seleciona o record `ROPE`.
3. Se o label `ROPE` corresponde diretamente a uma entidade visível, recurso, classe interna, ou outro tipo de record.
4. O papel exato dos slots `+0x38`, `+0x40`, `+0x48`.

## O que é descartado

1. Tratar `ROPE` como prova semântica suficiente para nomear os estados internos do dispatcher. O label ajuda a localizar o record, mas não nomeia `state_0..state_4`.
2. Tratar `0x001d3a30` como função órfã. Ela pertence a um record estruturado em `.data`.

## Próximo teste mínimo

Sem runtime manual:

1. Comparar os slots `+0x38`, `+0x40`, `+0x48` de records vizinhos para inferir papéis por posição.
2. Procurar código que use stride `0x64` sobre a região `.data` dos records.
3. Analisar `0x001d27a8` com foco em:
   - tamanho do bloco alocado/copied;
   - template copiado;
   - campos inicializados em `[entity + 0x800]`.

## Veredito conservador

`0x001d3a30` agora está vinculado a um record `.data` rotulado `ROPE`, com layout recorrente e stride `0x64`. A ligação entre o dispatcher `0x001d37c8` e o record `ROPE` é estruturalmente forte: o callback `+0x40` chama o dispatcher, enquanto o callback `+0x48` instala o state block em `[entity + 0x800]`.

Isso melhora o modelo do sistema sem exigir novo gameplay manual. A semântica dos cinco estados internos continua separada: eles são estados do state block associado ao record, não nomes de gameplay.

# rev.029 — State Block Provider Deeper Static Pass

## Data

2026-05-13

## Objetivo

Avançar a análise estática sem gameplay, aprofundando `0x00138e30`, chamada por `0x0013a0f8`, que por sua vez fornece o ponteiro gravado por `ROPE +0x48` em `[entity + 0x800]`.

## Escopo

Incluído:

- argumentos passados por `0x001d27a8` para `0x0013a0f8`;
- contrato mínimo de `0x0013a0f8 -> 0x00138e30`;
- comportamento estrutural observado em `0x00138e30`;
- campos escritos no bloco/registro interno retornado por `0x00138e30`.

Excluído:

- gameplay;
- runtime adicional;
- análise completa do alocador/pool;
- nomes definitivos de subsistema;
- assets e `DATA.DF`.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev027-rope-state-block-initializer.md` | contexto do callback `ROPE +0x48` |
| `research/elf/ghidra-rev028-state-block-provider-contract.md` | contrato inicial de `0x0013a0f8` |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |

## Resumo

`0x00138e30` é fortemente allocator/pool-like.

Ainda é prudente não fixar um nome definitivo, mas a função:

- recebe um identificador/handle em `$a0`;
- recebe tamanho pedido em `$a1`;
- recebe metadados de origem em `$a2/$a3`;
- alinha o tamanho para múltiplo de `0x10`;
- soma overhead de `0x40`;
- caminha uma lista via campos `+0x20`, `+0x24`, `+0x28`, `+0x2c`, `+0x34`, `+0x44`;
- escreve metadados no bloco selecionado;
- retorna `s1 + 0x40`, isto é, um ponteiro de payload após header de `0x40` bytes.

Para a cadeia do record `ROPE`, isso significa que o ponteiro instalado em `[entity + 0x800]` por `0x001d27a8` é provavelmente um payload retornado por um allocator/pool com header interno imediatamente antes dele.

## Argumentos vindos de `0x001d27a8`

Trecho de `0x001d27a8`:

| VA | Instrução | Papel |
|---|---|---|
| `0x001d27ac` | `lui a2,0x0062` | prepara parte alta de metadado |
| `0x001d27b4` | `addiu a2,a2,-28824` | `a2 = 0x00618f68` |
| `0x001d27c4` | `addiu a3,zero,434` | `a3 = 0x1b2` |
| `0x001d27d4` | `addiu a1,zero,144` | `a1 = 0x90` |
| `0x001d27e4` | `lw a0,-0x68e0(gp)` | `a0 = handle/contexto global` |
| `0x001d27e8` | `jal 0x0013a0f8` | chamada provedora/resolvedora |

O endereço `0x00618f68` contém a string curta:

```txt
src/item.c
```

Interpretação conservadora:

- `a1 = 0x90` é o tamanho pedido, coerente com o template de `0x90` bytes copiado depois por Rev.027;
- `a2 = 0x00618f68` e `a3 = 0x1b2` parecem metadados de origem/diagnóstico (`src/item.c`, linha 434);
- isso reforça que `0x0013a0f8`/`0x00138e30` têm comportamento allocator/debug-wrapper-like.

## Contrato revisado

Fluxo confirmado:

```txt
0x001d27a8
-> 0x0013a0f8(a0=handle, a1=0x90, a2="src/item.c", a3=0x1b2)
-> 0x00138e30(...)
-> returns payload pointer or zero
-> 0x001d27a8 stores returned pointer at [entity + 0x800]
```

## Instruções-chave de `0x00138e30`

### Entrada e preservação de argumentos

| VA | Instrução | Interpretação |
|---|---|---|
| `0x00138e30` | `addiu sp,sp,-0x4b0` | prólogo grande |
| `0x00138e40` | `daddu s6,a0,zero` | preserva argumento 1 |
| `0x00138e48` | `daddu s5,a3,zero` | preserva linha/metadado |
| `0x00138e50` | `daddu s4,a2,zero` | preserva arquivo/metadado |
| `0x00138e58` | `daddu s0,a1,zero` | preserva tamanho pedido |

### Normalização do tamanho

| VA | Instrução | Interpretação |
|---|---|---|
| `0x00138f3c` | `lui v1,0xffff` | prepara máscara |
| `0x00138f40` | `addiu v0,s0,15` | `size + 0xf` |
| `0x00138f44` | `ori v1,v1,0xfff0` | máscara `0xfffffff0` |
| `0x00138f4c` | `and v0,v0,v1` | alinha tamanho para múltiplo de `0x10` |
| `0x00138f50` | `addiu v0,v0,64` | soma overhead/header de `0x40` |
| `0x00138f58` | `srl s2,v0,4` | converte tamanho total para unidades de `0x10` |

Para o caso `ROPE`, `a1 = 0x90`.

Então:

```txt
aligned_size = 0x90
total_with_header = 0x90 + 0x40 = 0xd0
unit_count = 0xd0 / 0x10 = 0x0d
```

### Lista/pool observado

Campos usados em `0x00138e30`:

| Offset | Uso observado |
|---:|---|
| `+0x20` | ponteiro/ligação atualizado em operações de lista |
| `+0x24` | ponteiro/ligação copiado para novo bloco |
| `+0x28` | ponteiro/ligação reversa ou lateral |
| `+0x2c` | próximo candidato durante varredura |
| `+0x30` | recebe `$s6`, argumento/handle |
| `+0x34` | tamanho/unidades; comparado com `s2` e depois escrito |
| `+0x38` | recebe `$s5`, metadado de linha |
| `+0x44` | cabeça/lista inicial lida de `[s6 + 0x44]` |

### Escritas antes do retorno

| VA | Instrução | Interpretação conservadora |
|---|---|---|
| `0x001390e0` | `sw s0,0x20(s3)` | atualiza ligação do novo bloco |
| `0x001390f0` | `sw v0,0x24(s3)` | copia/ajusta ligação |
| `0x001390f4` | `sw v1,0x34(s3)` | escreve tamanho/unidades remanescentes |
| `0x001390fc` | `sw v0,0x28(s3)` | copia/ajusta ligação |
| `0x00139104` | `sw v1,0x2c(s3)` | copia/ajusta ligação |
| `0x00139114` | `sw s3,0x44(s6)` | atualiza cabeça/lista se necessário |
| `0x00139224` | `sw s6,0x30(s1)` | grava handle/metadado no bloco selecionado |
| `0x00139228` | `sw s5,0x38(s1)` | grava linha/metadado no bloco selecionado |
| `0x00139230` | `sw s3,0x24(s1)` | grava ligação |
| `0x00139238` | `sw fp,0x34(s1)` | grava tamanho/unidades ajustado |
| `0x00139244` | `sb zero,0x1f(s1)` | termina string/campo textual |

### Retorno

| VA | Instrução | Interpretação |
|---|---|---|
| `0x00139198` | `addiu s0,s1,64` | prepara ponteiro de payload após header |
| `0x0013925c` | `daddu v0,s0,zero` | retorna payload pointer |
| `0x00139274` | `daddu v0,zero,zero` | retorno zero em caminhos de falha |

O retorno não é o início do header interno. É:

```txt
payload_ptr = block_header + 0x40
```

Isso combina com `0x001d27a8`: o template de `0x90` bytes é copiado para o payload pointer retornado.

## Relação com o template de Rev.027

Rev.027 mostrou:

```txt
template source = 0x004c46b0
copy size       = 0x90
destination     = pointer returned by 0x0013a0f8
```

Rev.029 acrescenta:

```txt
requested size       = 0x90
allocator overhead   = 0x40
returned pointer     = internal_block + 0x40
```

Assim, o state block visto por `0x001d3a30` e `0x001d37c8` é o payload, não o header interno do allocator/pool.

## O que fica confirmado

1. `0x001d27a8` pede `0x90` bytes a `0x0013a0f8`.
2. `0x001d27a8` passa `0x00618f68` como metadado, e esse endereço contém `src/item.c`.
3. `0x001d27a8` passa `0x1b2` como metadado numérico, provavelmente linha 434.
4. `0x00138e30` alinha o tamanho pedido para múltiplo de `0x10`.
5. `0x00138e30` soma overhead de `0x40`.
6. `0x00138e30` retorna `block_header + 0x40` em caminho de sucesso.
7. O ponteiro gravado em `[entity + 0x800]` é o payload pointer retornado, não o header do bloco interno.

## O que fica provável

1. `0x00138e30` é allocator/pool-like.
2. `0x0013a0f8` é wrapper/guard para esse provider allocator-like.
3. Os metadados `src/item.c` e `0x1b2` servem para diagnóstico, assert ou rastreamento de alocação.

## O que fica possível

1. A estrutura antes do payload é um header de alocação de `0x40` bytes.
2. Os campos `+0x20`, `+0x24`, `+0x28`, `+0x2c` implementam lista duplamente ligada ou estrutura semelhante de free/used blocks.
3. O campo `+0x34` mede tamanho em unidades de `0x10`.

## O que permanece desconhecido

1. O significado exato de `$a0`/`$s6` como handle.
2. Se a lista é free list, used list, pool arena ou outra estrutura.
3. O significado completo dos campos `+0x20..+0x44` do header.
4. Se o mesmo provider é usado por muitos subsistemas além do record `ROPE`.

## O que é descartado

1. Dizer que `[entity + 0x800]` aponta para um header allocator interno. A evidência mostra que aponta para o payload retornado.
2. Tratar `0x0013a0f8` como caixa-preta total. Já há contrato suficiente para o caso `ROPE`.
3. Chamar `0x00138e30` de allocator definitivo sem ressalva. O comportamento é allocator-like, mas ainda falta mapear a estrutura global.

## Próximo teste mínimo

Sem gameplay:

1. Procurar outros callers de `0x0013a0f8` com `a1 = 0x90` ou metadados `src/item.c`.
2. Comparar providers de records vizinhos para ver se também usam esse allocator-like.
3. Continuar evitando nomes de gameplay para os estados internos.

## Veredito conservador

O state block do record `ROPE` é um payload de `0x90` bytes fornecido por uma rotina allocator/pool-like com overhead interno de `0x40` bytes. Esse payload é instalado em `[entity + 0x800]`, recebe o template de `0x004c46b0`, e então alimenta o dispatcher confirmado em `0x001d37c8`.

Esse avanço é totalmente estático e não depende de novo gameplay.

# rev.025 — Runtime-Confirmed Caller Context

## Data

2026-05-13

## Objetivo

Analisar o caller runtime-confirmado `0x001d3a30`, que alcançou o dispatcher `0x001d37c8` durante a sessão manual no PCSX2, e documentar o contexto mínimo que ele fornece para a máquina de estados de Rev.024.

## Escopo

Incluído:

- função/caller `0x001d3a30`;
- chamada estática para `0x001d37c8`;
- acesso a `[context + 0x15c]` e `[entity + 0x800]`;
- referência de dados direta a `0x001d3a30` em `.data`;
- funções vizinhas apontadas pela mesma região de dados, somente como contexto estrutural.

Excluído:

- análise de `DATA.DF`;
- análise de overlays;
- nomes fortes de gameplay;
- interpretação ampla de strings ou assets;
- patch runtime ou instrumentação do emulador.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `AGENTS.md` | regras de cautela e separação de evidência |
| `.local/key-concepts.md` | orientação conceitual |
| `research/elf/ghidra-rev023-dispatcher-table-resolution.md` | fonte de verdade para dispatcher e tabela |
| `research/elf/ghidra-rev024-internal-state-block-semantics.md` | estado atual dos blocos internos e validação runtime parcial |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |
| PCSX2 debugger, sessão manual de 2026-05-13 | confirmação runtime de `0x001d3a30 -> 0x001d37c8` |

## Resumo

`0x001d3a30` é o único caller estático encontrado para `0x001d37c8` via instrução `jal`. A sessão runtime de 2026-05-13 confirmou que esse caller é executado e alcança o dispatcher durante gameplay/load observado.

O caller preserva o argumento de entrada em `$s1`, lê:

```txt
entity_ptr = [s1 + 0x15c]
state_block_ptr = [entity_ptr + 0x800]
```

e chama o dispatcher apenas quando o campo:

```txt
[state_block_ptr + 0x04] == 1
```

O dispatcher é chamado com:

```txt
a0 = s1
jal 0x001d37c8
```

Isso encaixa com Rev.024: dentro do dispatcher, `s2` recebe o contexto e o fluxo volta a ler `[s2 + 0x15c]` e `[entity + 0x800]`.

## Instruções principais de `0x001d3a30`

| VA | Palavra | Instrução | Interpretação conservadora |
|---|---:|---|---|
| `0x001d3a30` | `0x27bdffb0` | `addiu sp,sp,-0x50` | prólogo |
| `0x001d3a44` | `0x0080882d` | `daddu s1,a0,zero` | preserva argumento de entrada em `$s1` |
| `0x001d3a50` | `0x8e23015c` | `lw v1,0x15c(s1)` | lê ponteiro via `[context + 0x15c]` |
| `0x001d3a54` | `0x8c720800` | `lw s2,0x800(v1)` | lê `state_block_ptr` via `[entity + 0x800]` |
| `0x001d3a58` | `0x8e420000` | `lw v0,0x0(s2)` | lê campo `+0x00` do state block |
| `0x001d3a5c` | `0x1053002c` | `beq v0,s3,0x001d3b10` | se campo `+0x00 == 1`, sai cedo |
| `0x001d3a64` | `0x8e42000c` | `lw v0,0x0c(s2)` | lê campo `+0x0c` |
| `0x001d3a68` | `0x50400005` | `beql v0,zero,0x001d3a80` | caminho depende de `[state_block_ptr + 0x0c]` |
| `0x001d3a70` | `0x0c074a6e` | `jal 0x001d29b8` | chamada direta se campo `+0x0c` não for zero |
| `0x001d3ae4` | `0x0c040a16` | `jal 0x00102858` | chamada comum antes do teste final |
| `0x001d3aec` | `0x8e22015c` | `lw v0,0x15c(s1)` | relê `[context + 0x15c]` |
| `0x001d3af0` | `0x8c440800` | `lw a0,0x800(v0)` | relê `state_block_ptr` |
| `0x001d3af4` | `0x8c830004` | `lw v1,0x04(a0)` | lê campo `+0x04` do state block |
| `0x001d3af8` | `0x38630001` | `xori v1,v1,0x0001` | testa se campo `+0x04 == 1` |
| `0x001d3afc` | `0x14600004` | `bne v1,zero,0x001d3b10` | se campo `+0x04 != 1`, sai |
| `0x001d3b04` | `0x0c074df2` | `jal 0x001d37c8` | chama o dispatcher confirmado |
| `0x001d3b08` | `0x0220202d` | `daddu a0,s1,zero` | delay slot: passa o contexto original como argumento |

## Fluxo mínimo confirmado

Pseudocódigo estrutural:

```c
void caller_001d3a30(void *context) {
    entity = *(context + 0x15c);
    state_block = *(entity + 0x800);

    if (*(state_block + 0x00) == 1) {
        return;
    }

    // Há caminhos auxiliares antes daqui, controlados por campos
    // do state block e por ponteiros de entity/context.
    call_00102858(context);

    entity = *(context + 0x15c);
    state_block = *(entity + 0x800);

    if (*(state_block + 0x04) == 1) {
        dispatcher_001d37c8(context);
    }
}
```

Notas:

- O pseudocódigo omite caminhos internos entre `0x001d3a64` e `0x001d3ae0`, porque eles exigem análise própria das chamadas `0x001d29b8`, `0x001d2bf0`, `0x001c05a8` e `0x001d2738`.
- O teste de `[state_block + 0x04] == 1` é direto: `lw`, `xori 1`, `bne`.
- A chamada para `0x001d37c8` passa o mesmo `context` recebido por `0x001d3a30`.

## Referência de dados a `0x001d3a30`

Busca byte-level por palavra little-endian `0x001d3a30` encontrou uma ocorrência direta:

| VA | Seção | Valor | Interpretação conservadora |
|---|---|---:|---|
| `0x002a3974` | `.data` | `0x001d3a30` | ponteiro de função ou entrada de tabela/record de callbacks |

Contexto local em `.data`:

| VA | Valor | Observação |
|---|---:|---|
| `0x002a396c` | `0x001d3b28` | ponteiro para função próxima |
| `0x002a3970` | `0x00000000` | zero/intercalador |
| `0x002a3974` | `0x001d3a30` | ponteiro para o caller runtime-confirmado |
| `0x002a3978` | `0x00000000` | zero/intercalador |
| `0x002a397c` | `0x001d27a8` | ponteiro para função que aloca/copia bloco e escreve `[entity + 0x800]` |
| `0x002a3988` | `0x45504f52` | bytes ASCII `ROPE` em little-endian |

O valor `ROPE` é evidência de rótulo/record local, mas sozinho não prova semântica de gameplay. O uso seguro aqui é apenas: a região de dados que contém `0x001d3a30` parece ser um record estruturado com ponteiros de função e metadados.

## Funções vizinhas na região de dados

### `0x001d3b28`

`0x001d3b28` acessa a mesma cadeia:

```txt
[context + 0x15c] -> entity
[entity + 0x800] -> state_block
```

e testa campos do state block:

| Offset | Uso observado |
|---:|---|
| `+0x00` | comparado com `1`; se igual, retorna cedo |
| `+0x04` | comparado com `1` por `xori`; controla caminho de saída/chamada |
| `+0x08` | lido via `state_base = state_block + 0x40`, equivalente a `[state_block + 0x48]`, mesmo campo de state id de Rev.024 |
| `+0x64` | lido via `[state_base + 0x24]`, equivalente a `[state_block + 0x64]` |

Chamadas diretas observadas:

| VA | Chamada |
|---|---|
| `0x001d3b9c` | `jal 0x001d12d8` |
| `0x001d3bb4` | `jal 0x001d2540` |
| `0x001d3bd4` | `j 0x0010ecb8` |

Interpretação provável: função irmã/update auxiliar associada ao mesmo state block. Não há evidência suficiente para nome forte.

### `0x001d27a8`

`0x001d27a8` chama `0x0013a0f8`, preserva o retorno em `$s6` e escreve:

```txt
[context + 0x15c] + 0x800 = s6
```

Instruções-chave:

| VA | Instrução | Interpretação conservadora |
|---|---|---|
| `0x001d27e8` | `jal 0x0013a0f8` | chamada que retorna ponteiro em `$v0` |
| `0x001d27f0` | `daddu s6,v0,zero` | preserva ponteiro retornado |
| `0x001d27f4` | `lw v1,0x15c(s5)` | lê entity/context interno |
| `0x001d2804` | `sw s6,0x800(v1)` | instala o ponteiro em `[entity + 0x800]` |
| `0x001d280c..0x001d284c` | sequência de `ld/sd` | copia dados de template para o bloco recém-retornado |
| `0x001d2858` | `sw v1,0x04(s6)` | inicializa campo `+0x04` do state block com valor vindo de `[arg + 0x30]` |
| `0x001d2944` | `sw s0,0x40(s3)` | escreve ponteiro em `[state_block + 0x40]` |
| `0x001d2978` | `swc1 f1,0x44(s3)` | escreve valor float em `[state_block + 0x44]` |

Interpretação provável: função de inicialização/alocação do state block usado por `0x001d3a30` e `0x001d37c8`. A evidência direta é a escrita em `[entity + 0x800]`.

## O que fica confirmado

1. `0x001d3a30` chama estaticamente `0x001d37c8` em `0x001d3b04`.
2. Não foi encontrado `jal` direto para `0x001d3a30` na busca byte-level da codificação `jal 0x001d3a30`.
3. Existe uma referência direta em `.data` para `0x001d3a30` em `0x002a3974`.
4. A sessão PCSX2 de 2026-05-13 confirmou que `0x001d3a30` executa e alcança `0x001d37c8`.
5. `0x001d3a30` passa seu argumento original para `0x001d37c8`.
6. `0x001d3a30` chama o dispatcher somente quando `[state_block_ptr + 0x04] == 1`.
7. `0x001d27a8` escreve um ponteiro retornado por `0x0013a0f8` em `[entity + 0x800]`.

## O que fica provável

1. A região `.data` em torno de `0x002a396c..0x002a3988` é um record estruturado com callbacks/funções relacionadas.
2. `0x001d27a8`, `0x001d3a30` e `0x001d3b28` fazem parte do mesmo subsistema/record, porque aparecem próximos em `.data` e acessam a mesma cadeia `[context + 0x15c] -> [entity + 0x800]`.
3. `[state_block_ptr + 0x04]` funciona como gate para permitir ou bloquear a chamada ao dispatcher.

## O que fica possível

1. O ASCII `ROPE` pode nomear o record/subsistema, mas isso permanece evidência fraca para semântica de gameplay.
2. `0x001d3b28` pode ser uma rotina irmã de update/finalização, mas o papel exato depende de análise das chamadas `0x001d12d8`, `0x001d2540` e `0x0010ecb8`.
3. `0x001d27a8` pode ser inicialização/construção do state block, mas o significado de `0x0013a0f8` ainda precisa ser verificado.

## O que é descartado

1. `0x001d3a30` não deve mais ser tratado como achado puramente estático: houve hit runtime e caminho até o dispatcher.
2. A ausência de `jal` direto para `0x001d3a30` não implica código morto; a referência em `.data` e o hit runtime sustentam chamada indireta/tabela.

## Próximo teste mínimo

Sem novo screenshot manual:

1. Fazer análise estática curta de quem referencia `0x002a3974` ou o início provável do record em `0x002a396c`.
2. Se for necessário runtime, preferir instrumentação ou cópia textual de registradores, não screenshots sucessivos.
3. Se investigar `0x001d27a8`, focar apenas em confirmar a origem/tamanho do bloco gravado em `[entity + 0x800]`.

## Veredito conservador

`0x001d3a30` é agora um caller runtime-confirmado do dispatcher `0x001d37c8`. Ele pertence a uma região de dados com ponteiros de função e usa a mesma cadeia de estado documentada em Rev.024. O próximo avanço técnico não deve ser mais gameplay manual, mas sim mapear a referência `.data` que contém `0x001d3a30` e a inicialização do ponteiro `[entity + 0x800]`.

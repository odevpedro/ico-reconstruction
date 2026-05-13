# rev.036 — Registration Path Survey

## Data

2026-05-13

## Objetivo

Executar os próximos passos definidos após Rev.035:

1. buscar quem carrega `descriptor_label -0x14` ou equivalente `record_start +0x40`;
2. confirmar o comportamento interno de `0x001b76f8` para o caso `ROPE`;
3. listar todos os callers de `0x0013f7a8`;
4. mapear funções em torno dos 17 leitores de `+0x46`;
5. investigar se há escritas estáticas ao campo `+0x46` da entry table.

## Escopo

Incluído:

- varredura binária do ELF para padrões de load em offsets específicos;
- análise completa da path de registro em `0x001b76f8`;
- lista completa de leitores e escritores de `+0x46`;
- callers completos de `0x0013f7a8` e `0x0013f778`;
- contexto das funções `0x00201e80` e `0x00240d??`;
- escritas ao campo `+0x46` em qualquer struct.

Excluído:

- runtime;
- gameplay;
- prova de que `0x001d3a30` foi registrado via qualquer caminho concreto;
- nomeação definitiva de subsistemas.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `research/elf/ghidra-rev035-entry-table-and-descriptor-correction.md` | hipóteses a verificar |
| `research/elf/ghidra-rev034-callback-signature-and-record-selection.md` | callers documentados de `0x001b76f8` |
| `research/elf/ghidra-rev033-node-callback-dispatch-chain.md` | cadeia de registro |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |

## 1. Busca por `lw rt,-0x14(rs)`

Varredura completa do ELF por `lw rt, -0x14(rs)` (opcode 0x23, imm16=0xffec):

```txt
Resultado: 0 hits em todo o segmento de código.
```

Interpretação:

- nenhum ponto do código carrega diretamente de `descriptor_label - 0x14` como offset relativo;
- `0x001d3a30` está armazenado em `0x002a3974` = `descriptor_label -0x14`, mas esse endereço é acessado de outra forma ou não é acessado estaticamente pelo offset `-0x14`.

## 2. Busca por `0x001d3a30` no ELF

Varredura por palavra alinhada `0x001d3a30` no ELF:

```txt
Único hit: file_off=0x001a4974, VA=0x002a3974
```

O valor `0x001d3a30` está armazenado apenas uma vez no binário, em `0x002a3974`.

Nenhum padrão `lui r,0x002a + lw/lbu offset=0x3974(r)` foi encontrado em varredura de split-immediate.

Interpretação:

- `0x001d3a30` não é carregado por construção absoluta de endereço no código estático verificado;
- o acesso deve ocorrer via cálculo de base + stride * index com um base que resulte em pointer para `0x002a3934` (record_start), não `0x002a3988` (descriptor_label).

## 3. Comportamento de `0x001b76f8` para ROPE

Trecho crítico:

```asm
0x001b7aa0: lw    a1,+36(s4)        -- a1 = [entry + 0x24]
0x001b7aa4: beq   a1,zero,0x001b7ac0 -- se +0x24 == 0, vai para caminho +0x40
0x001b7aa8: daddu a0,s3,zero
0x001b7aac: daddu a2,zero,zero
0x001b7ab0: jal   0x0013f7a8        -- registra [entry +0x24] se != 0
0x001b7ab4: addiu a3,zero,+19
0x001b7ab8: beq   zero,zero,0x001b7ad8 -- pula caminho +0x40
0x001b7abc: lbu   v1,+70(s4)        -- delay slot

0x001b7ac0: lw    a1,+64(s7)        -- a1 = [descriptor_label + 0x40]
0x001b7ac4: beq   a1,zero,0x001b7ad4 -- SE a1 == 0, PULA o registro inteiro
0x001b7ac8: daddu a2,zero,zero
0x001b7acc: jal   0x0013f7a8        -- registra [descriptor_label +0x40] se != 0
0x001b7ad0: addiu a3,zero,+19
```

Para ROPE:

- `descriptor_label = 0x002a3988`
- `descriptor_label +0x40 = 0x002a39c8 = 0x00000000` (zero, confirmado na tabela de Rev.035)
- portanto `beq a1,zero,0x001b7ad4` é tomado, e **nenhum registro ocorre**

Para que ROPE fosse registrado via `0x001b76f8`, seria necessário:

- que existisse uma entry com `+0x46 == 0x14` (ROPE index) E `+0x24 != 0`;
- esse `+0x24` conteria o callback concreto para aquela instância.

Rev.035 confirmou que nenhuma entry estática com `+0x46 == 0x14` foi encontrada no scan de `0x000..0x1ff`.

Conclusão:

```txt
0x001b76f8 não registra 0x001d3a30 para ROPE via o caminho estático verificado.
```

## 4. Todos os leitores de offset +0x46

Varredura completa por `lbu rt, +0x46(rs)`:

```txt
Total: 17 hits
```

| Endereço | Instrução |
|---|---|
| `0x00177304` | `lbu v1,+0x46(v0)` |
| `0x0018202c` | `lbu v1,+0x46(a2)` |
| `0x00190e88` | `lbu v0,+0x46(v1)` |
| `0x00191be0` | `lbu v0,+0x46(v1)` |
| `0x00192c30` | `lbu v0,+0x46(v1)` |
| `0x00192cc0` | `lbu v0,+0x46(a0)` |
| `0x001938c8` | `lbu v1,+0x46(v0)` |
| `0x001b72b4` | `lbu v1,+0x46(s2)` |
| `0x001b774c` | `lbu a0,+0x46(s4)` |
| `0x001b77c0` | `lbu a0,+0x46(s4)` |
| `0x001b7800` | `lbu a0,+0x46(s4)` |
| `0x001b79bc` | `lbu a1,+0x46(s4)` |
| `0x001b7abc` | `lbu v1,+0x46(s4)` |
| `0x001b7ad4` | `lbu v1,+0x46(s4)` |
| `0x001b7b38` | `lbu a1,+0x46(s4)` |
| `0x001b7bc4` | `lbu a2,+0x46(v1)` |
| `0x001e0154` | `lbu a3,+0x46(v1)` |

Os cinco leitores em `0x001b7??` estão dentro de `0x001b76f8`.

Os demais são em funções distintas ainda não mapeadas completamente.

## 5. Escritas ao offset +0x46

Varredura por `sb rt,+0x46(rs)`:

```txt
Total: 0 hits
```

Varredura por `sh rt,+0x46(rs)`:

```txt
0x00108a20: sh $v0,+0x46($sp)   -- escrita na pilha, não na entry table
0x0014066c: sh $v0,+0x46($s2)   -- escrita em struct apontada por $s2
```

Trecho de `0x0014066c`:

```asm
0x00140668: lw    v0,+40(sp)       -- v0 = valor da pilha
0x0014066c: sh    v0,+70(s2)       -- escreve v0 em [s2 + 0x46]
0x00140670: lw    v1,+36(sp)
0x00140674: sw    a0,+36(s2)
0x00140678: sw    v1,+24(s2)
0x0014067c: sw    zero,+76(s2)
0x00140680: beq   fp,zero,0x00140698
0x00140684: sh    zero,+72(s2)     -- zeroa [s2 + 0x48]
```

A função em torno de `0x00140668` copia múltiplos campos para a estrutura em `s2`, incluindo `+0x46` (tipo/descriptor index) e `+0x48` (flags). O valor de `+0x46` vem da pilha (`+40(sp)` = argumento passado acima do frame padrão).

Interpretação:

- `+0x46` NÃO é escrito por `sb` diretamente;
- é escrito como halfword `sh` a partir de um construtor/inicializador de struct;
- o campo `+0x46` na struct apontada por `s2` é preenchido externamente (dado carregado de outro contexto para a pilha).

Isso é consistente com a hipótese de Rev.035: o campo pode ser carregado de dados de room ou inicializado por uma rotina de criação de objeto.

## 6. Todos os callers de `0x0013f7a8`

Varredura por `jal 0x0013f7a8`:

| Callsite | Contexto de a1 | Origem do callback |
|---|---|---|
| `0x001b7ab0` | `lw a1,+36(s4)` = entry +0x24 | override de entry específica |
| `0x001b7acc` | `lw a1,+64(s7)` = descriptor_label +0x40 | slot do descriptor |
| `0x00201ed4` | não visível no trecho, contexto via entry table | sub-rotina de init com entry table |
| `0x00240e50` | `daddu a1,s5,zero` | argumento de função externa |
| `0x00240f90` | `daddu a1,s6,zero` | argumento de função externa |

Os callers `0x00240e50` e `0x00240f90` recebem o callback como argumento (`s5`, `s6`), indicando que a decisão de qual callback registrar ocorre em seus callers.

## 7. Relação `0x0013f778` — `0x0013f7a8`

Os dois são funções adjacentes e funcionalmente similares:

```asm
0x0013f778: daddu t0,a3,zero      -- salva a3 em t0
0x0013f77c: addiu sp,sp,-16
0x0013f780: andi  a3,a2,0x00ff
0x0013f784: sd    ra,+0(sp)
0x0013f788: daddu a2,a1,zero      -- a2 = callback (a1)
0x0013f78c: addiu t1,zero,+6144
0x0013f790: jal   0x0013f3f0      -- delega ao armazenador de node+0x1c
0x0013f794: daddu a1,a0,zero

0x0013f7a8: daddu v0,a3,zero      -- variante que preserva a3 em v0
0x0013f7ac: addiu sp,sp,-16
0x0013f7b0: andi  a3,a2,0x00ff
0x0013f7b4: daddu t1,t0,zero
0x0013f7b8: daddu a2,a1,zero
0x0013f7bc: sd    ra,+0(sp)
0x0013f7c0: daddu t0,v0,zero
0x0013f7c4: jal   0x0013f3f0
```

Ambas passam o argumento `a1` como `a2` para `0x0013f3f0`, que armazena em `node +0x1c`.

A diferença é que `0x0013f7a8` preserva `a3` em `t0`/`t1`, enquanto `0x0013f778` usa `t1=6144` (0x1800) fixo.

## 8. Função `0x00201e80` — caller alternativo via entry table

Trecho:

```asm
0x00201e90: lui   a0,0x002a
0x00201e94: addiu a0,a0,+19528    -- a0 = 0x002a4c48 (entry table base)
0x00201e98: lw    v1,+8(s0)       -- v1 = [s0 + 8] (index?)
0x00201e9c: mult  v1,v1,v0        -- v1 = index * stride (v0 = stride, not shown)
0x00201ea0: addu  v1,v1,a0        -- v1 = entry_base + index * stride = entry ptr
0x00201ea4: lhu   v0,+64(v1)      -- v0 = halfword from [entry + 0x40]
0x00201ea8: dsll  t0,v0,10        -- t0 = v0 << 10
0x00201eac: bne   t0,zero,0x00201ecc
                                   -- se [entry+0x40] != 0, toma caminho 0x0013f7a8
0x00201eb4: daddu a0,s0,zero
0x00201eb8: daddu a2,zero,zero
0x00201ebc: jal   0x0013f778      -- caminho alternativo: 0x0013f778
0x00201ec0: addiu a3,zero,+19

0x00201ecc: daddu a0,s0,zero
0x00201ed0: daddu a2,zero,zero
0x00201ed4: jal   0x0013f7a8      -- caminho principal: a1 não visível no trecho
0x00201ed8: addiu a3,zero,+19
```

Esta função usa `entry +0x40` como halfword (não como ponteiro de callback) para decidir o caminho de registro. Isso é uma perspectiva diferente da de `0x001b76f8`, onde `descriptor_label +0x40` era comparado como ponteiro.

O argumento `a1` de `0x0013f7a8` em `0x00201ed4` não está visível no trecho imediato; deve vir de um registro preservado de antes da branch.

## 9. Bases usadas para acessar o descriptor table

Bases `0x002a31b8` encontradas no ELF: 12 sites.

| Site (lui) | Site (addiu) | Uso subsequente |
|---|---|---|
| `0x001295f8` | `0x00129604` | `addu s0,v0,t2` — descriptor ptr em s0 |
| `0x00182034` | `0x0018203c` | `addu v0,a2,v1` — descriptor ptr, depois `lw a3,+0x30(v0)` |
| `0x001827e4` | `0x001827f0` | `addu v0,v0,v1` |
| `0x001828e8` | `0x001828f0` | `addu v0,v0,v1` |
| `0x001a8eac` | `0x001a8eb8` | `addu v1,v1,a0` |
| `0x001ab230` | `0x001ab23c` | `addu v1,v1,a0` |
| `0x001adba0` | `0x001adbac` | `addu v0,v1,v0`; depois `sh zero,+2(v0)` e `sh a0,+4(v0)` — ESCRITA nos campos do descriptor |
| `0x001ae48c` | `0x001ae490` | `addu v1,v1,v0` |
| `0x001b7bbc` | `0x001b7bc0` | `addu s2,a1,a0` |
| `0x001b7f44` | `0x001b7f4c` | `addu s1,v1,s2` |
| `0x001b7fec` | `0x001b7ff4` | `addu s0,s0,v0` |
| `0x001d4b70` | `0x001d4b78` | `addu a2,v1,a2` |

O site em `0x001adba0` é o único que escreve nos campos do descriptor:

```asm
0x001adbb0: addu  v0,v1,v0        -- v0 = descriptor_label
0x001adbd8: sh    zero,+2(v0)     -- descriptor_label + 2 = 0
0x001adbe0: sh    a0,+4(v0)       -- descriptor_label + 4 = a0
```

Isso sugere uma rotina de inicialização ou atualização parcial de um descriptor.

O campo `descriptor_label -0x14` (= callback slot = `0x001d3a30` para ROPE) NÃO é escrito por nenhum desses 12 sites.

## O que fica confirmado

1. `0x001b76f8` pula o registro de callback para ROPE porque `descriptor_label +0x40 == 0` e a comparação em `0x001b7ac4` descarta o `jal`.
2. Nenhum `lw rt,-0x14(rs)` existe no ELF; o slot `descriptor_label -0x14` não é acessado diretamente por esse padrão.
3. `0x001d3a30` aparece apenas uma vez no ELF (em `0x002a3974`); não há construção split-immediate que aponte para esse endereço.
4. Existem exatamente 5 callers de `0x0013f7a8`: `0x001b7ab0`, `0x001b7acc`, `0x00201ed4`, `0x00240e50`, `0x00240f90`.
5. `0x0013f778` e `0x0013f7a8` são funções adjacentes com a mesma lógica central (ambas delegam a `0x0013f3f0`).
6. O campo `+0x46` não tem `sb` writer estático; o único writer encontrado é `sh` em `0x0014066c`, em um construtor de struct.
7. O site `0x001adba0` escreve nos campos `+2` e `+4` do descriptor, mostrando que há inicialização parcial de descriptors em runtime.

## O que fica provável

1. ROPE é registrado via `entry[+0x24]` override quando a entry concreta existe (possivelmente criada em runtime a partir de dados de room).
2. O campo `entry[+0x46]` é preenchido por uma rotina de criação de objeto (como em `0x0014066c`) e não por store direto na entry table estática.
3. As funções `0x00240e50` e `0x00240f90` recebem o callback como argumento externo; seus callers determinam qual callback é passado.

## O que fica possível

1. A função `0x00201e80` pode ser um caminho de registro para ROPE se um caller externo construir a entry correta com `+0x24 = 0x001d3a30`.
2. O site `0x001adba0` é parte de uma sequência de inicialização de descriptor que pode incluir o slot `descriptor_label -0x14` em versões runtime não capturadas estaticamente.
3. `0x0013f778` é chamado em contextos onde o t0 original de `0x0013f7a8` não está disponível; pode ser um caminho mais simples de registro com t1=6144 fixo.

## O que permanece desconhecido

1. O caller de `0x00240e50` e `0x00240f90` — quem decide o valor de s5/s6 que vai como callback.
2. O argumento a1 de `0x00201ed4` — o trecho visível não mostra de onde vem.
3. Se o campo `entry[+0x24]` é preenchido com `0x001d3a30` por algum path de room loading.
4. Se `0x001adba0` faz parte de um init que eventualmente cobre o slot `-0x14`.
5. O significado dos campos `descriptor_label +2` e `+4` (escritos por `0x001adba0`).

## O que é descartado

1. Que `0x001b76f8` registre `0x001d3a30` para ROPE via o caminho descriptor_label+0x40. Zero-guard em `0x001b7ac4` confirma que o registro é pulado.
2. Que exista acesso estático direto a `0x002a3974` por split-immediate.
3. Que `sb` seja o mecanismo de escrita para `entry[+0x46]`.

## Próximo teste mínimo

Sem gameplay:

1. Rastrear o caller de `0x00240e50` e `0x00240f90` para entender quem fornece s5/s6.
2. Analisar `0x00201e80` completamente: qual é o frame de entrada e de onde vem o argumento `a1` no caminho `0x00201ed4`.
3. Analisar `0x001adba0` e arredores para ver se o init do descriptor inclui o slot `-0x14`.
4. Examinar os 12 outros leitores de `+0x46` (fora de `0x001b76f8`) para verificar se algum deles usa o valor `0x14` (ROPE) em um contexto de registro.

Com gameplay:

1. Breakpoint em `0x0013f3f0` para capturar `a2` (o callback sendo registrado) e `a1` (o objeto).
2. Verificar se algum path por `0x00240e50` ou `0x00240f90` passa `0x001d3a30` como a1.

## Veredito conservador

Rev.036 confirma que `0x001b76f8` não é o registrador de `0x001d3a30` para ROPE. O zero-guard em `0x001b7ac4` é prova direta. Existem cinco callers de `0x0013f7a8`, mas os três fora de `0x001b76f8` têm argumentos a1 não tracejáveis estaticamente. O campo `+0x46` não tem writer por `sb`; o único `sh` writer encontrado é um construtor de struct. O registro estático de `0x001d3a30` permanece não resolvido, mas o conjunto de candidatos está agora delimitado: `0x00201ed4`, `0x00240e50`, `0x00240f90` e qualquer path dinâmico de room loading.

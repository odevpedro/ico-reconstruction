# rev.031 — Record Callback Dispatchers

## Data

2026-05-13

## Objetivo

Avançar sem gameplay a partir das revisões 026-030, procurando quem chama indiretamente os slots de callback observados no record `.data` `ROPE`.

O objetivo mínimo é responder:

```txt
Os slots +0x38, +0x40 e +0x48 dos records têm dispatchers estáticos identificáveis?
```

## Escopo

Incluído:

- busca estática por padrões `lw reg, offset(base)` seguidos por `jalr reg`;
- foco nos offsets `+0x38`, `+0x40` e `+0x48`;
- funções que invocam callbacks via esses offsets;
- relação com o record `ROPE` e com o callback runtime-confirmado `0x001d3a30`.

Excluído:

- gameplay;
- screenshots;
- assets;
- `DATA.DF`;
- nomeação forte de fases de lifecycle;
- decompilação completa de todos os `jalr` do ELF.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `AGENTS.md` | disciplina de evidência e cautela |
| `research/elf/ghidra-rev026-rope-record-table-context.md` | layout do record `ROPE` |
| `research/elf/ghidra-rev027-rope-state-block-initializer.md` | papel confirmado de `ROPE +0x48` |
| `research/elf/ghidra-rev030-provider-caller-survey.md` | contexto amplo do provider usado por callbacks `+0x48` |
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções locais |

## Método

Foi feita uma varredura byte-level no ELF local, decodificando instruções MIPS suficientes para reconhecer:

```asm
lw    callback_reg, offset(base_reg)
jalr  ra, callback_reg
```

Também foi usada uma forma simples de tracking de registrador para capturar cópias diretas via `addu/or/daddu` com `zero`.

Offsets priorizados:

```txt
+0x38
+0x40
+0x48
```

Limite importante: esta varredura encontra padrões simples. Um caller pode existir por caminho mais indireto, por callback armazenado temporariamente, por tabela intermediária, ou por registrador derivado de outra rotina.

## Resultado curto

Foram encontrados três hits limpos onde um slot relevante é carregado e chamado por `jalr` no mesmo fluxo local:

| Load | Slot | Base | Call | Função aproximada | Interpretação conservadora |
|---|---:|---|---|---|---|
| `0x0013d16c` | `+0x38` | `s0` | `0x0013d170` | `0x0013d140` | dispatcher de callback `+0x38` via tabela de records |
| `0x0013fcac` | `+0x48` | `s0` | `0x0013fcb8` | `0x0013fc00` | dispatcher iterativo de callback `+0x48` |
| `0x001b7960` | `+0x38` | `s7` | `0x001b796c` | `0x001b76f8` | caller especializado de callback `+0x38` |

Além disso, `0x0013fc00` contém um segundo caminho de `+0x48` no record/list head:

```asm
0x0013fc38: lw    v0,+0x48(s2)
0x0013fc44: jalr  ra,v0
0x0013fc48: daddu a0,s2,zero
```

Esse caminho tem branch-likely/null-check entre o load e o `jalr`, por isso a varredura mais restritiva não o contou no resumo automático final, mas a sequência é clara no disassembly local.

## Dispatcher de `+0x48` em `0x0013fc00`

Trecho relevante:

```asm
0x0013fc08: lw    s2,-0x671c(gp)
...
0x0013fc38: lw    v0,+0x48(s2)
0x0013fc44: jalr  ra,v0
0x0013fc48: daddu a0,s2,zero
...
0x0013fc80: lw    s0,+0x0(v0)
...
0x0013fcac: lw    v0,+0x48(s0)
0x0013fcb8: jalr  ra,v0
0x0013fcbc: daddu a0,s0,zero
```

Leitura conservadora:

- `s2` começa como um ponteiro obtido de global via `gp`;
- o primeiro callback `+0x48(s2)` é chamado com `a0 = s2`;
- depois a função percorre uma estrutura/lista;
- para cada `s0` aceito pelos filtros, chama `+0x48(s0)` com `a0 = s0`.

Isso fortalece a interpretação de Rev.027 e Rev.030: callbacks no slot `+0x48` podem ser chamados por um dispatcher iterativo amplo.

## Relação com `ROPE +0x48`

Rev.026 mostrou:

| Record | Slot | Valor |
|---|---:|---|
| `ROPE` | `+0x48` | `0x001d27a8` |

Rev.027 mostrou que `0x001d27a8`:

- chama o provider `0x0013a0f8`;
- recebe um payload pointer;
- copia `0x90` bytes de template;
- instala o payload em `[entity + 0x800]`;
- inicializa `[state_block + 0x48]` como `0`.

Rev.031 acrescenta que existe dispatcher estático que chama callbacks `+0x48` em records/itens percorridos.

Modelo revisado:

```txt
record/list traversal
-> load [record + 0x48]
-> jalr callback(record)
-> for ROPE, callback can be 0x001d27a8
-> 0x001d27a8 installs [entity + 0x800]
-> later 0x001d3a30 may call 0x001d37c8
```

Esta cadeia ainda não prova que `0x0013fc00` chamou especificamente o record `ROPE` em runtime. Ela prova que existe um mecanismo estático plausível e direto para invocar callbacks `+0x48` de records com o mesmo layout.

## Dispatcher de `+0x38` em `0x0013d140`

Trecho relevante:

```asm
0x0013d14c: jal   0x00100410
0x0013d150: daddu s0,a0,zero
0x0013d154: lui   v1,0x6a
0x0013d158: sll   v0,v0,2
0x0013d15c: addiu v1,v1,+0x6f30
0x0013d164: addu  v0,v0,v1
0x0013d168: lw    s0,+0x0(v0)
0x0013d16c: lw    v0,+0x38(s0)
0x0013d170: jalr  ra,v0
0x0013d174: nop
0x0013d178: lw    v1,+0x40(s0)
```

Leitura conservadora:

- `0x00100410` produz um índice;
- esse índice seleciona uma entrada em uma tabela em torno de `0x006a6f30`;
- a entrada selecionada é tratada como record/descriptor;
- o slot `+0x38` é chamado indiretamente;
- logo depois o código lê `+0x40` do mesmo record, mas neste trecho não o chama por `jalr`.

Isso confirma que `+0x38` também é um slot de callback invocado por dispatcher estático.

## Caller especializado de `+0x38` em `0x001b76f8`

Trecho relevante:

```asm
0x001b7960: lw    v0,+0x38(s7)
0x001b7964: beq   v0,zero,0x001b797c
0x001b7968: daddu a0,sp,zero
0x001b796c: jalr  ra,v0
0x001b7970: daddu a1,s5,zero
```

Leitura conservadora:

- `+0x38(s7)` é callback opcional;
- a chamada passa `a0 = sp` e `a1 = s5`;
- o contexto de chamada é diferente de `0x0013d140`;
- portanto este é provavelmente um caminho especializado, não necessariamente o dispatcher principal de lifecycle.

## Situação do slot `+0x40`

Rev.026 mostrou:

| Record | Slot | Valor |
|---|---:|---|
| `ROPE` | `+0x40` | `0x001d3a30` |

Rev.025 e Rev.024 confirmaram que `0x001d3a30` executou em runtime e alcançou:

```txt
0x001d3a30 -> 0x001d37c8
```

Na varredura de Rev.031, porém, não apareceu um padrão limpo:

```asm
lw callback_reg,+0x40(record_reg)
jalr ra,callback_reg
```

Os hits próximos a `+0x40` foram ambíguos ou pertencem a outros padrões, por exemplo:

| Load | Observação |
|---|---|
| `0x0013d178: lw v1,+0x40(s0)` | leitura/check após chamada de `+0x38`, sem `jalr` direto |
| `0x001b7ac0: lw a1,+0x40(s7)` | valor passado como argumento para `0x0013f7a8`, não chamado diretamente |
| `0x002030bc`, `0x00203ff0`, `0x00204038` | cargas de `+0x40(sp)`, provavelmente stack/local callback context |
| `0x00250e50: lw a1,+0x40(s2)` | valor passado para rotina direta antes de outro callback via `v1` |
| `0x00251d08: lw a2,+0x40(a0)` | parece acessar tabela/lista interna, com callback carregado de outro offset |

Interpretação conservadora:

- `ROPE +0x40 = 0x001d3a30` continua confirmado como ponteiro de callback por dados e por runtime;
- o dispatcher estático que invoca esse slot ainda não foi isolado;
- esse caller pode usar caminho mais indireto do que o padrão simples `lw +0x40` seguido de `jalr`;
- também é possível que o slot `+0x40` seja entregue como argumento a outra rotina que faz o dispatch.

## Byte-level / instruction-level findings

| Achado | Evidência |
|---|---|
| `+0x48` é chamado por `0x0013fc00` | `lw v0,+0x48(s0)` em `0x0013fcac`; `jalr ra,v0` em `0x0013fcb8` |
| `+0x48` também aparece no head `s2` | `lw v0,+0x48(s2)` em `0x0013fc38`; `jalr ra,v0` em `0x0013fc44` |
| `+0x38` é chamado por `0x0013d140` | `lw v0,+0x38(s0)` em `0x0013d16c`; `jalr ra,v0` em `0x0013d170` |
| `+0x38` é chamado por `0x001b76f8` | `lw v0,+0x38(s7)` em `0x001b7960`; `jalr ra,v0` em `0x001b796c` |
| `+0x40` não teve dispatcher limpo isolado | varredura por `lw +0x40` próximo de `jalr` não produziu padrão record-slot direto |

## O que fica confirmado

1. Existe um dispatcher estático em `0x0013fc00` que chama callbacks carregados de `+0x48`.
2. O dispatcher `0x0013fc00` passa o próprio record/objeto como `a0` para o callback `+0x48`.
3. Existe um dispatcher estático em `0x0013d140` que chama callbacks carregados de `+0x38`.
4. Existe pelo menos um caller especializado em `0x001b76f8` que também chama `+0x38`.
5. `ROPE +0x48 = 0x001d27a8` agora tem um mecanismo estático plausível de chamada por slot.
6. O caller estático de `ROPE +0x40 = 0x001d3a30` ainda não foi localizado por padrão simples de `jalr`.

## O que fica provável

1. `+0x48` é um slot de callback de inicialização/ativação/registro em pelo menos parte dos records, mas o nome exato ainda não está provado.
2. `+0x38` também é callback de record, possivelmente para outra fase ou contexto.
3. `+0x40` é callback de record em `ROPE`, mas seu dispatcher usa padrão diferente ou passa por outra rotina.

## O que fica possível

1. `0x0013fc00` pode ser um loop amplo de lifecycle/update de records ou entidades.
2. `0x0013d140` pode selecionar um record por índice corrente e executar uma fase diferente via `+0x38`.
3. O slot `+0x40` pode ser chamado por um dispatcher que primeiro copia o ponteiro, armazena em stack, ou o passa como argumento para helper.
4. O slot `+0x40` pode ter papel ligado ao caminho runtime que executou `0x001d3a30`, mas isso ainda requer caller estático ou runtime trace melhor.

## O que permanece desconhecido

1. Qual rotina chama especificamente `ROPE +0x40`.
2. Se `0x0013fc00` chamou `ROPE +0x48` em runtime na sessão observada.
3. O nome original ou papel exato dos slots `+0x38`, `+0x40` e `+0x48`.
4. A tabela ou lista exata que contém o record `ROPE` no momento em que `0x0013fc00` itera.
5. Se todos os records de stride `0x64` obedecem ao mesmo contrato de callback.

## O que é descartado

1. Dizer que `0x001d3a30` não tem caller porque não há `jal` direto. Rev.025 já confirmou runtime, e Rev.026 mostrou ponteiro em `.data`.
2. Dizer que `+0x48` é apenas um campo de dados passivo. Há chamadas indiretas estáticas desse offset.
3. Dizer que `+0x40` tem caller estático resolvido nesta revisão. Ele continua pendente.
4. Nomear os slots como init/update/destroy de forma definitiva. A ordem de lifecycle ainda não está provada.

## Próximo teste mínimo

Sem gameplay:

1. Expandir a análise de `0x0013fc00` para identificar a estrutura/lista que fornece `s2` e `s0`.
2. Rastrear `0x0013f7a8`, porque `0x001b7ac0` passa um valor carregado de `+0x40(s7)` como argumento para essa rotina.
3. Fazer uma varredura dataflow mais ampla para o valor `0x001d3a30`, procurando padrões onde `+0x40` seja carregado, armazenado temporariamente e chamado depois.
4. Comparar `+0x40` de `BARREL`, `CHAIN`, `FLEVER` e `ROPE` para buscar um dispatcher comum por assinatura de função.

## Veredito conservador

Rev.031 avança o projeto sem gameplay ao confirmar dispatchers estáticos para slots de callback dos records. O slot `+0x48` agora tem um caminho de chamada claro em `0x0013fc00`, compatível com o papel de `ROPE +0x48 = 0x001d27a8` como instalador do state block. O slot `+0x38` também tem callers estáticos claros. O slot `+0x40`, onde fica o callback runtime-confirmado `0x001d3a30`, continua sendo a principal lacuna: sua existência como callback é forte, mas o dispatcher que o chama ainda não foi isolado estaticamente.

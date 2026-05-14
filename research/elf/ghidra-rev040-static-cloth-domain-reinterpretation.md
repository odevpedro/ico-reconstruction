# rev.040 — Static Cloth-Domain Reinterpretation

## Data

2026-05-14

## Objetivo

Reinterpretar estaticamente o dispatcher `0x001d37c8`, o callback
`0x001d3a30` e funções auxiliares próximas sob a correção de domínio trazida
pelo cross-reference com ICO-decomp: **cloth/chain simulation**, não
entity/gameplay state machine.

## Escopo

Incluído:

- dispatcher `0x001d37c8` e blocos internos `0..4`;
- callback runtime-confirmado `0x001d3a30`;
- funções próximas chamadas no fluxo: `0x001d2738`, `0x001d29b8`,
  `0x001d2bf0`, wrappers `0x001d2538`, `0x001d2540`, `0x001d2548`;
- template em `0x004c46b0`;
- cross-reference com ICO-decomp PAL via `symbol_addrs.txt` e `ICO-PAL.yaml`;
- caveat sobre `fumi/ios/thread.c` e `0x0013f7a8`.

Excluído:

- runtime/emulador;
- `DATA.DF`;
- `.gcm`;
- textura/asset extraction;
- nomes fortes de gameplay;
- prova de matching 1:1 USA/PAL.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `AGENTS.md` | regras de cautela e fonte de verdade atual |
| `research/ico-decomp-cross-reference-2026-05-14.md` | correção de domínio e source-file range |
| `research/elf/ghidra-rev039-cloth-domain-correction.md` | fechamento de domínio anterior |
| `research/elf/ghidra-rev024-internal-state-block-semantics.md` | análise dos cinco blocos internos |
| `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | callback `0x001d3a30` e relação com dispatcher |
| `research/elf/ghidra-rev027-rope-state-block-initializer.md` | inicializador `0x001d27a8` e template |
| `research/elf/ghidra-rev029-state-block-provider-deeper-static.md` | provider allocator/pool-like |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | gap de registro do callback |
| `/tmp/ICO-decomp/config/symbol_addrs.txt` | símbolos PAL vizinhos |
| `/tmp/ICO-decomp/config/ICO-PAL.yaml` | mapping de módulo/seções |
| `/tmp/ICO-decomp/ico2/fumi/ios/thread.c` | fonte C de `thread.c` no ICO-decomp |
| `.local/extracted/SCUS_971.13.elf` | bytes locais USA |
| Capstone 5.0.7 | disassembly local de ranges específicos |

## Evidência usada

Esta revisão usa:

- bytes locais do ELF USA;
- disassembly local via Capstone;
- notas validadas anteriores;
- symbol table e YAML públicos do ICO-decomp PAL.

Esta revisão **não** usa:

- execução runtime nova;
- dados de sala/layout;
- arquivos proprietários extraídos;
- nomes inferidos por IA como evidência.

## Caveat sobre ICO-decomp

O checkout local de ICO-decomp em `/tmp/ICO-decomp` contém `fumi/ios/thread.c`
como C, mas **não contém `sugipon/src/clothAnimation.c` decompilado como C**.
No YAML, `clothAnimation` está marcado como `asm`:

```txt
[0x0ced48, asm, sugipon/src/clothAnimation]
```

Logo, a evidência de domínio para `0x001d37c8` e `0x001d3a30` é
**source-file range / symbol-neighborhood evidence**, não leitura de C
decompilado de `clothAnimation.c`.

Também há um caveat na correlação com `thread.c`: `0x0013f7a8` cai perto de
`iosThreadStart` na tabela PAL, mas seu corpo USA não corresponde
semanticamente ao C de `iosThreadStart`.

No ICO-decomp:

```c
void iosThreadStart(struct IosThreadInfo const *th_info)
{
    StartThread(th_info->threadId, th_info->args);
}
```

Já `0x0013f7a8` rearranja argumentos e delega para `0x0013f3f0`.
Portanto, `0x0013f7a8 == iosThreadStart` deve ser tratado como descartado ou,
no mínimo, como correlação PAL/USA inválida para semântica.

## Cross-reference corrigido

Os endereços USA/PAL usados nesta família de notas coincidem numericamente
no range de interesse, mas a correlação continua sendo por contexto e símbolos
vizinhos, não por matching de função.

| Endereço | Relação no ICO-decomp | Interpretação segura |
|---:|---|---|
| `0x001d35f0` | `InitCloth4D` | símbolo vizinho anterior |
| `0x001d37c8` | `InitCloth4D + 0x1d8`, antes de `GetChainNodeGlobalQuaternion` | dispatcher não nomeado dentro do range cloth |
| `0x001d3a30` | `InitCloth4D + 0x440`, antes de `GetChainNodeGlobalQuaternion` | callback não nomeado dentro do range cloth |
| `0x001d3ad8` | `GetChainNodeGlobalQuaternion` | símbolo vizinho posterior |
| `0x001d3b80` | `MoveChainExtendedWeight` | função chain/cloth vizinha |
| `0x001d3b98` | `InitChainVelocity` | função chain/cloth vizinha |

Correção de sinal: frases anteriores como `InitCloth4D(-472)` são ambíguas.
O dispatcher está **depois** de `InitCloth4D` por `0x1d8` bytes.

## Byte-level findings

### Template do state block

Rev.027 identificou template copiado por `0x001d27a8`:

```txt
template source = 0x004c46b0
copy size       = 0x90
destination     = pointer returned by 0x0013a0f8
```

Verificação local:

| Offset | VA | Valor | Uso conhecido |
|---:|---:|---:|---|
| `+0x04` | `0x004c46b4` | `0x00000000` | sobrescrito por `[arg + 0x30]` em `0x001d2858` |
| `+0x2c` | `0x004c46dc` | `0x3f800000` | float `1.0` |
| `+0x44` | `0x004c46f4` | `0x0000012c` | contador usado por `state_1_block` |
| `+0x48` | `0x004c46f8` | `0x00000000` | state id inicial |
| `+0x60` | `0x004c4710` | `0x00000000` | campo usado por estados posteriores |
| `+0x64` | `0x004c4714` | `0x00000000` | campo escrito por `state_0_block` |

### Jump table

| Entry | VA | Valor | Bloco |
|---:|---:|---:|---|
| 0 | `0x00618fb0` | `0x001d3818` | `cloth_state_0_default_or_ready_check` |
| 1 | `0x00618fb4` | `0x001d3844` | `cloth_state_1_timed_vector_blend` |
| 2 | `0x00618fb8` | `0x001d391c` | `cloth_state_2_resource_setup` |
| 3 | `0x00618fbc` | `0x001d39e0` | `cloth_state_3_wait_for_resource` |
| 4 | `0x00618fc0` | `0x001d3a10` | common epilogue/no-op state |

Os nomes acima são rótulos de trabalho para esta revisão. Não são nomes
originais nem nomes de gameplay.

## Instruction-level findings

### Wrappers locais de evento/estado

O range próximo a `0x001d2538` contém wrappers curtos:

```asm
0x001d2538: j     0x001d12a8
0x001d253c: addiu a1,zero,0x30

0x001d2540: j     0x001d12a8
0x001d2544: addiu a1,zero,0x31

0x001d2548: j     0x001d12a8
0x001d254c: addiu a1,zero,0x32
```

Interpretação conservadora:

- `0x001d2538`, `0x001d2540`, `0x001d2548` são wrappers para uma função comum
  `0x001d12a8`, passando IDs constantes `0x30`, `0x31`, `0x32`;
- por vizinhança PAL, `0x001d12a8` fica dentro/ao lado do range
  `DispCloth4D`/`DispCloth4DWithAdd`;
- os wrappers parecem sinais/eventos internos de cloth, não estados de gameplay.

### `0x001d2738` — helper que ativa campos do state block

Trecho inicial:

```asm
0x001d2754: lw   v0,0x15c(s1)
0x001d2758: lw   s0,0x800(v0)
0x001d275c: jal  0x001d2650
0x001d2760: lw   a1,0x14(s0)
0x001d2768: sw   zero,0x0c(s0)
0x001d276c: sw   1,0x10(s0)
0x001d2774: sw   1,0x08(s0)
0x001d2778: lw   a0,0x15c(s1)
0x001d277c: jal  0x00105f00
0x001d2780: addiu a0,a0,0x130
0x001d279c: j    0x0010d530
```

Findings:

| Campo | Ação |
|---:|---|
| `[state_block + 0x08]` | escrito com `1` |
| `[state_block + 0x0c]` | escrito com `0` |
| `[state_block + 0x10]` | escrito com `1` |
| `[state_block + 0x14]` | passado para `0x001d2650` |
| `[context+0x15c]+0x130` | destino de cópia/vetor por `0x00105f00` |
| `[context+0x15c]+0x150` | passado para `0x0010d530` |

Interpretação cloth-domain provável:

- helper de ativação/reinicialização parcial do state block;
- parece preparar flags/estado interno e copiar/normalizar vetores ligados ao
  contexto/entity;
- é chamado por `0x001d3a30` quando uma condição externa específica é satisfeita.

### `0x001d27a8` — initializer/provider consumer

Rev.027 e Rev.029 continuam válidas, mas a nomenclatura muda:

```txt
0x001d27a8
-> provider 0x0013a0f8(a1=0x90, file="src/item.c", line=0x1b2)
-> copies cloth-domain template 0x004c46b0
-> writes returned payload to [context+0x15c]+0x800
```

O metadado `"src/item.c"` em `0x00618f68` ainda impede concluir que o arquivo
original desse helper seja `clothAnimation.c`. O mais seguro é:

```txt
0x001d27a8 is a record initializer that installs the 0x90-byte payload used by
the cloth-domain dispatcher. Its allocation/debug metadata says src/item.c.
```

### `0x001d29b8` — transform/chain sampling helper

`0x001d3a30` chama `0x001d29b8` quando `[state_block + 0x0c] != 0`.

Trechos relevantes:

```asm
0x001d29dc: lw   v0,0x15c(s5)
0x001d29e0: lw   s4,0x800(v0)
0x001d29e4: lw   a0,0x14(s4)
0x001d29e8: lw   v0,0x15c(a0)
0x001d29ec: lw   v1,0x604(v0)
...
0x001d2a18: lw   a1,0x0c(v1)
0x001d2a1c: jal  0x00118648
...
0x001d2a48: jal  0x0010d830
...
0x001d2a5c: jal  0x00102870
...
0x001d2bc4: jal  0x00104240
```

Findings:

| Campo / fonte | Uso |
|---|---|
| `[state_block + 0x14]` | objeto/contexto secundário |
| `[[state_block+0x14]+0x15c]+0x604` | escolhe um de dois caminhos |
| `[[state_block+0x14]+0x15c]+0x0c` | base de dados vetoriais/transform |
| `[[state_block+0x14]+0x15c]+0x10` | base de dados vetoriais/transform |
| stack temporário | usado para vetores/quaternions/matrizes |

Interpretação cloth-domain provável:

- helper de amostragem/cópia de transformações de chain/skeleton para alimentar
  atualização do cloth;
- o ramo em `[... + 0x604]` sugere variante de fonte de transform;
- não há evidência para gameplay state.

### `0x001d2bf0` — atualização geométrica/colisão ampla

`0x001d3a30` chama `0x001d2bf0` quando `[state_block + 0x0c] == 0` e
`[state_block + 0x08] != 0`.

Trecho inicial:

```asm
0x001d2c2c: lw   v0,0x15c(a2)
0x001d2c34: sw   zero,0x5f4(v0)
0x001d2c3c: lw   v1,0x800(v0)
0x001d2c4c: sw   zero,0x5f8(v0)
0x001d2c54: jal  0x00105f00
0x001d2c58: addiu a1,a1,0x130
0x001d2c68: jal  0x0019f530
...
0x001d2f3c: jal  0x00243ad0
0x001d2fc0: jal  0x00168a40
0x001d2fdc: jal  0x00166118
...
0x001d3038: mult a1,0x14
0x001d3040: addu v0,v0,0x004d4188
```

Findings:

| Campo / constante | Uso |
|---|---|
| `[context+0x15c]+0x5f4` | zerado no início |
| `[context+0x15c]+0x5f8` | zerado no início |
| `[context+0x15c]+0x130` | copiado para stack |
| `[context+0x15c]+0x150` | usado em operações vetoriais posteriores |
| `[state_block + 0x04]` | usado como índice em tabela stride `0x14` |
| `0x004d4188` | base de tabela de records de 0x14 bytes |
| `0x32f` | valor sentinela comparado contra entrada da tabela |

Interpretação cloth-domain provável:

- helper pesado de atualização geométrica/collision-like;
- mistura vetores do contexto com tabela indexada por `[state_block+0x04]`;
- pode encerrar/alterar o estado do block quando certas verificações passam
  (`sw 1,[state_block+0x00]`, zerando `[context+0x15c]+0x74` e `[context]+0x16c`
  em trechos posteriores já vistos).

Confiança:

- média para "geometric/collision-like";
- baixa para qualquer nome exato.

### `0x001d3a30` — callback/update gate

Fluxo reclassificado:

```txt
0x001d3a30(context)
-> state_block = [[context+0x15c]+0x800]
-> if [state_block+0x00] == 1: return
-> if [state_block+0x0c] != 0: call 0x001d29b8
-> else if [state_block+0x08] != 0: call 0x001d2bf0
-> else optional path via [entity+0x00], type 0x11, 0x001c05a8, 0x001d2738
-> common call 0x00102858(context)
-> if [state_block+0x04] == 1: call 0x001d37c8(context)
```

Reinterpretação:

- `0x001d3a30` é melhor entendido como callback/update wrapper do record que
  coordena helpers de cloth/chain e só chama o dispatcher quando o gate
  `[state_block+0x04] == 1` permite;
- os campos `+0x08`, `+0x0c`, `+0x14`, `+0x74`, `+0x16c` parecem controlar
  transições/flags de simulação, não estados de gameplay.

### `0x001d37c8` — dispatcher interno de cloth-domain

O dispatcher continua estruturalmente igual ao de Rev.023/Rev.024:

```txt
context
-> [context + 0x15c]
-> [entity + 0x800]
-> [state_block + 0x48]
-> jump table 0x00618fb0
```

O que muda é a leitura semântica dos blocos:

| Bloco | Ação estrutural confirmada | Reinterpretação conservadora |
|---|---|---|
| `0x001d3818` | chama `0x001f2148`; se retorno != 0, chama wrappers `0x001d2538`/`0x001d2540`, escreve state id `1` e `[+0x64]=1` | ready/default check; dispara sinais/eventos cloth internos |
| `0x001d3844` | usa `0x004c4750`, FPU, globals `0x00274ec0`, decrementa `[+0x44]`, troca para state `2` | blend/timer vector update |
| `0x001d391c` | cria/configura recurso `0x1b8`, chama helpers stage/vector, escreve state `3` | setup/resource creation para uma fase visual/geométrica |
| `0x001d39e0` | espera consulta em `[state_block+0x60]`; se pronta, escreve state `4`, marca `[state_block+0x00]=1` | wait/finish gate |
| `0x001d3a10` | epílogo comum | no-op/finished state |

As antigas leituras "Yorda/capture/menu/death/AI" ficam descartadas para este
range.

## Tabela de hipóteses revisada

| Item | Hipótese antiga | Reinterpretação Rev.040 | Confiança |
|---|---|---|---|
| `0x001d37c8` | entity/gameplay state dispatcher | dispatcher interno de cloth-domain | alta para domínio; média para papel exato |
| `0x001d3a30` | generic entity update callback | callback/update wrapper de cloth record | alta para domínio; média para papel exato |
| `state_1_block` | estado temporizado genérico | blend/timer de vetor/cloth usando template `0x004c4750` | média |
| `state_2_block` | setup de gameplay | setup de recurso/geometry stage para cloth-domain | média-baixa |
| `0x001d29b8` | helper desconhecido | transform/chain sampling helper | média |
| `0x001d2bf0` | helper desconhecido | atualização geométrica/collision-like | média-baixa |
| `0x0013f7a8` | possivelmente `iosThreadStart` | wrapper de registro/storage, não `iosThreadStart` | alta para descartar equivalência com C |

## O que fica confirmado

1. O ICO-decomp local não contém `clothAnimation.c` decompilado como C; o módulo
   está marcado como `asm` no YAML.
2. `0x001d37c8` e `0x001d3a30` ficam no source-file range
   `sugipon/src/clothAnimation` por symbol/YAML context.
3. `0x001d37c8` vem depois de `InitCloth4D`, não antes.
4. `0x001d3a30` chama o dispatcher somente quando `[state_block+0x04] == 1`.
5. `0x001d2738` escreve flags em `[state_block+0x08]`, `[+0x0c]`, `[+0x10]`
   e usa offsets vetoriais do contexto.
6. `0x001d27a8` instala o payload de `0x90` bytes em `[entity+0x800]` e copia
   o template `0x004c46b0`.
7. `0x001d29b8` usa `[state_block+0x14]` como contexto secundário e lê dados
   vetoriais/transform de offsets como `+0x0c`, `+0x10`, `+0x604`.
8. `0x001d2bf0` zera `[context+0x15c]+0x5f4/+0x5f8`, usa offsets
   `+0x130/+0x150`, e consulta tabela stride `0x14` em `0x004d4188`.
9. O C de `iosThreadStart` no ICO-decomp não corresponde ao corpo USA de
   `0x0013f7a8`.

## O que fica provável

1. `0x001d3a30` é um update wrapper do subsistema cloth/chain para o record
   associado ao label local `ROPE`.
2. O dispatcher `0x001d37c8` controla uma sequência curta de fases internas da
   simulação/efeito cloth, não uma state machine de entidade de alto nível.
3. `0x001d29b8` alimenta a simulação com transforms/chain data.
4. `0x001d2bf0` lida com atualização geométrica/collision-like mais pesada.
5. Os wrappers `0x001d2538/40/48` disparam eventos/sinais cloth internos por
   IDs `0x30/0x31/0x32`.

## O que fica possível

1. O label `ROPE` pode ser um record de objeto que usa cloth/chain internamente,
   mas isso não equivale a gameplay rope.
2. Os estados `0..4` podem corresponder a uma sequência visual/efeito específico
   dentro do cloth-domain, e não a fases gerais de todo cloth system.
3. A tabela em `0x004d4188` pode conter parâmetros por subtipo/variante de cloth
   indexados por `[state_block+0x04]`.
4. `0x001d2bf0` pode ser collision-like, constraint-like, ou ambos; a evidência
   atual mostra geometria/vetor/tabela, não nome definitivo.

## O que permanece desconhecido

1. O nome original de `0x001d37c8`.
2. O nome original de `0x001d3a30`.
3. A correspondência exata de cada state id com uma fase física.
4. O mecanismo exato que registra `0x001d3a30` em `node+0x1c`.
5. Se a entrada runtime que registra `0x001d3a30` vem de `entry[+0x24]` ou de
   `0x00201e70`.
6. O significado exato do label `ROPE`.
7. Se USA e PAL têm pequenas diferenças relevantes nesse miolo.

## O que é descartado

1. `0x001d37c8` como máquina de estado Yorda/capture/menu/death/AI.
2. `0x001d3a30` como callback de gameplay genérico sem vínculo cloth.
3. `0x0013f7a8` como `iosThreadStart` confirmado.
4. Uso de nomes `wind`, `collision`, `constraint` como conclusões fortes para
   states `1..3`; podem ser hipóteses, não nomes.
5. Frases que afirmem que o C de `clothAnimation.c` foi lido no ICO-decomp.

## Próximo teste mínimo sem emulador

1. Fazer uma correlação USA/PAL mais forte para o miolo `0x001d35f0..0x001d3ad8`
   usando bytes/instruções, se o ELF PAL estiver disponível localmente.
2. Mapear a tabela `0x004d4188` no ELF USA apenas como metadados/valores
   estruturais, sem entrar em assets.
3. Reanalisar `0x001d2bf0` por blocos básicos, porque ela é o helper mais rico
   e pode explicar melhor a semântica dos campos `+0x5f4`, `+0x5f8`,
   `+0x130`, `+0x150`.
4. Criar um `.sym` local para PCSX2 com nomes neutros:

```txt
001d37c8 cloth_domain_dispatcher_candidate
001d3a30 cloth_domain_update_callback_candidate
001d27a8 cloth_domain_payload_initializer_candidate
001d29b8 cloth_domain_transform_helper_candidate
001d2bf0 cloth_domain_geometry_update_candidate
0013f3f0 callback_storage_node_writer
0013f7a8 callback_registration_wrapper
```

## Próximo teste mínimo com emulador

O gap principal ainda exige runtime:

```txt
break 0x0013f7a8
capture a0, a1, a2, a3, ra
filter/observe when a3 == 0x13
check whether a1 == 0x001d3a30
map ra back to the actual callsite
```

## Veredito conservador

Rev.040 não resolve o registro de `0x001d3a30`, mas avança sem emulador ao
reclassificar a análise estática dentro do domínio correto. O conjunto
`0x001d27a8`, `0x001d29b8`, `0x001d2bf0`, `0x001d3a30` e `0x001d37c8` forma
um cluster coerente de payload/state-block, transform/chain sampling,
geometry/collision-like update e dispatch interno de cloth-domain.

O projeto deve parar de tratar esses endereços como state machine de gameplay.
A lacuna restante é mais estreita: descobrir por qual caminho genérico de
callback o update candidate `0x001d3a30` é registrado em `node+0x1c`.

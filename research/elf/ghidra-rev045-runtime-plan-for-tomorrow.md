# rev.045 — Runtime Plan for Tomorrow

## Resumo executivo

Esta revisão é um checkpoint técnico para retomar a investigação em runtime.
Ela não adiciona uma nova frente estática e não resolve a origem de `a1`.

A pergunta central para amanhã é:

```txt
De onde vem o argumento a1 passado para 0x001d27a8(a0,a1),
e qual estrutura/descriptor ele representa em runtime?
```

O estado atual é conservador:

- `0x001d27a8` é um inicializador do domínio cloth que consome `a0` e `a1`;
- `a1` é necessário porque `[a1+0x30]` é copiado para `[payload+0x04]`;
- `[payload+0x04]` depois indexa a tabela `0x004d4188`;
- os caminhos estáticos conhecidos não explicam satisfatoriamente quem fornece
  `a1`;
- Rev.044 considera a análise estática desse caminho esgotada;
- o próximo passo correto é capturar runtime com PCSX2/debugger.

## Onde paramos na Rev.044

Rev.044 testou se `0x001d27a8` poderia ser explicado por uma callback chain
estática já conhecida:

```txt
ROPE +0x48 = 0x001d27a8
-> staged/copiado para outra estrutura
-> chamado depois com a0 e a1
```

O resultado foi negativo para os caminhos analisados:

| Caminho | Resultado Rev.044 |
|---|---|
| `0x0013f3f0 -> node+0x1c` | armazena callback, mas nao preserva ponteiro auxiliar para `a1` |
| `0x0013fb70` | chama `node+0x1c` passando somente `a0=s2` |
| `0x0013fc00` slot `+0x48` | pode chamar callbacks de `+0x48`, mas nao prepara `a1` |
| scan de `lw ...,0x48(...)` | nao encontrou staged path util para `0x001d27a8(a0,a1)` |
| `0x00129660` | constructor-like e prepara `a1`, mas foi excluido para ROPE na leitura de Rev.044 porque lê `+0x58`, nao `+0x48` |

Tambem foi registrado o simbolo runtime:

```txt
cloth_constructor_like_descriptor_plus58
```

para marcar o caminho constructor-like em `0x00129660`.

## O que está confirmado

| Fato | Evidencia |
|---|---|
| `0x001d27a8` preserva `a0` em `s5` | prologo em Rev.043 |
| `0x001d27a8` preserva `a1` em `s4` | prologo em Rev.043 |
| `0x001d27a8` lê `[s4+0x30]` | `0x001d2850: lw v1,0x30(s4)` |
| `0x001d27a8` escreve `[payload+0x04]` | `0x001d2858: sw v1,0x04(s6)` |
| `[payload+0x04]` indexa a tabela `0x004d4188` | Rev.041 |
| `0x001d37c8` é dispatcher cloth-domain | Rev.038/040, corrigindo leituras anteriores |
| `0x001d3a30` chama o dispatcher `0x001d37c8` | Rev.025 e revisões posteriores |
| `0x001d27a8`, `0x001d3a30`, `0x001d3b28` aparecem no record `ROPE` por convenção record-start | Rev.026 |
| o caminho `node+0x1c -> 0x0013fb70` passa somente `a0` | Rev.033/044 |

O ponto mais importante confirmado para amanhã:

```txt
0x001d27a8 precisa de a1 valido, porque dereferencia [a1+0x30].
```

## O que foi descartado

| Hipotese descartada | Motivo |
|---|---|
| `0x0013fb70` explica `0x001d27a8(a0,a1)` por completo | dispatcher passa somente `a0=s2` |
| `0x0013f3f0` preserva um argumento auxiliar para `a1` | stores observados nao carregam esse ponteiro ate o dispatcher |
| os dois paths simples em `0x0013fc00` preparam `a1` | ambos setam `a0` no delay slot, sem setup visivel de `a1` |
| qualquer load de `+0x48` e necessariamente callback | varios usos sao flags/copia numerica |
| `0x00129660` prova chamada ROPE para `0x001d27a8` | em Rev.044, a leitura estatica de ROPE `+0x58` nao apontou para `0x001d27a8` |
| origem de `[initializer_arg+0x30]` ja foi encontrada | nao ha produtor estatico confirmado |

Esses descartes sao locais ao escopo analisado. Eles nao provam ausencia de
outros caminhos indiretos, overlays, dados runtime ou setup dinamico.

## O que ficou provável

1. `0x001d27a8` e um inicializador cloth-domain associado ao payload de
   `+0x800`.
2. `a1` provavelmente aponta para uma estrutura de inicializacao, descriptor,
   entry ou bloco runtime que contem ao menos um campo significativo em `+0x30`.
3. A callback chain `0x0013f7a8 -> 0x0013f3f0 -> node+0x1c -> 0x0013fb70`
   continua relevante para callbacks de update como `0x001d3a30`, mas nao
   explica sozinha o inicializador `0x001d27a8`.
4. O runtime deve revelar mais rapido a origem de `a1` do que novas varreduras
   estaticas amplas.

## Por que a análise estática foi esgotada

A analise estatica foi considerada esgotada para esta pergunta porque:

1. nao existe `jal 0x001d27a8` direto no ELF local;
2. a unica referencia direta a `0x001d27a8` encontrada no segmento carregado e
   dado de descriptor/record, nao xref de codigo;
3. os dispatchers estaticos simples de slot `+0x48` nao preparam `a1`;
4. o staged path `node+0x1c` chama callbacks com apenas `a0`;
5. scans por `lw ...,0x48(...)` seguidos de store ou `jalr` nao encontraram um
   caminho util que preserve `a1`;
6. o constructor-like path `0x00129660` e relevante, mas Rev.044 nao conseguiu
   liga-lo estaticamente a ROPE/`0x001d27a8`;
7. a proxima informacao necessaria e dinamica: valor real de `ra`, `a1`, regiao
   de memoria apontada por `a1`, e callsite real no momento da chamada.

Conclusao: continuar procurando estaticamente pode gerar mais hipoteses, mas
nao resolve a pergunta principal sem observar a execucao.

## Pergunta principal para runtime

```txt
De onde vem o argumento a1 passado para 0x001d27a8(a0,a1),
e qual estrutura/descriptor ele representa em runtime?
```

Subperguntas:

| Pergunta | Captura necessaria |
|---|---|
| Quem chama `0x001d27a8`? | `ra` na entrada de `0x001d27a8` |
| `a1` e valido? | dump de memoria em torno de `a1` |
| `a1` aponta para static data, heap, stack ou outra regiao? | classificacao de faixa de endereco |
| `[a1+0x30]` e o variant index esperado? | word em `a1+0x30` e comparacao com tabela `0x004d4188` |
| `a1+0x58` reabre caminho descriptor-like? | word em `a1+0x58`, se `a1` for valido |
| o callsite e `0x00129660` ou outro caminho? | `ra`, `pc`, backtrace manual e breakpoints auxiliares |

## Breakpoints recomendados

| Prioridade | Endereco | Condicao | Objetivo |
|---:|---:|---|---|
| 1 | `0x001d27a8` | sempre | capturar entrada real do inicializador e origem aparente de `a1` |
| 2 | callsite real de `0x001d27a8` | depois de identificado por `ra` | capturar registradores antes do `jalr` ou chamada indireta |
| 3 | `0x00129660` | callback target igual a `0x001d27a8`, se debugger permitir | testar constructor-like `+0x58` |
| 4 | `0x0013fb70` | `v0 == 0x001d27a8` ou `v0 == 0x001d3a30` | separar initializer path de update callback chain |
| 5 | `0x0013fc44` | `v0 == 0x001d27a8` | testar primeiro slot `+0x48 -> jalr` |
| 6 | `0x0013fcb8` | `v0 == 0x001d27a8` | testar segundo slot `+0x48 -> jalr` |
| 7 | `0x001d37c8` | apos `0x001d27a8` ter instalado payload | confirmar que payload instalado entra no dispatcher cloth |
| 8 | watchpoint em payload `+0x04` | quando payload vivo for conhecido | ver quem altera variant/mode depois da inicializacao |
| 9 | watchpoint em descriptor/record `+0x58` vivo | apenas se endereco vivo for identificado | detectar writes runtime para campo constructor-like |

Detalhes por breakpoint:

| Breakpoint | Objetivo | Registradores | Memoria | Pergunta respondida | Interpretacao A/B/C |
|---|---|---|---|---|---|
| `0x001d27a8` | entrada do initializer | todos os obrigatorios abaixo | `a0`, `a1`, `a1+0x30`, `a1+0x58` | quem chamou e o que e `a1`? | `ra` conhecido: callsite resolvido; `a1` invalido: modelo precisa rever chamada; `a1` valido: classificar estrutura |
| callsite via `ra` | confirmar delay slot/setup | `a0..a3`, `v0`, `v1`, `s*`, `t*` | base usada para callback | chamada direta/indireta? | se callsite novo, mapear funcao; se ja analisado, confirmar lacuna; se runtime-patched, buscar writer |
| `0x00129660` | testar constructor-like | `v0`, `s0`, `s1`, `s3`, `s6`, `sp`, `a0`, `a1` | `[s0+0x58]`, `sp+0x20`, `sp+0x50` | esse path chama `0x001d27a8`? | `v0=0x001d27a8`: caminho reaberto; `v0=0`: Rev.044 fortalecido; outro callback: comparar records |
| `0x0013fb70` | callback chain node | `v0`, `s0`, `s2`, `a0`, `ra` | node `s0`, object `s2` | node chain chama qual callback? | `v0=0x001d3a30`: update chain; `v0=0x001d27a8`: surpresa, capturar `a1`; outro: ignorar para esta pergunta |
| `0x0013fc44` / `0x0013fcb8` | slot `+0x48` direto | `v0`, `s0`, `s2`, `a0`, `a1`, `ra` | base record/entry | `+0x48` chama initializer? | se sim e `a1` valido, origem herdada; se `a1` lixo, path improvavel; se nunca bate, foco no callsite real |
| `0x001d37c8` | dispatcher cloth | `a0`, `v0`, `v1`, `s1`, `s2`, `s3` | `[context+0x15c]`, `[entity+0x800]`, `[payload+0x04]`, `[payload+0x48]` | payload inicializado e usado? | payload bate com `0x001d27a8`: cadeia fechada; outro payload: ha multiplas instancias; sem payload: timing errado |

## Captura obrigatória em `0x001d27a8`

Sempre que o breakpoint em `0x001d27a8` disparar, registrar:

```txt
pc
ra
sp
a0
a1
a2
a3
v0
v1
s0
s1
s2
s3
s4
s5
s6
s7
t0
t1
t2
t3
```

Tambem registrar:

| Item | Como capturar | Motivo |
|---|---|---|
| memoria em torno de `a0` | dump pequeno antes/depois de `a0` | confirmar objeto/contexto |
| memoria em torno de `a1` | dump pequeno antes/depois de `a1` | classificar estrutura inicializadora |
| word em `a1+0x30` | se `a1` for valido | valor copiado para `[payload+0x04]` |
| word em `a1+0x58` | se `a1` for valido | testar relacao descriptor/constructor-like |
| possiveis ponteiros dentro de `a1` | scan visual dos words | distinguir tabela, heap object, stack frame |
| regiao de `a1` | `.data`, `.rodata`, heap, stack, desconhecida | interpretar origem |
| caller via `ra` | resolver `ra-8` ou callsite equivalente | achar funcao/callsite real |
| funcao/callsite | anotar endereco e instrucao anterior | preparar breakpoint secundario |

Classificacao minima de regiao:

| Faixa | Interpretacao inicial |
|---|---|
| proxima de `0x002a31b8`, `0x002a3934`, `0x002a3988`, `0x002a4c48` | descriptor/entry/static data possivel |
| proxima de stack atual | argumento construido em stack |
| heap/alocacao runtime | estrutura dinamica; rastrear allocator/writer |
| `.text` | provavelmente ponteiro incorreto para struct; revisar captura |
| endereco invalido/baixo | captura incompleta ou chamada nao esperada |

## Plano de dumps de memória

No primeiro hit de `0x001d27a8`, fazer dumps pequenos e repetiveis:

| Dump | Tamanho sugerido | Objetivo |
|---|---:|---|
| `a0-0x20 .. a0+0x200` | `0x220` bytes | contexto/objeto que entra no initializer |
| `a1-0x20 .. a1+0x100` | `0x120` bytes | estrutura de inicializacao |
| `[a0+0x15c]-0x20 .. +0x900` | se valido | entity e slot `+0x800` |
| `sp .. sp+0x100` | `0x100` bytes | checar se `a1` aponta para stack ou foi derivado dela |
| `0x002a3934 .. 0x002a3998` | uma vez | comparar ROPE record-start estatico |
| `0x002a3988 .. 0x002a39ec` | uma vez | comparar ROPE descriptor-label estatico |
| payload retornado/alocado | depois de `0x001d27f0` ou fim da funcao | confirmar `[payload+0x04]` e `[payload+0x48]` |

Se o debugger permitir watchpoint depois de identificar enderecos vivos:

| Watchpoint | Condicao | Objetivo |
|---|---|---|
| write em `[payload+0x04]` | payload conhecido | confirmar writer inicial e writers posteriores |
| write em `[payload+0x48]` | payload conhecido | observar state id usado por `0x001d37c8` |
| write em `[descriptor_or_runtime_struct+0x58]` | estrutura viva conhecida | testar se `+0x58` e patchado dinamicamente |

## Como interpretar os cenários possíveis

### Cenário A

`a1` aponta para uma estrutura estatica em `.data` ou `.rodata`.

Interpretacao conservadora:

- favorece descriptor/table estatico;
- comparar `a1` com regioes `0x002a31b8`, `0x002a3934`, `0x002a3988` e
  `0x002a4c48`;
- se `a1+0x30` contem valor pequeno compatível com indice `0..7`, fortalece a
  ponte com Rev.041;
- se `ra` tambem aponta para callsite conhecido, a cadeia pode ser documentada
  como static+runtime confirmada.

### Cenário B

`a1` aponta para heap/runtime allocation.

Interpretacao conservadora:

- favorece construcao dinamica;
- nao invalida ROPE/cloth, mas desloca a pergunta para quem alocou/preencheu a
  estrutura;
- proximo passo seria rastrear allocator/writer do bloco de `a1`;
- watchpoints nos campos `a1+0x30` e possivelmente `a1+0x58` seriam mais uteis
  do que nova varredura global.

### Cenário C

`a1 + 0x58 = 0`.

Interpretacao conservadora:

- enfraquece a associacao direta com a hipotese constructor-like `+0x58`,
  conforme Rev.044;
- ainda pode ser estrutura valida se o campo relevante for `+0x30`;
- nao descarta `0x001d27a8`, porque a funcao confirmadamente consome `+0x30`,
  nao `+0x58`;
- proximo passo deve focar no caller via `ra`, nao no campo `+0x58`.

### Cenário D

`a1 + 0x58 != 0` e aponta para regiao compativel com ROPE/callback.

Interpretacao conservadora:

- reabre a hipotese de descriptor compativel com ROPE;
- se o valor for ponteiro para `.text`, comparar com callbacks conhecidos:
  `0x001d27a8`, `0x001d3a30`, `0x001d3b28`, vizinhos de BARREL/CHAIN;
- se o valor apontar para `.data`, verificar se e ponteiro indireto para
  descriptor/entry;
- ainda requer confirmar o callsite real antes de promover a hipotese.

### Cenário E

`ra` aponta para callsite diferente dos ja analisados.

Interpretacao conservadora:

- indica que a analise estatica perdeu um caminho, ou que a chamada ocorre por
  mecanismo indireto/callback nao coberto pela varredura;
- registrar `ra`, instrucoes ao redor do caller e registradores vivos;
- criar um breakpoint no caller e repetir a captura antes da chamada;
- so depois disso atualizar a cadeia estatica.

## Próximo passo após cada resultado

| Resultado runtime | Proximo passo |
|---|---|
| `ra` aponta para `0x00129660` | documentar constructor-like como callsite runtime e rastrear origem do indice/estrutura |
| `ra` aponta para `0x0013fc44` ou `0x0013fcb8` | entender de onde `a1` vem nesse dispatcher, pois ele nao e preparado localmente |
| `ra` aponta para `0x0013fb70` | verificar se `a1` e acidental/herdado; esse resultado seria inesperado para initializer |
| `ra` aponta para callsite novo | mapear funcao localmente e criar nova revisao tecnica |
| `a1` em `.data/.rodata` | comparar com descriptor table e entry table; evitar gameplay naming |
| `a1` em heap | rastrear allocator/writer e watchpoints em `a1+0x30` |
| `[a1+0x30]` fora de `0..7` | revisar se o valor e mesmo usado como variant index ou se ha transformacao posterior |
| `[a1+0x30]` em `0..7` | comparar com entry usada em `0x004d4188` e payload `+0x04` |
| payload `+0x04` nao bate com `[a1+0x30]` | revisar timing da captura ou existencia de writer posterior |

## Checklist para amanhã

1. Carregar o `.sym` atualizado no PCSX2/debugger, incluindo
   `cloth_constructor_like_descriptor_plus58`.
2. Colocar breakpoint principal em `0x001d27a8`.
3. No primeiro hit, registrar todos os registradores obrigatorios.
4. Classificar `a1`: static data, heap, stack ou desconhecido.
5. Dumpar memoria em torno de `a0`, `a1`, `sp` e `[a0+0x15c]`.
6. Registrar `[a1+0x30]` e `[a1+0x58]`, se `a1` for valido.
7. Resolver o caller por `ra`.
8. Criar breakpoint no callsite identificado.
9. Se o callsite for `0x00129660`, capturar `s0`, `[s0+0x58]`, `s6`,
   `sp+0x20` e `sp+0x50`.
10. Se o callsite for `0x0013fb70`, capturar node `s0`, object `s2` e
    callback `v0`.
11. Depois do initializer, confirmar payload em `[entity+0x800]`.
12. Se possivel, quebrar em `0x001d37c8` para verificar uso posterior do
    payload pelo dispatcher cloth.
13. So escrever nova revisao depois de ter `ra`, `a1`, dumps e interpretacao
    de regiao.

## Veredito

O estado atual nao permite concluir estaticamente a origem de `a1` em
`0x001d27a8(a0,a1)`.

Rev.044 encerra a frente estatica imediata: os caminhos conhecidos foram
testados e nenhum explica satisfatoriamente a chamada com dois argumentos. O
runtime deve responder primeiro quem chama `0x001d27a8` e para qual regiao
`a1` aponta.

O teste minimo de amanha e simples: breakpoint em `0x001d27a8`, captura completa
dos registradores, dump de `a1`, classificacao da memoria e resolucao do caller
via `ra`. Sem esses dados, qualquer nova explicacao para `a1` continua sendo
hipotese.

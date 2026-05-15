# Runtime Capture Automation Plan

## Objetivo

Reduzir o trabalho manual da sessao PCSX2/debugger para a investigacao de:

```txt
0x001d27a8(a0,a1)
```

Pergunta principal:

```txt
De onde vem a1, e qual estrutura/descriptor ele representa em runtime?
```

Este plano nao tenta resolver a pergunta estaticamente. Ele define um formato
de captura repetivel para que a parte manual seja limitada a:

1. iniciar o jogo no PCSX2 debugger;
2. carregar os simbolos;
3. ativar breakpoints;
4. salvar/copiar logs e dumps;
5. entregar os arquivos para analise.

## Escopo

Incluido:

- breakpoints prioritarios;
- campos obrigatorios de log;
- formato CSV e JSONL;
- nomes de arquivos;
- fluxo semi-automatizado caso o PCSX2 nao exponha scripting suficiente;
- fluxo automatizavel caso seja possivel exportar registradores/memoria;
- criterios de interpretacao rapida.

Excluido:

- investigar novas funcoes;
- alterar revisoes antigas;
- resolver `DATA.DF`, `.gcm`, assets, TM2 ou gameplay naming;
- afirmar resultado runtime antes da captura.

## Arquivos de saida esperados

Criar uma pasta local para cada sessao:

```txt
.local/runtime-captures/2026-05-14-cloth-a1/
```

Arquivos recomendados:

| Arquivo | Conteudo |
|---|---|
| `events.csv` | uma linha por breakpoint hit |
| `events.jsonl` | uma linha JSON por breakpoint hit, mais completa |
| `memory-index.csv` | indice dos dumps de memoria salvos |
| `notes.md` | observacoes manuais curtas da sessao |
| `dump_*.bin` | dumps binarios de memoria |
| `dump_*.txt` | dumps hex/text se o debugger exportar texto |

Esses arquivos ficam em `.local/` para nao entrarem no historico por engano.

## Breakpoints prioritarios

### BP1 - Initializer entry

```txt
address: 0x001d27a8
symbol: cloth_domain_initializer_candidate
condition: none
priority: highest
```

Objetivo:

- capturar a chamada real;
- resolver `ra`;
- classificar `a1`;
- dumpar memoria em torno de `a0`, `a1`, `sp`.

Registrar sempre:

```txt
pc,ra,sp,a0,a1,a2,a3,v0,v1,s0,s1,s2,s3,s4,s5,s6,s7,t0,t1,t2,t3
```

Memoria:

| Expressao | Tamanho | Nome sugerido |
|---|---:|---|
| `a0-0x20` | `0x220` | `dump_<n>_bp1_a0.bin` |
| `a1-0x20` | `0x120` | `dump_<n>_bp1_a1.bin` |
| `sp` | `0x100` | `dump_<n>_bp1_sp.bin` |
| `[a0+0x15c]-0x20`, se valido | `0x920` | `dump_<n>_bp1_entity.bin` |

Campos derivados:

```txt
a1_plus_30_word
a1_plus_58_word
a1_region
caller_guess
```

### BP2 - Constructor-like descriptor +0x58

```txt
address: 0x00129660
symbol: cloth_constructor_like_descriptor_plus58
condition: v0 == 0x001d27a8, if supported
priority: high
```

Objetivo:

- testar se o path constructor-like realmente chama `0x001d27a8`;
- capturar `s0`, `[s0+0x58]`, `sp+0x20`, `sp+0x50`;
- ver se `a1=sp+0x20` explica `[a1+0x30]`.

Registrar:

```txt
pc,ra,sp,a0,a1,a2,a3,v0,v1,s0,s1,s3,s6,t2,t3
```

Memoria:

| Expressao | Tamanho | Nome sugerido |
|---|---:|---|
| `s0-0x20` | `0xa0` | `dump_<n>_bp2_s0_descriptor.bin` |
| `sp` | `0x100` | `dump_<n>_bp2_sp.bin` |
| `sp+0x20` | `0x80` | `dump_<n>_bp2_arg.bin` |

Campos derivados:

```txt
s0_plus_58_word
sp_plus_50_word
callback_target
```

### BP3 - Node callback dispatcher

```txt
address: 0x0013fb70
symbol: node_callback_dispatcher
condition: v0 == 0x001d27a8 || v0 == 0x001d3a30, if supported
priority: medium
```

Objetivo:

- separar update callback chain de initializer path;
- confirmar se `node+0x1c` chama `0x001d3a30`;
- detectar caso inesperado em que `node+0x1c` chame `0x001d27a8`.

Registrar:

```txt
pc,ra,sp,a0,a1,v0,s0,s2
```

Memoria:

| Expressao | Tamanho | Nome sugerido |
|---|---:|---|
| `s0` | `0x94` | `dump_<n>_bp3_node.bin` |
| `s2-0x20` | `0x220` | `dump_<n>_bp3_object.bin` |

### BP4 - Direct +0x48 dispatch paths

```txt
addresses:
  0x0013fc44
  0x0013fcb8
condition: v0 == 0x001d27a8, if supported
priority: medium
```

Objetivo:

- verificar se os dispatchers `+0x48` chamam `0x001d27a8`;
- capturar se `a1` esta valido ou incidental.

Registrar:

```txt
pc,ra,sp,a0,a1,a2,a3,v0,v1,s0,s2
```

Memoria:

| Expressao | Tamanho | Nome sugerido |
|---|---:|---|
| `s0-0x20`, se valido | `0xa0` | `dump_<n>_bp4_s0.bin` |
| `s2-0x20`, se valido | `0xa0` | `dump_<n>_bp4_s2.bin` |
| `a1-0x20`, se valido | `0x120` | `dump_<n>_bp4_a1.bin` |

### BP5 - Cloth dispatcher

```txt
address: 0x001d37c8
symbol: cloth_domain_dispatcher_candidate
condition: after BP1 hit, if possible
priority: lower until initializer is captured
```

Objetivo:

- confirmar que o payload instalado pelo initializer e usado pelo dispatcher;
- capturar `[payload+0x04]` e `[payload+0x48]`.

Registrar:

```txt
pc,ra,sp,a0,v0,v1,s1,s2,s3
```

Memoria/expressoes:

```txt
entity = [a0+0x15c]
payload = [entity+0x800]
payload_plus_04 = [payload+0x04]
payload_plus_48 = [payload+0x48]
```

## Formato CSV minimo

Arquivo:

```txt
.local/runtime-captures/2026-05-14-cloth-a1/events.csv
```

Header:

```csv
hit_id,timestamp,frame,pc,bp_name,ra,sp,a0,a1,a2,a3,v0,v1,s0,s1,s2,s3,s4,s5,s6,s7,t0,t1,t2,t3,a1_region,a1_plus_30,a1_plus_58,callback_target,caller_guess,notes
```

Exemplo:

```csv
1,00:03:12.440,,0x001d27a8,bp1_initializer,0x00129668,0x01ffef00,0x003a1000,0x01ffef20,0x00000000,0x00000000,0x001d27a8,0x00000000,0x00000000,0x003a1000,0x00000000,0x00000000,0x00000000,0x003a1000,0x00000000,0x00000000,0x002a31b8,0x00000064,stack,0x00000001,0x00000000,0x001d27a8,0x00129660,
```

Regras:

- campos desconhecidos ficam vazios;
- valores hex sempre com prefixo `0x`;
- nao misturar decimal e hex no mesmo campo;
- cada hit de breakpoint recebe `hit_id` crescente.

## Formato JSONL recomendado

Arquivo:

```txt
.local/runtime-captures/2026-05-14-cloth-a1/events.jsonl
```

Uma linha por evento:

```json
{"hit_id":1,"timestamp":"00:03:12.440","pc":"0x001d27a8","bp_name":"bp1_initializer","registers":{"ra":"0x00129668","sp":"0x01ffef00","a0":"0x003a1000","a1":"0x01ffef20","a2":"0x00000000","a3":"0x00000000","v0":"0x001d27a8","v1":"0x00000000","s0":"0x00000000","s1":"0x003a1000","s2":"0x00000000","s3":"0x00000000","s4":"0x00000000","s5":"0x003a1000","s6":"0x00000000","s7":"0x00000000","t0":"0x002a31b8","t1":"0x00000064","t2":"0x00000000","t3":"0x00000000"},"derived":{"a1_region":"stack","a1_plus_30":"0x00000001","a1_plus_58":"0x00000000","caller_guess":"0x00129660"},"dumps":["dump_0001_bp1_a0.bin","dump_0001_bp1_a1.bin","dump_0001_bp1_sp.bin"],"notes":""}
```

JSONL e melhor que CSV para analise por IA porque preserva grupos de
registradores, campos derivados e lista de dumps.

## Indice de dumps

Arquivo:

```txt
.local/runtime-captures/2026-05-14-cloth-a1/memory-index.csv
```

Header:

```csv
hit_id,dump_file,base,size,expression,purpose
```

Exemplo:

```csv
1,dump_0001_bp1_a1.bin,0x01ffef00,0x120,a1-0x20,initializer argument structure
```

## Fluxo semi-automatizado

Usar este fluxo se o PCSX2 debugger nao exportar eventos por script.

1. Criar pasta da sessao em `.local/runtime-captures/`.
2. Abrir PCSX2 com debugger:

   ```sh
   flatpak run net.pcsx2.PCSX2 -debugger -- "/path/to/game.bin"
   ```

3. Carregar simbolos:

   ```txt
   .local/pcsx2-symbols.sym
   ```

4. Adicionar breakpoints:

   ```txt
   0x001d27a8
   0x00129660
   0x0013fb70
   0x0013fc44
   0x0013fcb8
   0x001d37c8
   ```

5. No primeiro hit de `0x001d27a8`, pausar e copiar registradores para
   `events.csv` ou `events.jsonl`.
6. Salvar dumps de memoria conforme o plano.
7. Resolver `ra` manualmente anotando:

   ```txt
   caller_guess = ra - 8
   ```

   Ajustar se o debugger mostrar delay slot/callsite com outro layout.

8. Criar breakpoint no caller identificado.
9. Continuar a execucao ate capturar o caller antes da chamada.
10. Encerrar depois de 1 a 3 hits bons. Mais hits so sao uteis se divergirem.

## Fluxo automatizavel

Usar este fluxo se houver forma de consultar registradores/memoria por console,
socket, script, hotkey macro ou export do debugger.

Para cada breakpoint hit:

1. Incrementar `hit_id`.
2. Ler registradores obrigatorios.
3. Classificar `a1_region`.
4. Ler words derivados:

   ```txt
   [a1+0x30]
   [a1+0x58]
   [a0+0x15c]
   [[a0+0x15c]+0x800]
   ```

5. Salvar dumps binarios.
6. Gravar linha em `events.jsonl`.
7. Se `pc == 0x001d27a8`, adicionar automaticamente breakpoint temporario em
   `ra-8`, se o debugger permitir.
8. Continuar execucao se o evento nao for conclusivo.

Pseudo-fluxo:

```txt
on_breakpoint(pc):
  regs = read_registers(required_regs)
  derived = derive_fields(regs)
  dumps = []

  if pc == 0x001d27a8:
    dumps += dump(a0-0x20, 0x220)
    dumps += dump(a1-0x20, 0x120)
    dumps += dump(sp, 0x100)
    if valid_ptr(read32(a0+0x15c)):
      dumps += dump(read32(a0+0x15c)-0x20, 0x920)
    set_temp_breakpoint(regs.ra - 8)

  if pc == 0x00129660:
    dumps += dump(s0-0x20, 0xa0)
    dumps += dump(sp, 0x100)

  write_jsonl(hit_id, pc, regs, derived, dumps)
```

## Classificacao de regioes

Usar classificacao conservadora. Os limites exatos podem ser refinados depois
com mapa de memoria do PCSX2.

| Regiao | Heuristica inicial | Interpretacao |
|---|---|---|
| `.text` | `0x00100000..0x0027ffff` aproximado | codigo; nao esperado para struct `a1` |
| `.data/.rodata` | perto de `0x002a0000..0x006fffff` | descriptor/static table possivel |
| descriptor table | perto de `0x002a31b8` | descriptor-label table |
| ROPE record-start | perto de `0x002a3934` | record-start convention |
| ROPE descriptor-label | perto de `0x002a3988` | descriptor-label convention |
| entry table | perto de `0x002a4c48` | entry table `0x4c` stride |
| stack | perto de `sp` | argumento construido no frame |
| heap/runtime | valido mas fora das regioes acima | estrutura dinamica |
| invalid | zero, baixo, unmapped, read fail | captura incompleta ou chamada inesperada |

## Criterios de parada

Parar a sessao quando um destes casos ocorrer:

1. `0x001d27a8` bateu e `ra` foi resolvido para um callsite.
2. `a1` foi classificado e `[a1+0x30]` foi capturado.
3. Houve dump valido de `a1`.
4. O caller foi capturado antes da chamada.

Nao precisa ficar coletando dezenas de hits se os tres primeiros sao iguais.
Mais hits so ajudam quando:

- `ra` muda;
- `a1_region` muda;
- `[a1+0x30]` muda;
- `callback_target` muda.

## Interpretacao rapida

| Resultado | Interpretacao inicial | Proxima acao |
|---|---|---|
| `ra == 0x00129668` | `0x00129660` provavelmente chamou o initializer | rastrear index/base do descriptor |
| `ra == 0x0013fc4c` ou perto | primeiro `+0x48` dispatcher | investigar origem incidental de `a1` |
| `ra == 0x0013fcc0` ou perto | segundo `+0x48` dispatcher | investigar origem incidental de `a1` |
| `ra == 0x0013fb78` ou perto | node dispatcher inesperado para initializer | verificar se `a1` e lixo/herdado |
| `ra` desconhecido | static scan perdeu caminho | mapear funcao do caller |
| `a1_region == static` | favorece descriptor/table | comparar com `0x002a31b8`, `0x002a3934`, `0x002a3988`, `0x002a4c48` |
| `a1_region == stack` | favorece caller que monta arg temporario | rastrear caller frame |
| `a1_region == heap` | favorece construcao dinamica | watchpoint em `a1+0x30` |
| `[a1+0x30] in 0..7` | compativel com Rev.041 variant table | comparar com `[payload+0x04]` |
| `[a1+0x58] == 0` | enfraquece hipotese `+0x58` | focar caller e `+0x30` |
| `[a1+0x58] != 0` | possivel descriptor-like | comparar valor com callbacks conhecidos |

## Como a IA deve usar os logs

Depois da sessao, entregar estes arquivos:

```txt
.local/runtime-captures/<session>/events.csv
.local/runtime-captures/<session>/events.jsonl
.local/runtime-captures/<session>/memory-index.csv
.local/runtime-captures/<session>/dump_*
```

A analise deve:

1. validar se os hits sao consistentes;
2. resolver `ra` para callsite;
3. classificar `a1`;
4. comparar `[a1+0x30]` com `[payload+0x04]`;
5. comparar `[a1+0x58]` com hipoteses descriptor-like;
6. gerar uma nova revisao `research/elf/ghidra-rev046-*` somente se houver
   evidencia runtime suficiente.

## Comando de partida

Comando pratico para abrir o PCSX2 debugger:

```sh
flatpak run net.pcsx2.PCSX2 -debugger -- "/home/peter/Imagens/Ico (USA)/Ico (USA).bin"
```

Se o caminho do jogo mudar, manter o resto do fluxo igual.

## Veredito

A automacao completa depende do que o debugger do PCSX2 expuser no ambiente
local. Mesmo sem API de scripting, este plano reduz o processo a uma captura
semi-automatizada com formato fixo.

O ponto de maior valor e capturar poucos eventos bons, nao muitos eventos
ruins. Um unico hit em `0x001d27a8` com `ra`, `a1`, `[a1+0x30]`, `[a1+0x58]` e
dump de memoria ja deve ser suficiente para orientar a Rev.046.

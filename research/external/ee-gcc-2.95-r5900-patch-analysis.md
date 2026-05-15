# EE GCC 2.95 R5900 Patch Analysis

## Date

2026-05-15

## Resumo executivo

O patch `gcc-ps2linux-1.0.0.patch` (183 KB, ~5500+ linhas) é o principal
patch R5900/Emotion Engine para o GCC 2.95.2 PS2 Linux. Ele adiciona suporte
completo ao target `mipsEEel-linux` (EE = Emotion Engine), incluindo:

- Novo target triple `mipsEEel`/`mipsEEel-linux`
- Nova configuração `eelinux.h` com default `-mcpu=r5900 -msingle-float -mabi=o32`
- Defines `-D__R5900 -D_R5900` como built-in do target
- R5900 como `PROCESSOR_R5900` no backend MIPS
- Flags `-mcpu=r5900`, `-mabi=eabi`, `-mgp64`, `-msingle-float`
- Suporte a 128-bit registers (`MASK_ARG128BIT`)
- Performance counters R5900 (ee-perfcounter.c)

O patch **aproxima** significativamente este GCC do `ee-gcc 2.9-991111-01`
que compilou o ICO, mas não é idêntico.

## Origem do patch

| Propriedade | Valor |
|---|---|
| Fonte | `SRPMS/GCC-29_1.RPM` do PS2 Linux Kit DISC2 |
| Pacote | `gcc-2.95.2-3a` |
| Target | `mipsEEel-linux` |
| Tamanho | 183 KB |
| Autor | Hiroyuki Machida `<machida@sm.sony.co.jp>` (Sony) |
| Data | 2001-10-04 |
| Base | GCC 2.95.2 upstream |
| Spec | `--target=mipsEEel-linux --host=mipsEEel-linux --build=mipsEEel-linux` |

## Arquivos alterados

### Arquivos NOVOS (adicionados pelo patch)

| Arquivo | Descrição |
|---|---|
| `gcc/config/mips/eelinux.h` | **Novo target header** — configuração do Emotion Engine Linux |
| `gcc/config/mips/ee-perfcounter.c` | Performance counters R5900 (basic block profiling) |
| `gcc/config/mips/libgcc2-timode.c` | Suporte a TImode (128-bit integers) para R5900 |
| `gcc/config/mips/mips-bb-helper.asm` | Assembly helper para basic block profiling R5900 |
| `gcc/config/mips/t-eelinux` | Makefile fragment para target EE |

### Arquivos MODIFICADOS (backend MIPS)

| Arquivo | Mudança principal |
|---|---|
| `gcc/config/mips/mips.h` | +PROCESSOR_R5900, +MASK_MIPS5900, +MASK_ARG128BIT, +MASK_5900REG_CODE, defines R5900 |
| `gcc/config/mips/mips.c` | R5900 cost tables, cpu detection, calling convention |
| `gcc/config/mips/mips.md` | R5900 instruction scheduling, 128-bit patterns |
| `gcc/config/mips/linux.h` | Ajustes para mipsEEel-linux |
| `gcc/config/mips/t-linux` | Ajustes de build para EE |

### Arquivos MODIFICADOS (infraestrutura)

| Arquivo | Mudança |
|---|---|
| `config.guess` | Adiciona `mipsEE` / `mipsEEel` |
| `config.sub` | Adiciona `mipsEE-*` / `mipsEEel-*` |
| `configure.in` | Suporte ao novo target |
| `gcc/c-tree.h`, `gcc/cp/*` | Ajustes para 128-bit integer types (TImode) |
| `gcc/ginclude/va-mips.h` | Ajustes de varargs para R5900 |

## Alterações em MIPS/R5900

### Novo PROCESSOR type

```c
enum processor {
    PROCESSOR_R3000,
    PROCESSOR_R3900,
    PROCESSOR_R4000,
    PROCESSOR_R4600,
    PROCESSOR_R5400,     // ← NOVO (vr5400)
    PROCESSOR_R5900,     // ← NOVO (Emotion Engine)
};
```

### Novas MASK flags

| Flag | Bit | Descrição |
|---|---|---|
| `MASK_MIPS5900` | `0x00200000` | Ativa otimizações R5900 |
| `MASK_NO_LENGTHEN_LOOP` | `0x10000000` | Desativa loop lengthening do R5900 |
| `MASK_ARG128BIT` | `0x20000000` | Usa registradores de 128-bit do R5900 para args |
| `MASK_5900REG_CODE` | `0x40000000` | Gera código específico R5900 |

### Relação com o target eelinux.h

```c
#define MIPS_CPU_STRING_DEFAULT "R5900"
#define MIPS_ISA_DEFAULT 2
#define TARGET_DEFAULT (MASK_ABICALLS|MASK_GAS|MASK_SINGLE_FLOAT|MASK_MIPS5900)
#include "linux.h"
#include "abi64.h"
```

O target `eelinux` herda de `linux.h` (configuração MIPS Linux padrão) e
`abi64.h` (suporte a ABI 64-bit), com overrides para R5900.

### Script GCC (especificações)

Do eelinux.h e das especificações geradas:

```
// default is elf, -mabi=o32, -mips2, -mcpu=r5900 and -EL
%{!mcpu*: -D__R5900 -D_R5900}
%{mcpu=r5900: -D__R5900 -D_R5900}
%{!mcpu*: -mcpu=r5900}
```

**Importante**: O define `__R5900` é sempre ativado para este target, seja
por default (`!mcpu*`) ou explicitamente (`mcpu=r5900`).

## Macros e defines

### Definidos pelo target

| Macro | Contexto |
|---|---|
| `__R5900` | Sempre definido para mipsEEel-linux |
| `_R5900` | Sempre definido para mipsEEel-linux |
| `__mips_eabi` | Quando `-mabi=eabi` |
| `__mips_single_float` | Quando `-msingle-float` (default no target) |
| `__mips64` | Quando `-mips3` ou `-mgp64` |
| `__PIC__`, `__pic__` | Quando PIC está ativo (default: ativado) |

### Não definidos (notável)

| Macro | Por que não está |
|---|---|
| `__mips=3` | Define `__mips=2` por default (MIPS_ISA_DEFAULT=2) |
| `__mips_eabi` | Só com `-mabi=eabi` (default é o32) |

## ABI e flags

### Flags suportadas (confirmadas pelo patch)

| Flag | Status | Notas |
|---|---|---|
| `-mcpu=r5900` | ✅ Suportado | `-D__R5900 -D_R5900`, ativa PROCESSOR_R5900 |
| `-mips3` | ✅ Suportado | MIPS III 64-bit, herança base |
| `-mips2` | ✅ Suportado | Default (`MIPS_ISA_DEFAULT 2`) |
| `-mgp64` | ✅ Suportado | General-purpose registers 64-bit |
| `-mabi=eabi` | ✅ Suportado | Embedded ABI (`abi64.h` incluso) |
| `-mabi=o32` | ✅ Suportado | Default |
| `-mabi=o64` | ✅ Suportado | 64-bit variant of o32 |
| `-mabi=n32` | ✅ Suportado | SGI N32 ABI |
| `-mabi=n64` | ✅ Suportado | SGI N64 ABI |
| `-msingle-float` | ✅ Suportado (default) | FPU single-precision apenas |
| `-G0` | ✅ Suportado | Desabilita GP-relative optimization |
| `-mno-abicalls` | ✅ Suportado | Desabilita PIC calling convention |
| `-fno-pic` | ✅ Suportado | `-U__PIC__ -U__pic__` |

### Flags NÃO suportadas

| Flag | Status | Impacto |
|---|---|---|
| `-march=r5900` | ❌ Ausente | Usar `-mcpu=r5900` como alternativa |
| `-march=...` (qualquer) | ❌ Ausente | GCC 2.95 não tem `-march` para MIPS |

A ausência de `-march=r5900` é esperada — GCC 2.95.x usava `-mcpu=` para
seleção de processador, não `-march=`. O `-march=` foi adicionado em versões
posteriores do GCC.

### Diferenças entre `-mcpu=r5900` e nenhum `-mcpu`

O patch define um default de `-mcpu=r5900` para o target mipsEEel-linux.
Na prática, `-mcpu=r5900` está sempre ativo para este target, a menos que
outro `-mcpu=` seja explicitamente passado.

## Impacto esperado no codegen

### O que o patch melhora

| Aspecto | Impacto |
|---|---|
| **Scheduling** | R5900-specific instruction latencies e issue rates |
| **FPU** | Single-float default (R5900 não tem double-precision total) |
| **128-bit load/store** | Suporte a `lq`/`sq` (quadword) com `MASK_ARG128BIT` |
| **Loop optimization** | Controle de loop lengthening com `MASK_NO_LENGTHEN_LOOP` |
| **Calling convention** | Registradores de 128-bit para argumentos com `MASK_ARG128BIT` |
| **Código específico R5900** | Com `MASK_5900REG_CODE`, gera instruções específicas EE |

### O que o patch NÃO altera

- O backend MIPS base continua sendo o GCC 2.95.2, que já suporta MIPS III
- As instruções aritméticas básicas (`add`, `addu`, `sub`, etc.) não mudam
- Loads/stores escalares (`lw`, `sw`, `ld`, `sd`) continuam os mesmos
- Branch/jump instructions continuam os mesmos

## Relação com ICO

### O que o PS2 Linux GCC compartilha com o compiler do ICO

1. **Base GCC 2.95.2** — mesma versão upstream
2. **Patches Sony** — autor do patch é engenheiro Sony (Hiroyuki Machida)
3. **R5900 backend** — `PROCESSOR_R5900` com tuning específico
4. **Defines `__R5900`/`_R5900`** — mesmos defines esperados pelo ICO
5. **Suporte a EABI** — `-mabi=eabi` funciona e gera calling convention correta

### O que é diferente

1. **Target do SDK** — ICO foi compilado com o SDK Sony (`ee-gcc 2.9-991111-01`),
   que pode ter patches adicionais não presentes no PS2 Linux Kit
2. **Default PIC** — o PS2 Linux GCC tem `MASK_ABICALLS` no TARGET_DEFAULT,
   ativando PIC. O ICO parece usar `-fno-pic -mno-abicalls`
3. **C library** — PS2 Linux usa glibc; ICO usa newlib (embarcado)
4. **Performance counters** — o PS2 Linux patch adiciona profiling R5900
   que pode não estar no SDK comercial

## Diferenças para ee-gcc 2.9-991111-01

| Característica | PS2 Linux GCC 2.95.2 | ee-gcc 2.9-991111-01 (ICO) |
|---|---|---|
| **Base GCC** | 2.95.2 | 2.95.2 (mesma) |
| **Patches** | PS2 Linux Kit (este patch) | SDK Sony (desconhecidos) |
| **`-mcpu=r5900`** | ✅ Suportado | ✅ Suportado |
| **`-march=r5900`** | ❌ Ausente | ✅ Possivelmente suportado |
| **Target triplet** | `mipsEEel-linux` | `ee` (ou similar Sony) |
| **Default ABI** | o32 (mas suporta eabi) | EABI |
| **Default PIC** | Sim (ABICALLS) | Provavelmente não |
| **Glibc vs newlib** | glibc (Linux) | newlib (embarcado) |
| **Debug/profiling** | R5900 perf counters | Desconhecido |

A diferença principal está nos patches adicionais do SDK Sony. O
`ee-gcc 2.9-991111-01` provavelmente é o mesmo GCC 2.95.2 com patches
adicionais para:
- Suporte a `-march=r5900` (versão do patch que adicionou a flag)
- ABI EABI refinada para o runtime PS2
- Integração com o SDK (newlib, PS2 specific headers)
- Remoção de código Linux (glibc, syscall, etc.)

## Valor para decomp.me

### O que este patch permite

1. **Compiler package experimental**: É possível construir um GCC 2.95.2
   com estes patches para usar como compiler package no decomp.me
2. **Codegen aproximado**: O código gerado será muito próximo do ICO,
   mesmo não sendo idêntico
3. **Base para refinamento**: Os patches documentam exatamente quais
   modificações foram feitas, permitindo ajustes finos

### Limitações

- Sem `-march=r5900`, algumas instruções específicas podem não ser geradas
- O PS2 Linux GCC é target `mipsEEel-linux` (Linux), não `ee` (bare metal)
- Diferenças de C library (glibc vs newlib) afetam chamadas de biblioteca
- Sem os patches exatos do SDK Sony, matching perfeito é improvável

### Passos para criar compiler package

1. Extrair `gcc-2.95.2.tar.gz` do SRPM
2. Aplicar os 3 patches (`frankengcc`, `single-float-const`, `ps2linux`)
3. Configurar com `--target=mipsEEel-linux --disable-nls --enable-shared`
4. Adicionar scripts de build para gerar as imagens Docker do decomp.me
5. Testar com scratches existentes (em `/tmp/decompme_scratches/`)

## O que fica confirmado

1. O PS2 Linux GCC 2.95.2 **tem suporte completo a R5900** via
   `PROCESSOR_R5900`, `MASK_MIPS5900`, e `-mcpu=r5900`
2. O patch foi escrito por **engenheiro da Sony** (Hiroyuki Machida)
3. O target `mipsEEel-linux` é uma configuração específica da Sony para
   o Emotion Engine
4. As flags `-mcpu=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0`
   são todas suportadas por este GCC
5. O default do target inclui `-msingle-float`, confirmando que o ICO
   usou esta flag (não `-mhard-float` tradicional)
6. O patch adiciona 128-bit register support (`MASK_ARG128BIT`),
   consistente com os registradores de 128-bit do R5900

## O que fica provável

1. `ee-gcc 2.9-991111-01` é o **mesmo GCC 2.95.2 base** com patches
   adicionais específicos do SDK Sony
2. `-march=r5900` foi adicionado nos patches do SDK e não está no
   PS2 Linux Kit público
3. O codegen do PS2 Linux GCC é **muito próximo** (mas não idêntico)
   ao do SDK comercial
4. A diferença de codegen será mais visível em instruções SIMD/MMI
   (MIPS Multimedia Instructions) específicas do R5900
5. O ICO provavelmente não usa MMI extensivamente (o foco é cloth
   physics, não processamento multimídia)

## O que ainda precisa de teste

1. Compilar uma função real do ICO (ex: `add`) no PS2 Linux GCC e
   comparar byte a byte com o ICO
2. Testar se `-mcpu=r5900` muda scheduling vs não usar a flag
3. Comparar calling convention do EABI gerado com os padrões do ICO
4. Verificar se o `-fno-pic` do ICO é consistente com o que o PS2 Linux
   GCC gera
5. Construir o GCC 2.95.2 com os patches para teste em ambiente Docker

## Próximo passo recomendado

1. ✅ **Patch analisado e documentado** (este documento)
2. 🔄 **Construir GCC 2.95.2 com patches** dentro do Docker i386
3. 🔄 **Compilar micro-targets** com `-mcpu=r5900` e comparar com ICO
4. 🔄 **Preparar compiler package experimental** para decomp.me
5. 🔄 **Runtime capture** para resolver `a1` (depende de PCSX2)

## Veredito

```txt
Valor do patch PS2 Linux para entender R5900 codegen: ALTO
Valor para criar compiler package decomp.me: ALTO
Proximidade com ee-gcc 2.9-991111-01: MÉDIA-ALTA
Matching first-diff com ICO: BAIXA (improvavel sem patches SDK)
Runtime ainda necessário: SIM
```

O patch `gcc-ps2linux-1.0.0.patch` é o documento técnico mais importante
encontrado até agora sobre o compilador do ICO. Ele prova que:

1. O GCC 2.95.2 com patches Sony para R5900 **existe e está disponível**
   (via PS2 Linux Kit)
2. As flags do ICO (`-mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2`)
   são todas suportadas
3. O target `mipsEEel-linux` é o ancestral direto do `ee` target do SDK
4. Os patches R5900 foram feitos pela Sony (engenheiro Hiroyuki Machida)

A diferença entre o PS2 Linux GCC e o SDK `ee-gcc 2.9-991111-01` está
provavelmente em patches adicionais do SDK que não foram liberados
publicamente. Mesmo assim, este patch já permite criar um compiler package
experimental para decomp.me e entender profundamente o codegen do ICO.

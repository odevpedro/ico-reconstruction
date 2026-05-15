# EE GCC 2.95 — Found and Validation Plan

## Date

2026-05-15

## Resumo executivo

Um cross-compiler PS2 Linux GCC 2.95.2 foi localizado no SourceForge
Kernelloader e extraído para validação. Ele é próximo do `ee-gcc
2.9-991111-01` usado pelo ICO, mas não é idêntico. Este documento registra
o achado e define um plano de validação controlado.

## O que foi encontrado

| Item | Valor |
|---|---|
| Fonte | SourceForge Kernelloader / Sony Linux Toolkit / Package Update Files / ps2stuff |
| Arquivo | `gcc-2.95.2-cross.tar.gz` (14 MB) |
| Path local | `/tmp/ps2-gcc-2.95/ps2/bin/ee-gcc` |
| Versão | GCC 2.95.2 19991024 (release) |
| Target triplet | `mipsEEel-linux` |
| Built-in defines | `-D__R5900 -D_R5900` (R5900 é target nativo) |
| State | Extraído, specs analisados, mas NÃO EXECUTÁVEL no glibc moderno |

Também disponível no mesmo repositório:
- `gcc-2.95-patched-cross.tar.gz`
- `gcc-2.95.2-cross-patched.tar.gz`
- `gcc-3.0.3-ps2linux-src.tgz` (source com patches)
- `binutils-2.9EE-cross.tar.gz`

## Por que isso importa

O compilador usado pelo ICO foi identificado como `ee-gcc 2.9-991111-01`
(Sony fork for R5900). O PS2 Linux GCC 2.95.2 é a versão pública mais
próxima conhecida. Embora não seja o mesmo build, ele permite:

1. Estudar os **patches R5900** aplicados ao GCC 2.95.2 base
2. **Comparar codegen** com o binário do ICO
3. **Criar um compiler package experimental** para decomp.me
4. **Documentar as diferenças** entre o GCC público PS2 Linux e o SDK Sony

## Diferença para ee-gcc 2.9-991111-01

| Característica | PS2 Linux GCC 2.95.2 | ee-gcc 2.9-991111-01 (ICO) |
|---|---|---|
| Base | GCC 2.95.2 oficial | GCC 2.95.2 com patches Sony |
| Target | `mipsEEel-linux` | `ee` (ou similar Sony) |
| `-march=r5900` | **NÃO suporta** | **Suporta** |
| C library | glibc (Linux) | newlib (embarcado/EABI) |
| ABI padrão | N64/o32 | EABI |
| Distribuição | PS2 Linux Kit (2001) | Sony SDK Comercial |

A ausência de `-march=r5900` é a diferença mais significativa. O backend
R5900 existe (os defines `__R5900`/`_R5900` estão lá), mas a interface de
linha de comando para selecioná-lo via `-march=` foi adicionada pela Sony
no SDK comercial.

## Flags suportadas

Das flags do ICO, estas são reconhecidas pelo PS2 Linux GCC 2.95.2:

| Flag | Status | Efeito |
|---|---|---|
| `-mips3` | ✅ | `-D__mips=3 -D__mips64`, ativa modo MIPS III 64-bit |
| `-mgp64` | ✅ | `-D__mips64`, força ponteiros 64-bit |
| `-mabi=eabi` | ✅ | `-D__mips_eabi`, Embedded ABI |
| `-msingle-float` | ✅ | `-D__mips_single_float`, FPU single-precision |
| `-G0` | ✅ | Desabilita GP-relative optimization |
| `-O2` | ✅ | Otimização nível 2 |

## Flags ausentes

| Flag | Status | Impacto |
|---|---|---|
| `-march=r5900` | ❌ | Sem tuning específico R5900; usa fallback MIPS III genérico |
| `-mno-gpopt` | N/A | Redundante com `-G0` |

A ausência de `-march=r5900` significa que o compilador não vai gerar
instruções específicas R5900 (como `mul` com accumulator).
O código gerado usará instruções MIPS III genéricas (como `mult`/`mfhi`
ou `dmult`/`mfhi`), que são funcionalmente equivalentes mas podem diferir
no padrão de registradores e tamanho.

## Defines built-in

Extraídos do specs:

```txt
-D__R5900 -D_R5900              (sempre definidos para mipsEEel-linux)
-D__mips=3 -D__mips64           (com -mips3)
-D__mips_eabi                   (com -mabi=eabi)
-D__mips_single_float           (com -msingle-float)
```

O define `__R5900` está sempre ativo para o target PS2 Linux, indicando
que o backend R5900 foi compilado no GCC mesmo sem a flag `-march=r5900`.

## Problema de compatibilidade glibc/i386

O binário `ee-gcc` é ELF 32-bit i386, linked contra glibc 2.2.5:

```txt
ELF 32-bit LSB executable, Intel i386, version 1 (SYSV),
dynamically linked, interpreter /lib/ld-linux.so.2,
for GNU/Linux 2.2.5, not stripped
```

No sistema atual (glibc 2.42), ldd resolve as bibliotecas mas o binário
falha com:

```
cpp: installation problem, cannot exec `cpp': Argument list too long
```

A causa provável é incompatibilidade entre a glibc 2.2.x esperada e a
glibc 2.42 real — funções internas do loader/tls ou comportamento do
alocador de argumentos mudou. O binário precisa de um ambiente i386
glibc 2.2.x para funcionar.

## Plano de container i386

Para executar o GCC 2.95.2 PS2 Linux, usar Docker/Podman com imagem
i386/glibc antiga:

```dockerfile
FROM i386/debian:squeeze
# ou i386/ubuntu:10.04
RUN apt-get update && apt-get install -y rpm2cpio cpio
COPY gcc-2.95.2-cross.tar.gz /tmp/
RUN cd /tmp && tar -xzf gcc-2.95.2-cross.tar.gz
ENV PATH="/tmp/ps2/bin:$PATH"
```

Teste mínimo dentro do container:

```bash
ee-gcc --version
ee-gcc -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2 \
  -S -o /tmp/test.s -xc - <<< 'int f(int x){return x+1;}'
cat /tmp/test.s
```

## Plano de micro-target codegen

Após conseguir rodar o GCC 2.95.2, compilar funções mínimas e comparar
com o assembly do ICO:

### Código de teste

```c
// test1: aritmética simples
int add(int a, int b) { return a + b; }

// test2: acesso a struct via ponteiro
int test(int *p, int x) {
    p[1] = x;
    return p[0] + x;
}

// test3: float
float fadd(float a, float b) { return a + b; }

// test4: loop
int sum(int *arr, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) s += arr[i];
    return s;
}
```

### Compilação

```bash
ee-gcc -S -O2 -G0 -mips3 -mgp64 -mabi=eabi -msingle-float test.c
```

### Comparação

Para cada função, comparar:
- Prólogo (stack frame, saved registers)
- Acesso a argumentos (a0, a1, etc.)
- Instruções de load/store
- Epílogo (return sequence)
- Uso de registradores temporários ($t vs $v)

Comparar com funções equivalentes no ICO (ex: `add` no ICO em 0x00xxxx).

## Plano de extração do DISC2.iso/SRPM

O Archive.org tem o PS2 Linux PAL Installation Discs com DISC2.iso (1.3 GB)
que deve conter os SRPMs originais.

### Download

```bash
curl -L -o /tmp/DISC2.iso \
  "https://archive.org/download/disc-1_20220424/DISC2.iso"
```

### Extração

```bash
mkdir -p /tmp/ps2linux-srpms
7z x /tmp/DISC2.iso -o/tmp/ps2linux_disc2
find /tmp/ps2linux_disc2 -name "*.src.rpm" | sort
```

### Alvos prioritários

```txt
gcc-2.95.2-*.src.rpm      # GCC fonte com patches Sony
binutils-*.src.rpm         # Binutils fonte
glibc-2.2.2-*.src.rpm     # glibc fonte (referência)
```

### Extração de SRPM

```bash
rpm2cpio gcc-2.95.2-*.src.rpm | cpio -idmv
find . -name "*.patch" -o -name "*.diff" -o -name "*.tar.gz"
```

## O que não fazer

- Não usar SDK oficial Sony vazado (legalmente arriscado)
- Não commitar binários do compilador no repositório
- Não tratar PS2 Linux GCC como equivalente ao ee-gcc 2.9-991111-01
- Não assumir matching sem first-diff
- Não gastar tempo excessivo tentando fazer o GCC 2.95.2 rodar no glibc
  moderno — use container
- Não perseguir `-march=r5900` no GCC 2.95.2 — ele não tem suporte e
  adicionar seria um fork do GCC

## Progresso

### SRPM do PS2 Linux Kit — ✅ ENCONTRADO E EXTRAÍDO

O arquivo `GCC-29_1.RPM` no diretório `SRPMS/` do DISC2.iso é o **Source RPM**
do **GCC 2.95.2-3a** para PS2 Linux (target `mipsEEel-linux`).

Conteúdo:

| Arquivo | Tamanho | Descrição |
|---|---|---|
| `gcc-2.95.2.tar.gz` | 49 KB | Upstream GCC 2.95.2 source (comprimido) |
| `gcc-2.95.2-frankengcc-patches.patch` | 94 linhas | C++ comdat linkage fix (Netscape) |
| `gcc-2.95.2-single-float-const.patch` | 79 linhas | `-fsingle-precision-constant` flag |
| `gcc-ps2linux-1.0.0.patch` | **183 KB** | **Patch principal PS2 Linux** |
| `gcc-ps2linux-1.0.0.spec` | — | RPM spec (build/install) |

O spec confirma:

- **Target**: `mipsEEel-linux`
- **Config**: `--target=mipsEEel-linux --host=mipsEEel-linux --build=mipsEEel-linux`
- **Depende de**: `binutils >= 2.9EE`
- **Build date**: 2001-10-04 (sexta-feira)
- **Changelog**: `Thu Mar 29 2001 Hiroyuki Machida <machida@sm.sony.co.jp>`
  — engenheiro da Sony adicionando suporte a PS2 Linux

O patch principal (`gcc-ps2linux-1.0.0.patch`) com 183 KB modifica dezenas de
arquivos do GCC para adicionar:
- R5900 performance counters (`sys_r5900.h`, `sysmips`)
- Target triple `mipsEEel`/`mipsEEel-linux` em `config.guess` e `config.sub`
- Configuração completa do backend MIPS para o target EE
- Suporte a `-D__R5900` e `-D_R5900` como built-in

### Container i386 — ✅ FUNCIONAL

Docker i386 (Debian Bookworm) → GCC 2.95.2 PS2 Linux executa sem erros.

Comando usado:
```bash
docker run --rm ps2-gcc-295 ee-gcc \
  -B/tmp/ps2/bin/ -B/tmp/ps2/lib/gcc-lib/mipsEEel-linux/2.95.2/ \
  -S -G0 -mips3 -mgp64 -mabi=eabi -msingle-float -O2 -fno-pic -mno-abicalls \
  test.c
```

### Codegen observado

| Função | Assembly gerado | Notas |
|---|---|---|
| `add` | `j $31; addu $2,$4,$5` | Tail call, 0 stack frame |
| `mul` | `j $31; mult $2,$4,$5` | 3-operand MIPS32 (não usa hi/lo) |
| `test` | `lw $2,0($4); sw $5,4($4); addu` | Load/store padrão |
| `ladd` | `j $31; daddu $2,$4,$5` | 64-bit add com `-mgp64` |
| `cond` | `movn $2,$4,$3` | Conditional move MIPS IV |
| `sum` | `dsll`/`daddu`/`lw`/`bne` loop | Loop com array |

### Comparação com ICO

**Similaridades**:
- Uso de `.ent`/`.end`/`.frame`/`.mask`/`.fmask` (mesmo estilo do ICO)
- `j $31` equivalente a `jr $ra` (mesmo encoding)
- `addu`/`daddu` para aritmética
- EABI calling convention (argumentos em a0-a3, retorno em v0)
- `-fno-pic -mno-abicalls` remove código de GP-relative (mais próximo do ICO)

**Diferenças esperadas**:
- ICO usa `-march=r5900` (não disponível aqui) → seleção diferente de
  instruções (ex: `mul` vs `mult`/`mflo`)
- ICO foi compilado com patches Sony adicionais
- ICO usa newlib (embarcado), não glibc (Linux)

### DISC2.iso

Download em andamento via Archive.org (~175MB de 1359MB, 13%).

## Próximo passo recomendado

1. ✅ Container i386 criado e funcional
2. ✅ GCC 2.95.2 executando dentro do container
3. ✅ Micro-targets compilados e comparados (6 near-matches)
4. ✅ Divergências de codegen documentadas (ld/lw, JT 4B/8B)
5. ✅ Compilador disponível em `/tmp/ps2-gcc-2.95/` (prebuilt)
6. ❌ Build do GCC 2.95 do source falhou (autoconf + newlib)
   → Usar o prebuilt como base para compiler package decomp.me é viável
7. 🔄 **Retomar runtime capture** para resolver origem do `a1` em
   `0x001d27a8` (gargalo real da análise)

## Veredito

```txt
Valor do PS2 Linux GCC 2.95.2 para codegen: MÉDIO-ALTO
Valor para matching first-diff com ICO: BAIXO
Valor para compiler package decomp.me: MÉDIO
Runtime (a1 source) continua necessário: SIM
DISC2.iso download prioritário: NÃO (fazer depois do container test)
```

O PS2 Linux GCC 2.95.2 é o compilador público mais próximo do ee-gcc
que compilou o ICO, mas não é idêntico. A diferença principal
(`-march=r5900`) sugere que o código gerado será similar mas não
exatamente igual. O maior valor está em entender os patches R5900 e ter
uma base para compiler package experimental no decomp.me.

A prioridade real do projeto continua sendo runtime capture para
`0x001d27a8(a0, a1)`.

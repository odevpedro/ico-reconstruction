# EE GCC 2.95 Archive Search Plan

## Date

2026-05-15

## Resumo executivo

Plano de busca e validação para localizar fontes legais/históricas do compilador
EE GCC usado no desenvolvimento de ICO (provisoriamente identificado como
`ee-gcc 2.9-991111-01`, baseado em GCC 2.95.2 com patches Sony R5900).

## Por que isso importa para ICO

O compilador que produziu o binário do ICO foi identificado nas revisões
Rev.038-044 como:

- **Compilador**: EE GCC 2.9-991111-01 (Sony fork for R5900)
- **Flags**: `-march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2`

Este compilador era parte do **SDK de desenvolvimento PlayStation 2 da Sony**,
não do PS2 Linux Kit público. No entanto, o PS2 Linux Kit (lançado pela Sony
em 2001) incluía um GCC 2.95.2 com patches para o Emotion Engine (R5900) que
pode ser o mesmo compiler base ou próximo o suficiente para:

1. Criar um **compiler package experimental para decomp.me**
2. **Comparar codegen** com o binário do ICO
3. **Documentar os patches Sony R5900** aplicados ao GCC 2.95.2

## Fontes-alvo

### 1. SourceForge Kernelloader — Sony Linux Toolkit (ALTA PRIORIDADE)

**URL**: https://sourceforge.net/projects/kernelloader/files/Sony%20Linux%20Toolkit/

Arquivos encontrados no diretório `Package Update Files/ps2stuff/`:

| Arquivo | Tamanho | Descrição |
|---|---|---|
| `gcc-2.95.2-cross.tar.gz` | ~? | GCC 2.95.2 cross-compiler pré-compilado |
| `gcc-2.95-patched-cross.tar.gz` | ~? | GCC 2.95.x patched cross-compiler |
| `gcc-2.95.2-cross-patched.tar.gz` | ~? | GCC 2.95.2 cross patched |
| `binutils-2.9EE-cross.tar.gz` | ~? | EE binutils (assembler/linker) |
| `gcc-3.0.3-cross-patched.tar.gz` | ~? | GCC 3.0.3 patched cross |
| `gcc-2.95.2-cross.tar.gz.md5sum` | ~? | Checksum |

Também em `Package Update Files/gcc/`:

| Arquivo | Descrição |
|---|---|
| `gcc-3.0.3-ps2linux-src.tgz` | GCC 3.0.3 source com patches PS2 Linux |

Também em `Package Update Files/cfyc/`:

| Arquivo | Descrição |
|---|---|
| `gcc-2.95.2-ps2linux-win32.zip` | Versão Windows do GCC PS2 Linux |

O arquivo maior `sony_ps2_linux_toolkit.7z` (1.1 GB) pode conter o kit completo.

### 2. Archive.org — PS2 Linux PAL Installation Discs (ALTA PRIORIDADE)

**URL**: https://archive.org/details/disc-1_20220424

Duas ISOs disponíveis:
- `DISC1.iso` (1.0 GB) — Sistema Linux, ferramentas de usuário
- `DISC2.iso` (1.3 GB) — **SRPMS, fontes, toolchain** ← Prioridade máxima

O Disco 2 deve conter:
- `gcc-2.95.2-*.src.rpm` — Source RPM do GCC com patches Sony
- `binutils-*.src.rpm` — Source RPM do binutils
- `glibc-2.2.2-*.src.rpm` — Source RPM da glibc

### 3. Archive.org — PlayStation2-Linux.com Repository

**URL**: https://archive.org/details/PlayStation2-Linux.com_Repository

Repositório completo do site playstation2-linux.com (1.1 GB).

### 4. Web Archive — PS2 Linux community sites

- http://ps2-stuff.tensioncore.com/gcc_build.html (arquivado)
- http://playstation2-linux.com (arquivado via web.archive.org)

### 5. GNU GCC 2.95.2 upstream (baseline)

**URL**: https://ftp.gnu.org/gnu/gcc/gcc-2.95.2/

- `gcc-core-2.95.2.tar.gz` — Core compiler source
- `gcc-g++-2.95.2.tar.gz` — G++ source
- `gcc-2.95.1-2.95.2.diff.gz` — Diff from 2.95.1

Útil como baseline para comparar patches.

### 6. rickgaiser/gcc-ps2 (GitHub, experimental)

**URL**: https://github.com/rickgaiser/gcc-ps2

Repositório público com GCC patcheado para PS2. Experimental — não confirmado
como matching do ICO, mas útil para entender patches R5900.

## Termos de busca

```txt
gcc-2.95.2*.src.rpm
gcc*.src.rpm ps2linux
binutils*.src.rpm ps2linux
ee-gcc* ps2
mips*-gcc* r5900
ps2linux-toolchain*
BlackRhino SRPMS gcc
ps2stuff gcc cross
```

## Arquivos esperados

### Do PS2 Linux Kit Disc 2 (SRPMS)

- `gcc-2.95.2-*.src.rpm`
- `binutils-*.src.rpm`  
- `glibc-2.2.2-*.src.rpm`
- `kernel-2.2.1-*.src.rpm`
- `XFree86-*.src.rpm`

### Do SourceForge Kernelloader

- Prebuilt cross-compiler tarballs (já encontrados)

## Procedimento de extração de SRPM

```bash
# Se SRPM encontrado
mkdir -p /tmp/ps2linux-gcc
cd /tmp/ps2linux-gcc
rpm2cpio gcc-*.src.rpm | cpio -idmv
find . -maxdepth 2 -type f | grep -Ei 'patch|diff|gcc|mips|r5900|ee|emotion'
```

Para extrair ISO do Archive.org:
```bash
# Baixar ISO
curl -L -o /tmp/DISC2.iso "https://archive.org/download/disc-1_20220424/DISC2.iso"
# Montar (precisa de loop device) ou extrair com 7z
7z x /tmp/DISC2.iso -o/tmp/ps2linux_disc2
find /tmp/ps2linux_disc2 -name "*.src.rpm" -o -name "*gcc*" -o -name "*patch*"
```

## Critérios de validação

Para classificar um compilador candidato, usar esta ordem:

1. **Identificar versão upstream**: qual GCC base (ex: 2.95.2)
2. **Listar patches**: quantos, nomes, o que modificam
3. **Verificar target triplet**: esperado `mipsEEel-linux` ou similar
4. **Verificar suporte a R5900**: `-march=r5900` deve ser aceito
5. **Testar flags do ICO**:
```bash
echo 'int f(int x){return x+1;}' | cc -march=r5900 -mips3 -mgp64 -mabi=eabi \
  -msingle-float -G0 -O2 -c -x c - -o /tmp/test.o 2>&1
```
6. **Comparar assembly com ICO**: compilar função pequena e comparar
   código gerado com padrões do ICO

## Teste mínimo de compilação

```bash
cat > /tmp/test_ico.c << 'EOF'
int add(int a, int b) { return a + b; }
int mul(int a, int b) { return a * b; }
EOF

# Com o compilador candidato:
cc -march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2 \
  -c /tmp/test_ico.c -o /tmp/test_ico.o

# Desassemblar e comparar:
objdump -d /tmp/test_ico.o
```

## Classificação dos achados

| Categoria | Critério |
|---|---|
| `confirmed_ps2linux_gcc_2.95.2_source` | SRPM verificado, patches identificados, compila para R5900 |
| `confirmed_ps2linux_prebuilt_cross` | Binário pré-compilado funcional, aceita flags do ICO |
| `generic_gcc_2.95_upstream` | GCC 2.95.2 sem patches PS2 — baseline para diff |
| `modern_ps2dev_toolchain` | GCC moderno (10-15.x) — útil para tooling, não para matching |
| `needs_signature_test` | Encontrado mas não testado com flags do ICO |
| `unusable_or_legal_risk` | SDK proprietário de origem duvidosa — não usar |

## Legais e técnicos

### OK para usar
- PS2 Linux Kit (distribuição oficial Sony, disponível no Archive.org)
- SourceForge Kernelloader (código aberto, comunidade)
- GNU GCC upstream (GPL)
- ps2dev toolchain (BSD/GPL open source)

### NÃO usar
- Official PS2 SDK vazado (ProDG, SN Systems, etc.) — legalmente arriscado
- Binários de toolchain comercial de origem não verificada
- Qualquer coisa marcada como "NDA only" ou "confidential"

## Progresso

1. ✅ SourceForge explorado — gcc-2.95.2-cross identificado
2. ✅ `gcc-2.95.2-cross.tar.gz` baixado (14 MB)
3. ✅ Extraído para /tmp/ps2-gcc-2.95/
4. ✅ Compilador identificado: GCC 2.95.2 19991024, target mipsEEel-linux
5. ✅ Specs file analisado — suporta `-mips3`, `-mgp64`, `-mabi=eabi`, `-msingle-float`, `-G0`
6. ❌ Não suporta `-march=r5900` (recurso adicionado pelo SDK Sony)
7. ❌ Binário 32-bit incompatível com glibc moderna 2.42 (falha ao executar)
8. ⏳ Baixar `DISC2.iso` do Archive.org (1.3 GB) para SRPMS

### Resultados da análise do specs

O arquivo specs do GCC 2.95.2 PS2 Linux confirma suporte a:

| Flag | Suportado | Notas |
|---|---|---|
| `-mips3` | ✅ | Define `-D__mips=3 -D__mips64` |
| `-mgp64` | ✅ | Define `-D__mips64` |
| `-mabi=eabi` | ✅ | Define `-D__mips_eabi` |
| `-msingle-float` | ✅ | Define `-D__mips_single_float` |
| `-G0` | ✅ | Passado para assembler |
| `-march=r5900` | ❌ | Não reconhecido — específico do SDK Sony |
| `-mfp64` | ✅ | Usado com mips3 para FPU 64-bit |

O compilador já define `-D__R5900 -D_R5900` como built-in do target, indicando
que o backend R5900 faz parte do GCC 2.95.2 PS2 Linux, mas a interface
`-march=` para selecioná-lo foi adicionada posteriormente no SDK Sony.

## Próximo passo recomendado

1. ✅ SourceForje explorado — gcc-2.95.2-cross baixado e analisado
2. ⏳ Baixar `DISC2.iso` do Archive.org (1.3 GB) para obter SRPMS originais
3. ⏳ Extrair SRPMS do ISO e obter patches fonte
4. ⏳ Tentar executar GCC 2.95.2 via container Docker (i386/glibc 2.2)
5. ⏳ Comparar codegen com ICO

## Veredito

```txt
Chance de achar gcc-2.95.2 PS2 Linux SRPM no DISC2.iso: ALTA
Chance de achar exatamente ee-gcc 2.9-991111-01 do build comercial: BAIXA
Chance de usar PS2 Linux GCC para entender codegen: MÉDIA (compatibilidade)
Chance de usar PS2 Linux GCC para compiler package decomp.me: MÉDIA
Chance de matching perfeito first-diff com ICO: BAIXA
```

O valor real não está em achar o compilador mágico, mas em:
1. Obter os patches R5900 para entender o codegen
2. Criar um compiler package experimental para decomp.me
3. Documentar as diferenças entre o PS2 Linux GCC e o SDK comercial

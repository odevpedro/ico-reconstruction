# EE GCC 2.95 Local Compiler Package Plan

## Date

2026-05-15

## Resumo executivo

Plano para empacotar o GCC 2.95.2 PS2 Linux prebuilt como um compiler
package local utilizável por scripts e, futuramente, adaptável para o
decomp.me. O foco é uso prático agora, não submissão à plataforma.

## Pré-requisitos existentes

| Componente | Localização |
|---|---|
| GCC 2.95.2 prebuilt | `/tmp/ps2-gcc-2.95/ps2/` |
| ee-gcc binary | `/tmp/ps2-gcc-2.95/ps2/bin/ee-gcc` |
| Specs file | `/tmp/ps2-gcc-2.95/ps2/lib/gcc-lib/mipsEEel-linux/2.95.2/specs` |
| Docker i386 image | `ps2-gcc-295` (container funcional) |
| SRPM patches | `/tmp/ps2linux-gcc-src/` |
| MIPS cross toolchain | `~/ps2dev/` (ps2dev moderno para assembling/linking) |

## Uso local

### Via Docker (recomendado)

```bash
# Compilar um arquivo .c com flags ICP
docker run --rm -i ps2-gcc-295 bash -c '
    cat > /tmp/test.c
    B="-B/tmp/ps2/bin/ -B/tmp/ps2/lib/gcc-lib/mipsEEel-linux/2.95.2/"
    F="-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls"
    /tmp/ps2/bin/ee-gcc $B -S $F /tmp/test.c -o /tmp/test.s
    cat /tmp/test.s
' < input.c > output.s
```

### Via script wrapper

```bash
#!/bin/bash
# ee-ico-compile.sh — compila C para MIPS R5900 (estilo ICO)
IMAGE="ps2-gcc-295"
B="-B/tmp/ps2/bin/ -B/tmp/ps2/lib/gcc-lib/mipsEEel-linux/2.95.2/"
F="-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls"
docker run --rm -i "$IMAGE" /tmp/ps2/bin/ee-gcc $B $F "$@"
```

Uso:
```bash
./ee-ico-compile.sh -c file.c -o file.o
./ee-ico-compile.sh -S file.c -o file.s
```

## Flags padrão

```
ICP = -O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls
```

### Variações úteis

| Finalidade | Flags |
|---|---|
| Padrão ICO | ICP (acima) |
| Com EABI | ICP + `-mabi=eabi` |
| Com PIC | ICP sem `-fno-pic -mno-abicalls` |
| Sem gp64 | ICP sem `-mgp64` |
| Com debug | ICP + `-g` |

## Dependências do container

O container `ps2-gcc-295` é baseado em `i386/debian:bookworm` com:
- `libc6` (i386) — para executar o binário 32-bit
- `curl ca-certificates` — apenas para build

O GCC prebuilt é ELF 32-bit i386 linked contra glibc 2.2.5.
O container i386 Debian Bookworm fornece glibc 2.36 compatível.

## Limitações conhecidas

1. **Sem `-march=r5900`**: usar `-mcpu=r5900` como substituto
2. **Target Linux**: compilador espera glibc, não newlib (embarcado)
3. **32-bit binário**: precisa de i386 userspace ou Docker multiarch
4. **PIC default**: `MASK_ABICALLS` no TARGET_DEFAULT — usar `-fno-pic -mno-abicalls`
5. **Jump tables**: sempre `.dword` (8B), ICO usa `.word` (4B)
6. **Pointer loads**: `ld` (64B) vs ICO `lw` (32B)

## Pipeline completo (splat + compile)

```bash
# 1. Split
splat split splat/SCUS_971.13.cloth-full.yaml

# 2. Compilar C mínimo
docker run --rm -i ps2-gcc-295 ... -S cloth_get_variant.c -o cloth_get_variant.s

# 3. Comparar com ICO (do splat output)
diff -u <(grep -A6 "glabel func_001D3D70" /tmp/ico-cloth-full/asm/cloth_sub_001d3d70.s) \
        cloth_get_variant.s
```

## Adaptação para decomp.me

O decomp.me usa imagens Docker como compiler packages. Para adaptar:

1. **Criar Dockerfile** que copia o prebuilt para a imagem:
```dockerfile
FROM i386/debian:bookworm
COPY ps2/ /usr/local/ps2/
ENV PATH="/usr/local/ps2/bin:$PATH"
```

2. **Publicar** no ghcr.io (requer conta GitHub + permissão no repo
   `decompme/compilers`)

3. **Registrar** no arquivo `values.yaml` do repo `decompme/compilers`
   com um ID único (ex: `ee-gcc-2.95-ps2linux-r5900`)

4. **Testar** com scratches existentes em `/tmp/decompme_scratches/`

**Nota**: Este passo requer aprovação dos mantenedores do decomp.me.
Não tentar sem antes testar localmente e documentar o compiler behavior.

## Checklist de uso local

- [ ] Docker instalado
- [ ] Imagem `ps2-gcc-295` construída
- [ ] Script wrapper `ee-ico-compile.sh` criado
- [ ] Testado com `echo 'int f(int x){return x+1;}' | ./ee-ico-compile.sh -S -xc - -o /tmp/test.s`
- [ ] Output comparado com padrões ICO
- [ ] Divergências documentadas (ld/lw, JT)

## Próximo passo

1. Criar script wrapper no repositório (em `splat/` ou `tools/`)
2. Documentar no `splat/Makefile` como alvo `make scratch`
3. Usar para os próximos scratches (func_001D4358, func_001D3BF0)
4. Não submeter ao decomp.me até ter mais resultados de scratches

# rev.038 — External Integration: decomp.me Scratches

## Data

2026-05-13

## Objetivo

Executar a integração externa iniciada em Rev.038:

1. Executar CCC (Chaos Compiler Collection) no ELF para detectar símbolos de debug
2. Gerar scratches decomp.me para as funções mapeadas

## Escopo

Incluído:
- execução do CCC `stdump identify` no ELF
- extração de bytes e disassembly via Capstone para 6 funções
- geração de pacotes .s + .hex + .meta.json + README
- refinamento das flags de compilador

Excluído:
- cross-reference com ICO-decomp (ver `research/ico-decomp-cross-reference-2026-05-14.md`)
- submissão manual ao decomp.me
- runtime validation
- análise de DATA.DF

## Fontes usadas

| Fonte | Uso |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | bytes e instruções |
| `github.com/chaoticgd/ccc` v2.1 | stdump para debug symbols |
| Capstone 5.0.7 | disassembly MIPS64 R5900 |

## Resultado 1: CCC — Nenhum símbolo de debug

```txt
$ stdump identify SCUS_971.13.elf
SCUS_971.13.elf: none
```

Confirmação definitiva: ELF completamente stripped. Sem .mdebug, sem .symtab, sem .dynsm, sem SNDLL.

Nota sobre a ferramenta: CCC foi baixado como binário pré-compilado (v2.1 linux-glibc2.35), extraído e executado sem necessidade de build.

## Resultado 2: Scratches decomp.me gerados

6 pacotes em `/tmp/decompme_scratches/`:

| Função | Endereço USA | Bytes | Stack | Instruções |
|--------|-------------|-------|-------|------------|
| state_dispatcher | 0x001d37c8 | 616 | 0x60 | 154 |
| rope_entity_callback | 0x001d3a30 | 248 | 0x50 | 62 |
| callback_registration | 0x0013f7a8 | 40 | 0x10 | 10 |
| sister_callback_reg | 0x0013f778 | 32 | 0x10 | 8 |
| node_callback_storage | 0x0013f3f0 | 528 | 0x90 | 132 |
| object_init_reg_path | 0x001b76f8 | 992 | 0x130 | 248 |

Cada pacote contém:
- `.s` — assembly listing completo (Capstone MIPS64 LE)
- `.bin.hex` — raw bytes em hex string
- `.meta.json` — metadados (endereço, stack, calls, data refs, compilador)

### Flags do compilador

Flags refinadas via pesquisa cruzada com outros projetos PS2 (Klonoa 2, PaRappa 2, Sly Cooper):

```
-march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2
```

Compiler: ee-gcc 2.9-991111-01 (Sony fork)

Correções em relação às flags inferidas anteriormente:
- Adicionado `-march=r5900` (mais preciso que só `-mips3`)
- Adicionado `-mabi=eabi` (ESSENCIAL — define calling convention)
- Adicionado `-msingle-float` (R5900 FPU é single-precision apenas)
- `-mno-gpopt` é redundante com `-G0`

## Resultado 3: Disassembly via Capstone

Todas as 6 funções foram disassembladas com 100% de cobertura (nenhuma instrução MMI/VU0 não decodificada no range selecionado).

Destaques do disassembly:

### state_dispatcher (0x1d37c8)
```asm
001d37c8: addiu  $sp, $sp, -0x60
001d37cc: sd     $s2, 0x30($sp)
001d37d0: sd     $s1, 0x20($sp)
001d37d4: move   $s2, $a0          ; s2 = context
001d37d8: sd     $ra, 0x50($sp)
001d37dc: sd     $s3, 0x40($sp)
001d37e0: sd     $s0, 0x10($sp)
001d37e4: lw     $v0, 0x15c($s2)   ; v0 = [context + 0x15c] (entity)
001d37e8: lw     $s3, 0x800($v0)   ; s3 = [entity + 0x800] (state_block)
001d37ec: addiu  $s1, $s3, 0x40    ; s1 = state_block + 0x40
001d37f0: lw     $v1, 8($s1)       ; v1 = [state_block + 0x48] (state_id)
001d37f4: sltiu  $v0, $v1, 5       ; bounds check
001d37f8: beqz   $v0, 0x1d3818     ; if invalid, goto state_0 fallback
001d37fc: lui    $v0, 0x62
001d3800: sll    $v1, $v1, 2
001d3804: addiu  $v0, $v0, -0x7050 ; v0 = 0x00618fb0 (jump table)
001d3808: addu   $v1, $v1, $v0
001d380c: lw     $a0, ($v1)        ; load jump target
001d3810: jr     $a0               ; dispatch
001d3814: nop
```

### callback_registration (0x13f7a8)
```asm
0013f7a8: move   $v0, $a3          ; preserve original a3
0013f7ac: addiu  $sp, $sp, -0x10   ; prologue (16 bytes)
0013f7b0: andi   $a3, $a2, 0xff    ; mask a2 to byte
0013f7b4: move   $t1, $t0          ; preserve caller t0
0013f7b8: move   $a2, $a1          ; a2 = callback pointer
0013f7bc: sd     $ra, ($sp)        ; save return address
0013f7c0: move   $t0, $v0          ; t0 = original a3
0013f7c4: jal    0x13f3f0          ; delegate to storage
0013f7c8: move   $a1, $a0          ; a1 = original a0 (handle)
0013f7cc: ld     $ra, ($sp)        ; restore ra
```

## O que é confirmado

1. ELF não tem símbolos de debug (CCC confirmou).
2. 6 scratches decomp.me gerados com dados verificados por byte.
3. Compilador flags refinadas: -march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2.

## O que é provável

1. Os scratches podem atrair contribuições de matching se submetidos ao decomp.me.
2. As flags de compilador estão corretas para o ee-gcc 2.9-991111-01.

## Próximo teste mínimo

1. Submeter scratches ao decomp.me manualmente.
2. Ver cross-reference com ICO-decomp em `research/ico-decomp-cross-reference-2026-05-14.md`.

## Veredito conservador

CCC confirmou ausência de debug symbols. Scratches decomp.me estão prontos para submissão com flags de compilador refinadas.

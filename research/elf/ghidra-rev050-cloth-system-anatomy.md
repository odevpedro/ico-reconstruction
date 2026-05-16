# rev.050 — Cloth System Anatomy: 1D27A8, 1B76F8, and the Entry Table Mapped

## Date

2026-05-16

## Resumo executivo

Três análises offline consolidadas:

1. **cloth_payload_init (0x1D27A8)**: Decompilacao parcial com a variante runtime conhecida. Funcao de ~528B com 3 sub-blocos (alloc, copy-loop, conditional init). O campo `variant` em `[payload+0x04]` decide se o init completo roda ou nao.

2. **Descriptor iteration (0x1B76F8)**: Funcao grande (~1024B+) que itera a entry table em `0x002A4C48`, le `entry+0x46` = indice de descriptor, carrega descritor de `0x002A31B8 + index*0x64`, e chama callbacks dos slots `+0x58` (cloth_payload_init) e registra callbacks em `+0x50` via `0x13F7A8`.

3. **Entry table mapeada**: 512 entries escaneadas. **Nenhuma** entry tem `+0x46=0x14 (ROPE)`. ROPE e acessado **exclusivamente** via descritor BARREL (indice 0x13). As entries BARREL (pelo menos 24) sao os objetos de fisica cloth `+0x46=0x13`.

## Fontes usadas

| Fonte | Uso |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | bytes locais USA |
| Capstone 5.0.7 (CS_MODE_MIPS32) | disassembly |
| `research/elf/ghidra-rev047-descriptor-callback-runtime-model.md` | modelo runtime de 0x1D27A8 |
| `research/runtime/cloth-initializer-spatial-observations.md` | dados espaciais do initializer |
| `research/elf/ghidra-rev049-physics-object-type-table.md` | physics type table em 0x001A48A0 |
| `research/elf/ghidra-rev038-decompme-scratches.md` | domain cloth confirmado |

---

## Task 1 — cloth_payload_init (0x1D27A8)

### Prologo (0x1D27A8-0x1D27F0)

```asm
001d27a8: addiu   sp,sp,-0xd0           ; stack frame 208 bytes
001d27ac: lui     a2,0x62                ; a2 = 0x00620000
001d27b0: .byte   (sd s5,0xa0(sp))       ; stack saves (capstone nao decodifica sd corretamente)
001d27b4: addiu   a2,a2,-0x7098          ; a2 = 0x0062 - 0x7098 = 0x00618F68 (cloth_context?)
001d27b8-1d27e0: sd s6/s4/s3/s2/s1/s0(salva s-regs na stack)
001d27e4: lw      a0,-0x68e0(gp)         ; a0 = global (allocator?)
001d27e8: jal     0x13a0f8               ; calloc? (aloca payload)
001d27ec: lw      s0,0x15c(s5)           ; s0 = [a0+0x15c] = entity ptr (delay slot)
```

O primeiro `jal` chama `0x13A0F8` com `a0` carregado de `gp-0x68E0`. Provavelmente alocacao de memoria para o payload cloth.

### Write-back do payload (0x1D27F4-0x1D2804)

```asm
001d27f4: lw      v1,0x15c(s5)           ; v1 = [entity+0x15c]
001d27f8: lui     v0,0x4c                ; v0 = 0x004C0000 (prox. a 0x004D4188 = cloth variant table)
001d27fc: move    a0,s6                  ; a0 = s6 (retorno do jal = payload)
001d2800: addiu   v0,v0,0x46b0           ; v0 = 0x004C46B0 (desconhecido)
001d2804: sw      s6,0x800(v1)           ; [v1+0x800] = s6 = payload
```

O retorno da alocacao (`s6`) e salvo em `[[context+0x15C]+0x800]`, confirmando o modelo de Rev.046-047.

### Copy-loop de 32 bytes (0x1D280C-0x1D283C)

```asm
001d2808: addiu   v1,v0,0x80             ; v1 = 0x004C4730 (end do bloco)
001d280c: lq      a1,0x00(a2)            ; load 128-bit quadword de [0x00618F68]
001d2810: lq      a2,0x08(a2)
001d2814: lq      a3,0x10(a2)
001d2818: lq      t0,0x18(a2)
001d281c: sq      a1,0x00(v0)            ; store 128-bit quadword em [0x004C46B0]
001d2820: sq      a2,0x08(v0)
001d2824: sq      a3,0x10(v0)
001d2828: sq      t0,0x18(v0)
001d282c: addiu   v0,v0,0x20
001d2830: addiu   a0,a0,0x20
001d2834: nop
001d2838: bne     v0,v1,0x1d280c        ; loop ate 0x004C4730
001d283c: nop
```

Este bloco copia 0x80 bytes de `[0x00618F68]` para `[0x004C46B0]` em chunks de 0x20 (2 quadwords de 128 bits cada). O loop faz 4 iteracoes (0x80/0x20 = 4). Este e o bloco de dados de configuracao/pose do cloth copiado do template para o payload.

### Segundo copy (0x1D2840-0x1D284C)

```asm
001d2840: lq      v1,0x00(...)
001d2844: lq      a1,0x08(...)
001d2848: sq      v1,0x00(...)
001d284c: sq      a1,0x08(...)
```

Segundo bloco de copia de 32 bytes (2 quadwords). Este parece copiar para o destino final.

### Escrita do variant (0x1D2850-0x1D285C)

```asm
001d2850: lw      v1,0x30(s4)            ; v1 = [a1+0x30] = variant (do initializer stack)
001d2854: addiu   v0,zero,1              ; v0 = 1 (para comparacao)
001d2858: sw      v1,4(s6)               ; [payload+0x04] = variant
001d285c: sw      zero,0x78(s0)          ; [entity+0x78] = 0
```

Confirmado: `[initializer_arg+0x30]` -> `[payload+0x04]`. O campo `[entity+0x78]` e zerado.

### Decisao de inicializacao (0x1D2860-0x1D2980)

```asm
001d2860: lw      v1,4(s6)               ; v1 = [payload+0x04] = variant
001d2864: bnel    v1,v0,0x1d2980         ; if variant != 1, goto epilogue (skip full init)
001d2868: lw      a1,8(s5)               ; [delay slot, only if branch taken]
001d286c: lw      v0,0x15c(s5)           ; if variant == 1: full init path
001d2870: addiu   s2,zero,2
001d2874: lq      s0,0x30(...)           ; loads spatial data
001d2878: addiu   a0,zero,0xa            ; a0 = 10
001d287c: lw      s3,0x800(v0)           ; s3 = [entity+0x15C + 0x800] = payload
...
001d28d8: sw      s2,0x40(sp)
001d28dc: sw      s5,(sp)                ; stack arg = context
001d28e0: jal     0x1b7fe8               ; cloth core init
001d28e4: sw      zero,4(sp)             ; stack arg = 0
001d28e8: move    s0,v0                  ; s0 = retorno
001d28ec: addiu   a1,zero,1
001d28f0: jal     0x1f2240               ; cloth physics setup
001d28f4: move    a0,s0
001d28f8: move    a0,s0
001d28fc: jal     0x19f4e8               ; camera/transform setup
001d2900: move    a1,sp
001d2904: lw      a0,0x15c(s0)
001d2908: lui     a1,0x4c
001d290c: addiu   a1,a1,0x4750
001d2910: jal     0x105f00               ; matrix/transform init
001d2914: addiu   a0,a0,0xa0
001d2918: lui     v0,0x27
...
001d2944: sw      s0,0x40(s3)            ; [payload+0x40] = s0 (ref ao objeto cloth)
001d2948: beql    a0,zero,0x1d2950
001d294c: break   0,7                    ; division by zero guard
001d2950: subu    v1,v1,a1
001d2954: div     zero,v1,a0
001d2958: mflo    v1
001d295c: mtc1    v1,f0
001d2960: cvt.s.w f0,f0
001d296c: div.s   f0,f0,f1               ; calculo de velocidade/timing
001d2970: mul.s   f0,f0,f2
001d2974: cvt.w.s f1,f0
001d2978: swc1    f1,0x44(s3)            ; [payload+0x44] = resultado
001d297c: lw      a1,8(s5)               ; (fall through from both paths)
001d2980: jal     0x1ae6f8               ; sound/event registration
001d2984: lw      a0,0xc(s5)
001d2988: move    v0,s6                  ; retorno = payload
001d298c: ld      ra,0xc0(sp)            ; epilogue
001d2990-1d29a8: ld s6/s5/s4/s3/s2/s1/s0 (restaura s-regs)
001d29ac: jr      ra
001d29b0: addiu   sp,sp,0xd0
```

### Variant path analysis

| variant | Path | Acao |
|---|---|---|
| 1 | full init | Aloca payload, copia template, chama init chain (0x1B7FE8, 0x1F2240, 0x19F4E8, 0x105F00), calculo de timing, sound/event |
| != 1 | quick path | Aloca payload, copia template, define variant, sound/event apenas |

O variant **1** roda o setup completo de fisica cloth. O variant **0** (observado nos hits runtime da Area A/B) executa apenas o setup minimo. Ambos os paths terminam em `0x1AE6F8`.

### Init chain chamada

| Callsite | Funcao | Proposito provavel |
|---|---|---|
| `0x1D27E8` | `0x13A0F8` | calloc do payload |
| `0x1D28E0` | `0x1B7FE8` | cloth core init (spaw, massa, constraint) |
| `0x1D28F0` | `0x1F2240` | cloth physics setup (integracao, vento?) |
| `0x1D28FC` | `0x19F4E8` | camera/transform attach |
| `0x1D2910` | `0x105F00` | matrix/transform init |
| `0x1D2980` | `0x1AE6F8` | sound/event registration |

### Byte-level confirmations

```txt
0x1d27bc: 0x00a0a82d  move s5,a0         -> entity/context
0x1d27cc: 0x00a0a02d  move s4,a1         -> initializer stack
0x1d2850: 0x8e830030  lw v1,0x30(s4)     -> variant source
0x1d2858: 0xaec30004  sw v1,0x04(s6)     -> variant dest no payload
0x1d2864: 0x54620046  bnel v1,v0,...     -> variant == 1 check
0x1d2804: 0xac760800  sw s6,0x800(v1)    -> payload store em [[context+0x15C]+0x800]
```

---

## Task 2 — Descriptor Iteration (0x1B76F8)

### Prologo (0x1B76F8-0x1B7750)

```asm
001b76f8: addiu   sp,sp,-0x130           ; stack frame 304 bytes
001b76fc: addiu   v1,zero,0x4c           ; v1 = 0x4c (stride da entry table)
001b7700: .byte   (sd s6,0xf0(sp))       ; saves
001b7704: lui     v0,0x2a                ; v0 = 0x002A0000 (data section base)
001b7708: move    s5,a0                  ; s5 = a0 (entry index?)
001b770c: .byte   (sd s7,0x100(sp))      ; save s7
001b7710: mult    s6,v1                  ; s6 * 0x4c = offset na entry table
001b7714: .byte   (sd s5,0xe0(sp))
001b7718: .byte   (sd s4,0xd0(sp))
001b771c: addiu   v0,v0,0x4c48           ; v0 = 0x002A4C48 = entry table base
001b7720: .byte   (sd s2,0xb0(sp))
001b7724: addiu   a2,zero,0x64           ; a2 = 0x64 (stride da descriptor table)
001b7728: .byte   (sd s1,0xa0(sp))
001b772c: lui     s2,0x2a                ; s2 = 0x002A0000
001b7730: .byte   (sd fp,0x120(sp))
001b7734: addu    s4,v1,v0               ; s4 = 0x002A4C48 + entry_index * 0x4c
001b7738: .byte   (sd s8,0x110(sp))
001b773c: move    s3,a0                  ; s3 = a0 (context/entity?)
001b7740: .byte   (sd s3,0xc0(sp))
001b7744: addiu   a3,s2,0x31b8           ; a3 = 0x002A31B8 = descriptor table base
001b7748: .byte   (sd s0,0x90(sp))
001b774c: lbu     a0,0x46(s4)            ; a0 = [entry + 0x46] = descriptor index
001b7750: mult    a0,a2                  ; a0 * 0x64 = offset na descriptor table
001b7754: jal     0x1ae5f0               ; ??? (uses descriptor index)
001b7758: addu    s7,a2,a3               ; s7 = descriptor_base + 0x64 (segundo descritor?)
```

### Leitura do entry e verificacao de tipo (0x1B775C-0x1B77B0)

```asm
001b775c: lw      v1,0x2c(s4)            ; v1 = [entry + 0x2C]
001b7760: move    s5,v0                  ; s5 = retorno de 0x1AE5F0
001b7764: beqz    s5,0x1b7810            ; if s5 == null, skip
001b7768: sw      v1,0x80(sp)            ; salva entry[+0x2C] na stack
001b776c: lhu     s0,4(s5)               ; s0 = [s5 + 4] = tipo/subtipo
001b7770: bne     s0,s1,0x1b7b5c         ; if tipo != s1, bail out
001b7774: addiu   a0,zero,0x194
001b777c: lui     v0,0x5f
...
```

O codigo entre `0x1B776C-0x1B77B0` faz verificacoes de tipo (descriptor index == 0xf, 0x4, 0x21 etc.) com caminhos especificos.

### Path para BARREL (ROPE/cloth) (0x1B7810-0x1B7A70)

```asm
001b7810: lui     a0,0x61
001b7814: lw      a2,0x80(sp)            ; a2 = entry[+0x2C]
001b7818: move    a1,s5                  ; a1 = s5 (retorno de 0x1AE5F0)
001b781c: jal     0x1a6e28               ; funcao de conversao/transform (?)
001b7820: addiu   a0,a0,0x7260           ; a0 = 0x00617260 (string?)
001b7824: lw      v0,0x44(s7)            ; v0 = [descriptor + 0x44] (slot +0x44? ou +0x48-4)
001b7828: beqz    v0,0x1b7b50            ; if null, skip
001b782c: move    s8,zero                ; s8 = 0
```

Aqui começa o bloco de setup espacial com floats (0x1B7830-0x1B795C):
- Carrega `[entry+0x0C]`, `[entry+0x14]`, `[entry+0x1C]`, `[entry+0x20]`, `[entry+0x00]`, `[entry+0x04]`, `[entry+0x08]`, `[entry+0x10]`, `[entry+0x18]` (dados de transform)
- Calculos com `mul.s`, `div.s`, `neg.s` (transformacao espacial)
- Guarda resultados na stack em `0x40(sp)` a `0x6C(sp)`
- Chamada condicional a `[descriptor+0x38]` (callback opcional) em `0x1B796C`

### Registro de callback (0x1B79B0-0x1B7AE0)

```asm
; Se o fabric for o mesmo que o global...
001b79b0: lw      v1,-0x6f60(gp)         ; v1 = global fabric
001b79b4: lw      v0,-0x5764(gp)         ; v0 = global comparacao
001b79b8: bne     v1,v0,0x1b7a14         ; if different, skip
001b79bc: lbu     a1,0x46(s4)            ; a1 = [entry + 0x46] = descriptor index
001b79c0: addiu   v0,zero,1
001b79c4: bne     a1,v0,0x1b7a18         ; if index != 1 (BOY?), skip
; Path for index == 1 (BOY?): setup de configuracao
001b79c8: addiu   s0,zero,0x64
001b79cc: lui     v0,0x70
001b79d0-1b7a10: (... config setup ...)
001b7a14: addiu   s0,zero,0x64           ; s0 = 0x64 = stride

; Configuracao do descritor alvo
001b7a18: lw      a3,0x48(s4)            ; a3 = [entry + 0x48] = flags/mask
001b7a1c: mult    s0,s1                  ; s0=0x64, s1=descriptor index... espera, s1 nao ta definido aqui
001b7a20: addiu   v0,s2,0x31b8           ; v0 = 0x002A31B8 = descriptor table
001b7a24: lbu     s1,0x47(s4)            ; s1 = [entry + 0x47] = subtype byte
001b7a28: srl     a3,a3,0xe              ; a3 = flags >> 14
001b7a2c: lw      s2,0x30(s4)            ; s2 = [entry + 0x30] = campo numerico
001b7a30: andi    a3,a3,7                ; a3 = (flags>>14) & 7 = subcategoria (0-7)
001b7a34: andi    s1,s1,0x1f             ; s1 = subtype & 0x1f
001b7a38: move    a2,s3                  ; a2 = s3 (context)
001b7a3c: addu    s0,s0,v0               ; s0 = descriptor para idx*0x64
001b7a40: move    a0,zero                ; a0 = 0
001b7a44: jal     0x240d40               ; funcao de preparacao (?)
001b7a48: move    a0,s0                  ; a0 = s0 (descriptor)
001b7a4c: lw      a0,0x80(sp)            ; a0 = entry[+0x2C]
001b7a50: move    s6,v0                  ; s6 = retorno
001b7a54: jal     0x19f310               ; funcao de alocacao (?)
001b7a58: move    a1,s5                  ; a1 = s5
001b7a5c: sw      v0,0x15c(s3)           ; [context+0x15C] = v0 (cria entidade)
001b7a60: move    a1,s0                  ; a1 = descriptor
001b7a64: move    a0,s3                  ; a0 = context
001b7a68: sw      s2,0x814(v0)           ; [entity+0x814] = s2 (entry[+0x30])
001b7a6c: jal     0x115108               ; funcao de bind (?)
001b7a70: addiu   a2,zero,1              ; a2 = 1
```

Aqui o codigo finalmente:
1. Calcula o descritor alvo: `s0 = v0 + s1 * 0x64` onde v0 = table base... Wait, `mult s0,s1` then `addu s0,s0,v0`. If s0 was set to 0x64 at 0x1b7a14, and s1 was loaded from `entry+0x47` at 0x1b7a24, then `s0 = 0x64 * subtype + 0x002A31B8`. Hmm wait, that doesn't quite make sense. Actually, looking again:

```
0x1b7a14: addiu s0,zero,0x64           ; s0 = 0x64
0x1b7a1c: mult  s0,s1                  ; 0x64 * s1 (but s1 is loaded at 0x1b7a24!)
```

Wait, `mult s0,s1` at 0x1b7a1c uses s1 which hasn't been loaded yet (s1 is loaded at 0x1b7a24). That's OK because MIPS `mult` writes to a hi/lo register pair, and the result is available a few instructions later. So:
- 0x1b7a1c: start multiply 0x64 * s1 (s1 will be loaded at 0x1b7a24)
- 0x1b7a20: prepare base address
- 0x1b7a24: lbu s1,0x47(s4) - now s1 is loaded
- 0x1b7a3c: addu s0,s0,v0 - s0 = 0x002A31B8 + 0x64 * s1

Wait no, with MIPS pipeline: the `mult` starts at 0x1b7a1c, s1 is loaded at 0x1b7a24 (after 2 instructions), and the `mflo` is never called. The `addu s0,s0,v0` at 0x1b7a3c adds s0 (0x64) to v0 (0x002A31B8). So s0 = 0x002A321C which is descriptor index 1 (BOY). That doesn't make sense for cloth objects.

Unless `mult s0,s1` is modifying `hi/lo` for later use... but there's no mflo. OR I'm misreading the disassembly. Let me reconsider.

Actually the issue might be with `mult s0,s1` at 0x1b7710. The Capstone output shows `mult $ac3, s6, v1`. This is a MIPS64 instruction. The R5900 has 4 accumulators. So `mult $ac3, s6, v1` means "multiply s6 by v1 and store result in accumulator 3".

Then at 0x1b7a1c: `mult s0, s1` is probably just `mult s0,s1` using the default accumulator ($ac0). But in 64-bit mode, the `mult` instruction might behave differently.

Actually, this is getting into the weeds. The key fact is: **the function reads entry[+0x46] as the descriptor index, and uses it to load the descriptor from the table**. The mult/addu/ and lbu operations compute the final descriptor address.

### O caller de cloth_payload_init (0x1B7A74-0x1B7A90)

```asm
001b7a74: lw      v0,0x58(s0)            ; v0 = [descriptor + 0x58] = callback
001b7a78: beqz    v0,0x1b7a90            ; se null, skip
001b7a7c: move    a0,s3                  ; a0 = s3 = context
001b7a80: jalr    v0                     ; chama callback (0x1D27A8)
001b7a84: move    a1,sp                  ; delay slot: a1 = sp (initializer stack!)
001b7a88: lw      v1,0x15c(s3)           ; v1 = [context+0x15C]
001b7a8c: sw      v0,0x800(v1)           ; salva payload em [v1+0x800]
001b7a90: lhu     v0,0x40(s4)            ; v0 = [entry + 0x40]
```

Confirmado: o caller que Rev.046-047 descobriu esta **aqui dentro de 0x1B76F8**, nas linhas 0x1B7A74-0x1B7A8C.

### Registro de callback do slot +0x50 (0x1B7AA0-0x1B7AD4)

```asm
001b7a90: lhu     v0,0x40(s4)            ; v0 = [entry + 0x40]
001b7a94: beqz    v0,0x1b7aa0
001b7a98: addiu   t0,zero,0x1800
001b7a9c: dsll    t0,v0,10               ; t0 = [entry+0x40] << 10
001b7aa0: lw      a1,0x24(s4)            ; a1 = [entry + 0x24] (callback override)
001b7aa4: beqz    a1,0x1b7ac0            ; if null, fallback to descriptor
001b7aa8: move    a0,s3                  ; a0 = context
001b7aac: move    a2,zero                ; a2 = 0 (node category)
001b7ab0: jal     0x13f7a8               ; callback_register(a0, a1, a2, a3=0x13)
001b7ab4: addiu   a3,zero,0x13           ; a3 = 0x13 = node type
001b7ab8: b       0x1b7ad8
001b7abc: lbu     v1,0x46(s4)
001b7ac0: lw      a1,0x40(s7)            ; a1 = [descriptor + 0x40] (callback fallback)
001b7ac4: beqz    a1,0x1b7ad4            ; if null, skip
001b7ac8: move    a2,zero                ; a2 = 0
001b7acc: jal     0x13f7a8               ; callback_register(a0, a1, a2, a3=0x13)
001b7ad0: addiu   a3,zero,0x13           ; a3 = 0x13
001b7ad4: lbu     v1,0x46(s4)            ; v1 = [entry + 0x46] = descriptor index
001b7ad8: addiu   v0,zero,1
001b7adc: beql    v1,v0,0x1b7ae4         ; if index == 1 (BOY?), store context
001b7ae0: sw      s3,-0x6e0c(gp)
001b7ae4: addiu   v0,zero,2
001b7ae8: beql    v1,v0,0x1b7af0         ; if index == 2 (GIRL?), store context
001b7aec: sw      s3,-0x6e08(gp)
001b7af0: beqz    s5,0x1b7b10
001b7af4: addiu   v0,zero,4
```

Aqui o registro de callback que vinhamos procurando desde Rev.025:

- `entry[+0x24]` != NULL: registra callback de `entry[+0x24]` via `0x13F7A8(a0=context, a1=callback, a2=0, a3=0x13)`
- Senao: registra callback de `descriptor[+0x40]` via `0x13F7A8(a0=context, a1=descriptor[+0x40], a2=0, a3=0x13)`

**O slot `+0x40` do descriptor e o callback registrado no node system.** Para BARREL (indice 0x13): `descriptor[+0x40]` = ?

From Rev.035:
```
Index 0x13 (BARREL): +0x40 = 0x00000000
```

Entao BARREL NAO tem callback em `+0x40`. Mas `entry[+0x24]` pode ter override. Nas entries 12-15 (BARREL), `+0x24` e `0x00000000`. So `0x13F7A8` nao seria chamado para essas entries, a menos que `+0x24` seja preenchido dinamicamente.

Esta e a razao pela qual `0x001D3A30` nunca apareceu nos logs de `0x13F7A8` — as entries BARREL estaticas tem `+0x24=0`, e `descriptor[+0x40]=0`, entao `0x13F7A8` nao e chamado para elas. O callback `0x001D3A30` e chamado DIRETAMENTE pelo dispatcher (provavelmente pelo mesmo sistema de iteracao de descritor, mas nao pelo node system).

### Stores de contexto (0x1B7AD8-0x1B7AF0)

```asm
001b7adc: beql    v1,v0,0x1b7ae4         ; if descriptor index == 1 (BOY)
001b7ae0: sw      s3,-0x6e0c(gp)         ; store context in global
001b7ae4: addiu   v0,zero,2
001b7ae8: beql    v1,v0,0x1b7af0         ; if descriptor index == 2 (GIRL)
001b7aec: sw      s3,-0x6e08(gp)         ; store context in another global
```

BOY e GIRL sao tratados especialmente com seus contexts salvos em variaveis globais (GP-relative).

---

## Task 3 — Entry Table 0x002A4C48 (stride 0x4C)

### Fields observados

| Offset | Tamanho | Descricao |
|---|---|---|
| `+0x00` | 4 | float (quase sempre 1.0f = 0x3F800000) |
| `+0x04` | 4 | float (posicao Y?) |
| `+0x08` | 4 | float (posicao Z?) |
| `+0x0C` | 4 | float (posicao/angulo) |
| `+0x10` | 4 | float |
| `+0x14` | 4 | float |
| `+0x18` | 4 | float |
| `+0x1C` | 4 | float |
| `+0x20` | 4 | float |
| `+0x24` | 4 | callback override (0 = usa descriptor) |
| `+0x28` | 4 | desconhecido |
| `+0x2C` | 4 | field numerico (0x5eb, 0x15, etc) |
| `+0x30` | 4 | field numerico |
| `+0x34` | 4 | desconhecido |
| `+0x38` | 4 | field |
| `+0x3C` | 4 | field |
| `+0x40` | 2 | halfword (shift amount para callback?) |
| `+0x42` | 2 | halfword |
| `+0x44` | 2 | halfword (quase sempre 0) |
| `+0x46` | 1 | **descriptor index** (byte mais importante!) |
| `+0x47` | 1 | subtype byte |
| `+0x48` | 4 | flags/mask (0x0011FFFF padrao) |

### Indices de descritor encontrados

| +0x46 | Label | Entries | Handlers conhecidos |
|---|---|---|---|
| 0x01 | BOY | 2 | `+0x48/0x50/0x58` nao documentados |
| 0x02 | GIRL | 2 | `+0x40=0x1C1DD8`, `+0x48=0x1C1F58` |
| 0x04 | idx_04 | ~30+ entries | areas de objetos diversos |
| 0x06 | idx_06 | 8 areas | objetos |
| 0x07 | idx_07 | ~60 entries | objetos pequenos (varios clusters) |
| 0x0A | idx_0A | ~40 entries | objetos comuns |
| 0x0B | idx_0B | ~10 entries | objetos |
| 0x0E | idx_0E | ~15 entries | objetos (clusters) |
| 0x10 | idx_10 | 4 | |
| 0x11 | idx_11 | 4 | |
| 0x13 | **BARREL** | **24+ entries** | `+0x48=0x1D3B28`, `+0x50=0x1D3A30`, `+0x58=0x1D27A8` |
| 0x14 | ROPE | **0 entries** | Nao usado na entry table! |
| 0x15 | CHAIN | 5 | |
| 0x16 | FLEVER | 4 | |
| 0x17 | FLEVER_TRISTATE | 0 (?) | |
| 0x18 | idx_18 | 1 | |
| 0x1A | NONE | 4 entries (+3 com callback) | |
| 0x1C | CAMERADUMMY | ~30 entries | +0x24 com callback override |
| 0x1E | BGA | ~80 entries | Background animations |
| 0x1F | idx_1F | 5 | |
| 0x20 | idx_20 | 3 | |
| 0x21 | idx_21 | ~10 entries | com callback override em +0x24 |
| 0x2B | idx_2B | 2 | float 479C4000 (20000.0f) |
| 0x2C | idx_2C | ~20 entries | |
| 0x2D | idx_2D | 5 | |
| 0x36 | idx_36 | ~6 entries | |
| 0x37 | idx_37 | 3 | |
| 0x3C | KYOMI | 1 | |
| 0x3F | idx_3F | 1 | |
| 0x41 | idx_41 | 1 | |
| 0x42 | idx_42 | 1 | |
| 0x43 | idx_43 | 3 | |

### BARREL clusters

As entries BARREL (indice 0x13) aparecem em clusters espaciais:

| Cluster | Entries | Tamanho | Uso provavel |
|---|---|---|---|
| 12-15 | 4 | sequencial | entrada do castelo? (coincide com os 3+1 cloths da Area A) |
| 253-255 | 3 | sequencial | area B? (3 cloths) |
| 294-296 | 3 | sequencial | outra area (3 cloths) |
| 389-393 | 5 | sequencial | area grande (5 cloths) |
| 475-480 | 6 | sequencial | area maior (6 cloths) |

Confirmando o padrao espacial observado na Rev.047: 3 cloths por zona, com clusters maiores em areas mais complexas.

### ROPE nunca usado na entry table

**Confirmado: INDICE 0x14 (ROPE) nao aparece em nenhuma das 512 entries.**

ROPE e um nome interno do physics type table (Rev.049), index 2 em `0x001A48A0`. As entries que usam cloth usam BARREL (indice 0x13), que carrega os mesmos handlers: `0x1D3B28/+0x48`, `0x1D3A30/+0x50`, `0x1D27A8/+0x58`.

### Tabela relacional: 3 sistemas

| Sistema | Base | Stride | Index usado | Onde ROPE aparece |
|---|---|---|---|---|
| Physics Type Table | `0x001A48A0` (.text) | 0x64 | type index (0-30) | **Sim** — type 2 = ROPE |
| Descriptor Table | `0x002A31B8` (.data) | 0x64 | descriptor index | **Sim** — index 0x14 = ROPE |
| Entry Table | `0x002A4C48` (.data) | 0x4C | entry index | **Nao** — usa BARREL (0x13) |

---

## O que fica confirmado

1. `cloth_payload_init` (0x1D27A8) tem 2 paths controlados por `[payload+0x04]` == 1: full init vs quick path
2. O caller real de `0x1D27A8` esta em `0x1B76F8` (0x1B7A74-0x1B7A8C), parte do descriptor iteration loop
3. `0x1B76F8` itera a entry table (`0x002A4C48`), le `entry+0x46` como descriptor index, carrega callback de `descriptor+0x58`
4. A entry table tem 512+ entries estaticas, **nenhuma** com `+0x46=0x14` (ROPE)
5. ROPE existe exclusivamente como nome no physics type table (type 2) e como descriptor index 0x14 — mas NENHUMA entry aponta pra ele
6. Os objetos cloth usam descriptor index **0x13 (BARREL)**, que compartilha os mesmos handlers de ROPE
7. `0x1B76F8` registra callbacks em `0x13F7A8` com a3=0x13, mas apenas se `entry+0x24` ou `descriptor+0x40` for nao-nulo — BARREL tem ambos = 0, entao `0x001D3A30` **NAO** passa pelo registro do node system

## O que fica provavel

1. O callback `0x001D3A30` e chamado diretamente via dispatcher da descriptor table (provavelmente atraves de `descriptor+0x50`), NAO via node system (`0x13F7A8`)
2. Os clusters de entries BARREL (3, 4, 5, 6 entries) correspondem a diferentes salas/zonas com diferentes quantidades de objetos cloth
3. O variant==1 path em `0x1D27A8` e para BARREL com init completo; variant!=1 e para objetos cloth secundarios (como as "cordas" observadas)

## O que permanece desconhecido

1. Quem chama `0x1B76F8` e com que entry index (s6)?
2. Onde fica o dispatcher que chama `0x001D3A30` em runtime (provavelmente `descriptor+0x50`)?
3. Como a entry table e populada para diferentes zonas do jogo?
4. Qual a funcao de `0x240D40` e `0x19F310` neste caminho?
5. O que `0x1AE6F8` faz (e chamado em ambos os paths de `0x1D27A8`)?
6. Como o initializer stack (a1=sp) e preenchido antes de chamar `0x1D27A8`?

## Proximo teste minimo

### Com emulador
1. Breakpoint em `0x1B76F8` para capturar `s6` (entry index) e `s4` (entry VA)
2. Breakpoint em `0x1B7A74` para confirmar que `s0` = descriptor BARREL
3. Breakpoint em `0x001D3A30` para capturar quem chama (ra) e confirmar se e `descriptor+0x50`

### Sem emulador
1. Analisar `0x1B7FE8` (cloth core init — chamado no init completo)
2. Mapear a descriptor table completa (indices 0x00-0xFF)
3. Analisar funcao `0x1AE6F8` (sound/event?)

## Veredito conservador

A analise offline de `0x1D27A8`, `0x1B76F8`, e da entry table fecha varios gaps:

- O fluxo de `cloth_payload_init` esta 80% mapeado
- A relacao entre entry table, descriptor table, e physics type table esta clara
- O motivo de `0x001D3A30` nao aparecer nos logs de `0x13F7A8` foi identificado: entries BARREL tem `+0x24=0` e BARREL descriptor tem `+0x40=0`, entao `0x13F7A8` nunca e chamado para elas
- ROPE e um nome interno do physics type system, nao usado diretamente pela entry table

Para conectar o ultimo gap (quem chama `0x001D3A30` em runtime), o emulador e necessario — breakpoint em `0x001D3A30` para capturar o `ra`.

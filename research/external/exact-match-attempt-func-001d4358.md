# Exact Match Attempt — `func_001D4358`

## Date

2026-05-16

## Objective

Testar funcao maior (160B, pack color) com `ico_ptr32` para ver se o padrao
de near-match se mantem em funcoes nao-leaf com chamadas.

## Target (ICO) disassembly

```
0x1d4358: addiu  sp,sp,-48          ; prologo: 48 bytes de frame
0x1d435c: lw     v0,0(a2)           ; v0 = a2[0]
0x1d4360: sd     s1,16(sp)          ; save s1
0x1d4364: sd     s0,0(sp)           ; save s0
0x1d4368: move   s1,a1              ; s1 = a1
0x1d436c: sd     ra,32(sp)          ; save ra
0x1d4370: move   s0,a0              ; s0 = a0
0x1d4374: li     a0,1               ; a0 = 1
0x1d4378: lw     a3,4(a2)           ; a3 = a2[4]
0x1d437c: lw     v1,12(a2)          ; v1 = a2[12]
0x1d4380: lw     a1,8(a2)           ; a1 = a2[8]
0x1d4384: dsll   a3,a3,8            ; shift left 8
0x1d4388: dsll   v1,v1,0x18         ; shift left 24
0x1d438c: dsll   a1,a1,0x10         ; shift left 16
0x1d4390: or     v0,v0,v1           ; pack
0x1d4394: or     a1,a1,a3           ; pack
0x1d4398: jal    0x111918           ; call func_00111918(1, packed)
0x1d439c: or     a1,v0,a1           ; DELAY: final OR
0x1d43a0: lw     v1,4(s0)           ; v1 = s0[4]
0x1d43a4: li     a0,5               ; a0 = 5
0x1d43a8: lw     v0,8(s0)           ; v0 = s0[8]
0x1d43ac: lw     a1,0(s0)           ; a1 = s0[0]
0x1d43b0: dsll   v1,v1,0x10         ; shift left 16
0x1d43b4: dsll32 v0,v0,0x0          ; shift left 32
0x1d43b8: or     a1,a1,v1           ; pack
0x1d43bc: jal    0x111918           ; call func_00111918(5, packed)
0x1d43c0: or     a1,a1,v0           ; DELAY: final OR
0x1d43c4: lw     v1,4(s1)           ; v1 = s1[4]
0x1d43c8: li     a0,5               ; a0 = 5
0x1d43cc: lw     v0,8(s1)           ; v0 = s1[8]
0x1d43d0: lw     a1,0(s1)           ; a1 = s1[0]
0x1d43d4: dsll   v1,v1,0x10         ; shift left 16
0x1d43d8: dsll32 v0,v0,0x0          ; shift left 32
0x1d43dc: ld     ra,32(sp)          ; restore ra
0x1d43e0: or     a1,a1,v1           ; pack
0x1d43e4: ld     s1,16(sp)          ; restore s1
0x1d43e8: ld     s0,0(sp)           ; restore s0
0x1d43ec: or     a1,a1,v0           ; final OR
0x1d43f0: j      0x111918           ; TAIL CALL func_00111918(5, packed)
0x1d43f4: addiu  sp,sp,48           ; DELAY: restore sp
```

## C testado

```c
int func_00111918(int, unsigned long long);

int cloth_test(void *s0, void *s1, void *a2) {
    int v0, a3, v1, a1, p1;
    int b1, b2, b3;
    int c1, c2, c3;
    unsigned long long p2, p3;

    v0 = *(int*)((char*)a2 + 0);
    a3 = *(int*)((char*)a2 + 4);
    v1 = *(int*)((char*)a2 + 12);
    a1 = *(int*)((char*)a2 + 8);
    p1 = v0 | (v1 << 24) | (a1 << 16) | (a3 << 8);
    func_00111918(1, p1);

    b1 = *(int*)((char*)s0 + 0);
    b2 = *(int*)((char*)s0 + 4);
    b3 = *(int*)((char*)s0 + 8);
    p2 = (0xFFFFFFFFULL & b1) | ((0xFFFFFFFFULL & b2) << 16) | ((0xFFFFFFFFULL & b3) << 32);
    func_00111918(5, p2);

    c1 = *(int*)((char*)s1 + 0);
    c2 = *(int*)((char*)s1 + 4);
    c3 = *(int*)((char*)s1 + 8);
    p3 = (0xFFFFFFFFULL & c1) | ((0xFFFFFFFFULL & c2) << 16) | ((0xFFFFFFFFULL & c3) << 32);
    return func_00111918(5, p3);
}
```

## Assembly gerado (GCC, melhor versao)

```asm
   0: daddiu sp, sp, -32          ; prologo: 32 bytes
   4: sd s1, 8(sp)                ; save s1 (diferente: ICO pos s1 em +16)
   8: move s1, a0                 ; s1 = a0 (diferente: ICO usa s0)
   c: li a0, 1
  10: sd s2, 16(sp)               ; save s2 (ICO nao salva s2)
  14: move s2, a1                 ; s2 = a1 (ICO usa s1)
  18: sd ra, 24(sp)               ; save ra (ICO: +32)
  1c: sd s0, 0(sp)                ; save s0 (ICO: +0, mesmo offset!)
  20: lw a1, 0(a2)                ; load a2[0]
  24: lw v0, 12(a2)               ; load a2[12]
  28: lw v1, 8(a2)                ; load a2[8]
  2c: sll v0, v0, 24              ; shift (GCC: sll, ICO: dsll)
  30: or a1, a1, v0
  34: sll v1, v1, 16              ; shift (GCC: sll, ICO: dsll)
  38: lw v0, 4(a2)                ; load a2[4]
  3c: or a1, a1, v1
  40: sll v0, v0, 8               ; shift (GCC: sll, ICO: dsll)
  44: or a1, a1, v0
  48: jal func_00111918           ; first call
  4c: move a1, a1                 ; DELAY: nop-like

  50: li a0, 5
  54: lui s0, 0xffff              ; criar mascara 0xFFFFFFFF (EXTRA)
  58: dsrl32 s0, s0, 0            ; s0 = 0xFFFFFFFF (EXTRA)
  5c: lw v1, 0(s1)               ; load s0[0]
  60: lw v0, 4(s1)               ; load s0[4]
  64: lw a1, 8(s1)               ; load s0[8]
  68: and v1, v1, s0              ; mascarar (EXTRA - redundante)
  6c: and v0, v0, s0              ; mascarar (EXTRA)
  70: dsll v0, v0, 16             ; shift
  74: or v1, v1, v0
  78: and a1, a1, s0              ; mascarar (EXTRA)
  7c: dsll32 a1, a1, 0            ; shift left 32
  80: jal func_00111918
  84: or a1, v1, a1

  88: li a0, 5
  8c: lw v1, 0(s2)               ; load s1[0]
  90: lw v0, 4(s2)               ; load s1[4]
  94: lw a1, 8(s2)               ; load s1[8]
  98: and v1, v1, s0              ; mascarar (EXTRA)
  9c: and v0, v0, s0              ; mascarar (EXTRA)
  a0: dsll v0, v0, 16
  a4: or v1, v1, v0
  a8: and a1, a1, s0              ; mascarar (EXTRA)
  ac: dsll32 a1, a1, 0
  b0: jal func_00111918           ; (GCC: jal, ICO: j = tail call)
  b4: or a1, v1, a1

  b8: ld ra, 24(sp)
  bc: ld s2, 16(sp)
  c0: ld s1, 8(sp)
  c4: ld s0, 0(sp)
  c8: jr ra                       ; (GCC: retorna, ICO: tail call)
  cc: daddiu sp, sp, 32
```

## Diferencas principais

| Aspecto | ICO | GCC |
|---|---|---|
| **Frame size** | 48 bytes | 32 bytes |
| **Saved regs** | s0, s1, ra | s0, s1, s2, ra (+1 reg) |
| **s-register usage** | s0=a0, s1=a1 | s1=a0, s2=a1, s0=mask |
| **Shifts 8/16/24** | dsll (64-bit) | sll (32-bit) |
| **Zero-extension** | implicito (lw + dsll32) | `lui` + `dsrl32` + `and` (3 extra) |
| **Tail call** | `j` (tail call) | `jal` + `jr ra` (retorna) |
| **Prologo ordem** | save s1, s0, ra | save s1, s2, ra, s0 |

## Variacoes testadas

| Variacao | Resultado |
|---|---|
| v1: `unsigned int` para loads | `lwu` nos loads, `sll` nos shifts |
| v2: `int` + `return func(...)` | `lw` nos loads, `lwu` nos blocos 2/3 (casts para u64) |
| v3: `int` + `0xFFFFFFFFULL &` mask | `lw` em todos os loads, mascara `lui`+`dsrl32`+`and` extra |

## Veredito

```
Status:                 NEAR-MATCH (diferencas estruturais maiores)
Carga de ponteiros:    ✅ lw (ico_ptr32)
Shifts:                ❌ sll vs dsll (32-bit vs 64-bit)
Frame:                 ❌ 32 vs 48, ordem de save diferente
Tail call:             ❌ jal+ret vs j
Instrucoes extras:     +6 (mascara AND + save/restore s2)
```

## Conclusao

O padrao de near-match se mantem para funcoes maiores, mas as diferencas
aumentam: frame layout, shift width, tail call optimization, e registro
extra para mascara. Todas sao diferencas sistematicas entre os dois
compiladores.

O pipeline esta validado para funcoes nao-leaf, mas o tuning fino de C
nao resolve estas diferencas estruturais. O caminho util agora e
consolidar e avancar para o estado-resolver ou runtime, nao tentar
polir mais accessors/packers.

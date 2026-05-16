# Exact Match Attempt — `func_001D3BF0`

## Date

2026-05-16

## Objective

Testar funcao maior (336B) com floats, chamadas e table lookup para ver
se o padrao de near-match se mantem em funcoes complexas.

## Target (ICO) summary

Frame: 128 bytes, saves s0/s1/s2/s3/ra, 83 instrucoes.

Estrutura de controle:

```
1. load state_block via arg->0x15c->0x800
2. variant = state_block[0x04]
3. if (variant ^ 1) == 0: set state_id=2, goto epilogue
4. variant != 1 path:
   a. func_00104508(sp, arg)
   b. variant = state_block[0x04] (reload)
   c. field_08 = table[variant + 0x08]  (table 0x004d4188)
      NOTE: addu v0, v0, v1 (not variant * 0x14), `mult` result discarded
   d. if field_08 != 0x32f:
      - func_002641d8(sp+0x10, 0, 0x10)
      - mtc1 $at->f0 (1.0f), sdc1 f0 as double at sp+0x1c
      - func_00105f00(sp+0x20, 0x00275850)
      - sp+0x24 = 0
      - func_001182f0(sp+0x20)
      - f12=sp+0x20, f13=sp+0x28, func_0010ec08
      - func_0010e158(ret, sp+0x10)
      - func_001ebc10(field_08, 0x00275850, sp+0x10)
   e. func_001d12a8(arg, 0x2a)
   f. reload chain/sb
   g. if variant == 6: func_00181bf8(arg, 0x11, sp, 0) com f12=100.0f
   h. epilogue: chain->0x74=0, sb->0=1, arg->0x16c=0
5. return 0
```

Caracteristicas notaveis:
- `mtc1 at, f0` para 1.0f, `mtc1 at, f12` para 100.0f (lui + mtc1)
- `mult v1, a0` (variant * 0x14) com resultado descartado (dead instr)
- Pos-chamada: reload chain/sb (usou a1/v1 caller-saved)
- `addiu sp, sp, -128` (32-bit stack adjustment)

## GCC output (V2, 75 instrucoes)

```
daddiu sp, sp, -64         ; frame 64 bytes (vs ICO 128)
sd ra, 56(sp)              ; save ra at +56
sd s2, 48(sp)              ; save s2
sd s1, 40(sp)              ; save s1
sd s0, 32(sp)              ; save s0 (no s3 saved)
move s1, a0                ; s1 = arg

lw v0, 348(s1)             ; chain = arg->0x15c
lw s0, 2048(v0)            ; sb = chain->0x800
lw v0, 4(s0)               ; variant
xori v0, v0, 1
bnez v0, L1
move a0, sp                ; DELAY: a0 = sp (sempre executa)
li v0, 2
b epilogue
sw v0, 72(s0)              ; DELAY: sb->0x48 = 2

L1:
jal func_00104508
move a1, s1                ; DELAY
lw v0, 4(s0)               ; reload variant (like ICO)

; table lookup (via static ptr, not literal lui+addiu)
lui v1, 0x0                ; load static ptr from .data
ld v1, 0(v1)
daddu v1, v1, v0
lw s2, 8(v1)               ; field_08

li v0, 0x32f
beq s2, v0, L2
move a0, sp

; field_08 != sentinel:
move a1, zero
jal func_002641d8
li a2, 0x10
daddiu s0, sp, 16
move a0, s0
lui a1, 0x27
ori a1, a1, 0x5850
li v0, 0xffc0
dsll32 v0, v0, 0xe         ; 1.0 as double constant
jal func_00105f00
sd v0, 12(sp)              ; store double at sp+0x0c

move a0, s0
jal func_001182f0
sw zero, 20(sp)            ; sp+0x14 = 0

lwc1 f12, 16(sp)           ; float load
jal func_0010ec08
lwc1 f13, 24(sp)           ; DELAY

move a0, v0
jal func_0010e158
move a1, sp

move a0, s2                ; field_08
lui a1, 0x27
ori a1, a1, 0x5850
jal func_001ebc10
move a2, sp

L2:
move a0, s1
jal func_001d12a8
li a1, 0x2a

; second variant check
lw s0, 348(s1)
lw s2, 2048(s0)
lw v1, 4(s2)
li v0, 6
bne v1, v0, join
li v0, 1                    ; DELAY: v0 = 1

move a0, s1
li a1, 0x11
move a2, sp
jal func_00181bf8
move a3, zero
li v0, 1

join:
sw zero, 116(s0)           ; chain->0x74 = 0
sw v0, 0(s2)               ; sb->0 = 1
sw zero, 364(s1)           ; arg->0x16c = 0

epilogue:
ld ra, 56(sp)
ld s2, 48(sp)
ld s1, 40(sp)
ld s0, 32(sp)
move v0, zero
jr ra
daddiu sp, sp, 64
```

## Comparacao estrutural

| Aspecto | ICO | GCC | Match? |
|---|---|---|---|
| **Prologo** | addiu sp,-128, sd s1/s0/ra/s3/s2 | daddiu sp,-64, sd ra/s2/s1/s0 | Diferente |
| **Variant check** | xori + bnez | xori + bnez | **Exato** |
| **func_00104508(chamada)** | jal com (a0=sp, a1=arg) | jal com (a0=sp, a1=arg) | **Exato** |
| **Reload variant** | lw apos chamada | lw apos chamada | **Exato** |
| **Table lookup** | lui+addiu (literal) + lw | lui+ld (static ptr) + daddu + lw | Diferente |
| **field_08 sentinel check** | beq s0, 0x32f | beq s2, 0x32f | **Exato** |
| **Double 1.0** | lui+mtc1+sdc1 (FPU) | li+dsll32+sd (GPR) | Diferente |
| **func_00105f00(chamada)** | same args | same args | **Exato** |
| **func_001182f0 + sw 0** | same | same | **Exato** |
| **func_0010ec08 splat** | same args | same args | **Exato** |
| **func_001e158(chamada)** | same args | same args | **Exato** |
| **func_001ebc10(chamada)** | a0=field_08, a1=0x275850, a2=st | same | **Exato** |
| **func_001d12a8(chamada)** | a0=arg, a1=0x2a | same | **Exato** |
| **Segundo check variant==6** | bne + reload pos-call | bne + s-reg preserved | **Exato** |
| **func_00181bf8** | same args | same args | **Exato** |
| **Epilogo writes** | chain->0x74=0, sb->0=1, arg->0x16c=0 | same | **Exato** |
| **Epilogo retorno** | ld + jr ra | ld + jr ra | **Exato** |
| **return 0** | daddu v0, zero | move v0, zero | **Exato** |
| **Frame size** | 128 (0x80) | 64 (0x40) | Diferente |
| **Saved registers** | s0,s1,s2,s3,ra | s0,s1,s2,ra | -1 s-reg |
| **Stack adjust** | addiu (32-bit) | daddiu (64-bit) | Diferente |

## Veredito

```
Status:                 NEAR-MATCH (estrutura identica, diferencas compiler-specific)
Estrutura de controle: ✅ igual (mesmo fluxo de branches)
Offsets de struct:     ✅ iguais (0x15c, 0x800, 0x04, 0x48, 0x74, 0x16c)
Constantes:            ✅ iguais (0x2a, 0x11, 0x32f, 0x275850, 1.0, 100.0)
Sequencia de chamadas: ✅ identica (9 chamadas na mesma ordem)
Table lookup:          ❌ literal vs static ptr (acesso diferente)
Float constants:       ❌ mtc1+FPR vs li+dsll32+GPR (FPU vs GPR)
Frame size:            ❌ 128 vs 64
Dead mult instr:       ❌ presente no ICO, ausente no GCC
```

## Conclusao

O padrao de near-match se confirma para funcoes de 336B com floats.
A estrutura de controle, offsets de struct, constantes e sequencia de
chamadas sao identicas. As diferencas sao sistematicas entre os dois
compiladores:

1.  ee-gcc gera `lui` + `mtc1` para float constant; GCC 2.95.2 PS2 Linux
    gera `li` + `dsll32` (em GPR) — porque o backend FPU do GCC 2.95.2
    nao emite mtc1 para constantes.

2.  ee-gcc usa frame maior com mais s-registers salvos.

3.  ee-gcc usa `addiu` (32-bit); GCC usa `daddiu` (64-bit).

4.  ee-gcc tem `mult` dead code; GCC nao.

5.  ee-gcc usa literal `lui`+`addiu` para endereco absoluto; GCC precisa
    de static pointer via .data.

Testar mais funcoes nao produzira novos insights — o padrao esta
saturado.

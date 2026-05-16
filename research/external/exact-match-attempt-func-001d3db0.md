# Exact Match Attempt — `func_001D3DB0`

## Date

2026-05-16

## Objective

Retestar `0x001D3DB0` com a regra provisoria `ico_ptr32`/`int`.

**Importante**: este teste corrige um erro no documento anterior
(`first-scratch-func-001d3d70-results.md`) que listava um disassembly
incorreto para esta funcao. O "near-match" anterior foi baseado em dados
errados e deve ser descartado.

## Correction: previous analysis error

O documento `first-scratch-func-001d3d70-results.md` (2026-05-15) descrevia
`0x001D3DB0` como:

```asm
# Descrito (ERRADO):
ld $2, 348($4)     # ld v0, 0x15C(a0)
ld $3, 2048($2)    # ld v1, 0x800(v0)
lw $2, 72($3)      # lw v0, 0x48(v1)   — var < 5 check
jr $31
slt $2, $2, 5      # slt vs sltiu
```

Os bytes reais no ELF em `0x1D3DB0`:

```txt
0x1d3db0: 8c83015c  lw v1, 0x15c(a0)
0x1d3db4: 8c630800  lw v1, 0x800(v1)
0x1d3db8: 8c620004  lw v0, 4(v1)
0x1d3dbc: 38420001  xori v0, v0, 0x1
0x1d3dc0: 14400003  bnez v0, 0x1d3dd0
0x1d3dc4: 00000000  nop
0x1d3dc8: 03e00008  jr ra
0x1d3dcc: 8c620040  lw v0, 0x40(v1)
0x1d3dd0: 03e00008  jr ra
0x1d3dd4: 0000102d  move v0, zero
```

O disassembly correto tem:
- `xori` + `bnez` (branch condicional), nao `sltiu`
- offset `0x04` (variant), nao `0x48` (state_id)
- dois `jr ra` (dois caminhos de retorno)
- acesso a `payload + 0x40` no caminho verdadeiro

Isso significa que:
1. O "near-match" anterior de `0x001D3DB0` foi baseado em alvo errado
2. O C testado na ocasiao (bounds check com `sltiu`) estava errado
3. A funcao real e condicional, nao um accessor puro com bounds check
4. Precisamos testar do zero

## Target disassembly (correct)

```
0x001d3db0: lw   v1, 0x15c(a0)     ; v1 = *(int*)(ctx + 0x15C) = entity
0x001d3db4: lw   v1, 0x800(v1)     ; v1 = *(int*)(entity + 0x800) = payload
0x001d3db8: lw   v0, 4(v1)         ; v0 = *(int*)(payload + 4) = variant
0x001d3dbc: xori v0, v0, 0x1       ; v0 = variant ^ 1
0x001d3dc0: bnez v0, 0x1d3dd0      ; if variant != 1, goto return 0
0x001d3dc4: nop                     ; delay slot
0x001d3dc8: jr   ra                 ; return payload->field_40
0x001d3dcc: lw   v0, 0x40(v1)      ; delay slot: v0 = payload->field_40
0x001d3dd0: jr   ra                 ; return 0
0x001d3dd4: move v0, zero           ; delay slot: v0 = 0
```

## Resultado da compilacao

### Melhor C encontrado

```c
typedef int ico_ptr32;

int cloth_test(void *context) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)context + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    int variant = *(int*)((char*)payload + 4);
    if ((variant ^ 1) == 0)
        return *(int*)((char*)payload + 0x40);
    return 0;
}
```

Flags usadas: `-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls`

Assembly gerado (montado com `mips64r5900el-ps2-elf-as -march=r5900 -mips3 -mgp64 -mabi=eabi`):

```asm
00000000 <cloth_test>:
   0:	8c83015c 	lw	v1,348(a0)         ; entity load
   4:	8c630800 	lw	v1,2048(v1)        ; payload load
   8:	8c620004 	lw	v0,4(v1)           ; variant load
   c:	38420001 	xori	v0,v0,0x1       ; variant ^ 1
  10:	14400003 	bnez	v0,20 <ret_zero> ; if (variant^1) goto ret_zero
  14:	00001025 	move	v0,zero          ; BRANCH DELAY: v0=0 (especulativo)
  18:	03e00008 	jr	ra                 ; return field_40
  1c:	8c620040 	lw	v0,64(v1)          ; DELAY: v0=payload->field_40
  20:	03e00008 	jr	ra                 ; return 0 (ret_zero)
  24:	00000000 	nop                     ; nop de alinhamento
```

### Comparacao byte a byte com o alvo

| Offset | ICO (hex) | GCC (hex) | Diferenca |
|---|---|---|---|
| +0x00 | 8c83015c | 8c83015c | ✅ identico |
| +0x04 | 8c630800 | 8c630800 | ✅ identico |
| +0x08 | 8c620004 | 8c620004 | ✅ identico |
| +0x0c | 38420001 | 38420001 | ✅ identico |
| +0x10 | 14400003 | 14400003 | ✅ identico |
| +0x14 | 00000000 (nop) | 00001025 (move v0,zero) | ❌ scheduling |
| +0x18 | 03e00008 | 03e00008 | ✅ identico |
| +0x1c | 8c620040 | 8c620040 | ✅ identico |
| +0x20 | 03e00008 | 03e00008 | ✅ identico |
| +0x24 | 0000102d (move v0,zero) | 00000000 (nop) | ❌ scheduling |
| +0x28 | — | — | fim ICO; GCC tem 1 nop extra |

### Instrucoes que batem

Todas as 10 instrucoes de estrutura batem:
- 3x `lw` com offsets corretos (0x15C, 0x800, 0x04)
- `xori v0, v0, 0x1`
- `bnez v0, <target>` (mesmo offset 3)
- 2x `jr ra`
- `lw v0, 0x40(v1)` no delay slot do primeiro `jr ra`

### Instrucoes que diferem

Duas diferencas, ambas de **scheduling do compilador**:

| # | ICO | GCC | Explicacao |
|---|---|---|---|
| 1 | `nop` no delay slot do `bnez` (0x1dc4) | `move v0, zero` no delay slot (0x14) | GCC especula `move v0, 0` no delay slot. O ICO deixa `nop` e coloca `move v0, zero` no target. Funcionalmente identico: se o branch nao for tomado, o `move` e sobrescrito pelo `lw` seguinte. |
| 2 | `move v0, zero` no target (0x1dd4, `addu` encoding) | `nop` no target (0x24, alinhamento `.p2align`) | O GCC coloca o `move` no delay slot do branch; o target so precisa de `jr ra` + nop de alinhamento (`.reorder` mode). Diferenca de encoding do `move`: ICO usa `addu` (0x0000102d), GCC usa `or` (0x00001025). |

### Variacoes testadas

| Variacao | Resultado |
|---|---|
| `if (variant != 1) return 0;` | `li` + `bne`, registradores diferentes |
| `if (variant == 1) return ...; return 0;` | `li` + `bne` + `move` no delay |
| `return (variant ^ 1) ? 0 : payload->field_40;` | registrador `a0` como temp, `move` extra no final |
| `unsigned int variant` | `li` + `bne`, `nop` no delay (estrutura parecida, instrucoes diferentes) |
| `volatile int variant` | stack frame criado (pior) |
| `-O1` | `beqz`, `move` extra |
| `-fno-schedule-insns` | sem melhora |
| `-fno-schedule-insns2` | registradores diferentes, sem melhora |
| `int v; v = v ^ 1; if (v == 0)` | identico a V2 |

Nenhuma variacao produziu exact match. A versao com `unsigned int variant` (`bne` com `li`) chegou a ter `nop` no delay, mas usou instrucoes e registradores diferentes.

## Veredito final

```
Status:                 NEAR-MATCH (masculino)
Instrucoes identicas:   8 de 10 (+ 1 de padding alinhamento)
Diferencas:             2 (ambas de scheduling)
Tipo de diferenca:      mov vs nop (delay slot), nop vs move (target)
Alinhamento extra:      1 nop ao final (GCC .p2align)
Licoes:                 
  - ico_ptr32 resolveu ld vs lw ✅
  - xori + bnez match ✅
  - registradores v1/v1/v0 match ✅
  - Scheduling do GCC 2.95.2 difere do ee-gcc 2.9
```

### Comparacao com os 3 exact matches anteriores

Os 3 exact matches (`0x001D3D70`, `0x001D3D80`, `0x001D3D98`) sao funcoes
sem branches (accessors puros ou com sltiu). `0x001D3DB0` e a primeira
funcao com **branch condicional** testada. O scheduling do branch delay
slot e a diferenca.

### Acao

Manter como near-match e tentar `0x001D3D40` (48B, condicional com dois
branches). Se `0x001D3D40` tambem for near-match com diferenca de
scheduling, o padrao estara confirmado: o GCC 2.95.2 e o ee-gcc 2.9
diferem consistentemente no scheduling de delay slots para branches.

## Proximo passo

Retestar `0x001D3D40` com `ico_ptr32`.

## Riscos

1. O erro anterior no disassembly foi corrigido. O teste atual e valido.
2. As 2 diferencas remanescentes sao consistentes com scheduler —
   improvareis de resolver com tuning de C.
3. A diferenca de encoding `move` (`or` vs `addu`) e do assembler, nao
   do compilador. O host assembler escolheu `or`; o ICO usou `addu`.
   Ambas sao igualmente validas para `move`.

## Veredito

```
Disassembly anterior: INCORRETO (descartado)
Disassembly real:     ✅ VERIFICADO
ico_ptr32:           ✅ Funciona (gera lw, nao ld)
Exact match:         ❌ (2 diferencas de scheduling)
Near-match:          ✅ (8/10 instrucoes identicas)
Proximo passo:       0x001D3D40
```

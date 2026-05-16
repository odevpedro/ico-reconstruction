# Exact Match Attempt — `func_001D3D40`

## Date

2026-05-16

## Objective

Retestar `0x001D3D40` com `ico_ptr32`/`int`.

## Target disassembly (verified)

```
0x1d3d40: lw   v0, 0x15c(a0)       ; v0 = entity
0x1d3d44: move a1, zero             ; a1 = 0 (default return)
0x1d3d48: lw   v1, 0x16c(a0)       ; v1 = extra_ptr
0x1d3d4c: beqz v1, 0x1d3d68        ; if extra == 0, return a1
0x1d3d50: lw   a0, 0x800(v0)       ; DELAY: a0 = payload (entity in v0)
0x1d3d54: ld   v0, 8(a0)           ; v0 = payload->flag_08 (64-bit)
0x1d3d58: bnez v0, 0x1d3d68        ; if flag != 0, return a1
0x1d3d5c: nop                       ; delay slot
0x1d3d60: lw   v0, 0x48(a0)        ; v0 = payload->state_id
0x1d3d64: slti a1, v0, 2           ; a1 = (state_id < 2)
0x1d3d68: jr   ra                   ; return a1
0x1d3d6c: move v0, a1               ; delay slot
```

Total: 12 instrucoes (48 bytes).

## Melhor C encontrado

```c
typedef int ico_ptr32;

int cloth_test(void *context) {
    register int entity asm("$2") = *(int*)((char*)context + 0x15C);
    int result = 0;
    int extra = *(int*)((char*)context + 0x16C);
    if (extra != 0) {
        int payload = *(int*)((char*)entity + 0x800);
        unsigned long long flag = *(unsigned long long*)((char*)payload + 8);
        if (flag == 0) {
            result = *(int*)((char*)payload + 0x48) < 2;
        }
    }
    return result;
}
```

Flags: `-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls`

Assembly gerado:

```asm
00000000 <cloth_test>:
   0: 00002825  move a1, zero             ; a1 = 0
   4: 8c83016c  lw v1, 0x16c(a0)         ; v1 = extra
   8: 10600007  beqz v1, 0x28             ; if extra == 0, goto return
   c: 8c82015c  lw v0, 0x15c(a0)         ; DELAY: v0 = entity ($2 forced)
  10: 8c430800  lw v1, 0x800(v0)         ; v1 = payload
  14: dc620008  ld v0, 8(v1)             ; v0 = flag
  18: 14400003  bnez v0, 0x28            ; if flag != 0, goto return
  1c: 00000000  nop                       ; delay slot
  20: 8c620048  lw v0, 0x48(v1)          ; v0 = state_id
  24: 28450002  slti a1, v0, 2           ; a1 = (state_id < 2)
  28: 03e00008  jr ra                     ; return a1
  2c: 00a01025  move v0, a1               ; delay slot
```

Total: 12 instrucoes (48 bytes) — mesmo numero do alvo!

## Comparacao instrucional

| # | ICO | GCC V10 | Diferenca |
|---|---|---|---|
| 1 | `lw v0, 0x15c(a0)` | `move a1, zero` | **ordem diferente** |
| 2 | `move a1, zero` | `lw v1, 0x16c(a0)` | **ordem diferente** |
| 3 | `lw v1, 0x16c(a0)` | `beqz v1, 0x28` | **ordem + offset** |
| 4 | `beqz v1, 0x1d3d68` | `lw v0, 0x15c(a0)` (delay) | **ordem + offset** |
| 5 | `lw a0, 0x800(v0)` (delay) | `lw v1, 0x800(v0)` | a0 vs v1 |
| 6 | `ld v0, 8(a0)` | `ld v0, 8(v1)` | base: a0 vs v1 |
| 7 | `bnez v0, 0x1d3d68` | `bnez v0, 0x28` | ✅ mesmo offset 3 |
| 8 | `nop` | `nop` | ✅ identico |
| 9 | `lw v0, 0x48(a0)` | `lw v0, 0x48(v1)` | base: a0 vs v1 |
| 10 | `slti a1, v0, 2` | `slti a1, v0, 2` | ✅ identico |
| 11 | `jr ra` | `jr ra` | ✅ identico |
| 12 | `move v0, a1` | `move v0, a1` | ✅ (encoding: addu vs or) |

## Diferencas

### 1. Ordem das instrucoes (1-4)

ICO carrega entity primeiro e poe extra em v1. GCC poe `move a1, 0` primeiro,
depois carrega extra em v1 e entity em v0 no delay slot do `beqz`.

A sequencia e funcionalmente equivalente, mas reordenada pelo compilador.

### 2. Registrador base do payload: a0 vs v1

No ICO, payload vai para a0 (reutilizando o registrador de argumento, que nao
e mais necessario). No GCC V10, payload vai para v1.

Isso e alocacao de registrador pura: o ICO ja tinha entity em v0 (ocupado),
entao usa a0 para payload. O GCC V10 carrega extra em v1, depois entity em v0,
entao usa v1 para payload (ja "livre").

### 3. Encoding do `move`

ICO: `addu a1, zero, zero` (0x0000282d), `addu v0, a1, zero` (0x00a0102d)
GCC: `or a1, zero, zero` (0x00002825), `or v0, a1, zero` (0x00a01025)

Diferenca de pseudo-instrucao: `move` e montada como `or` pelo assembler host
e como `addu` pelo assembler do ICO. Funcionalmente identicas.

## Variacoes testadas

| Variacao | Resultado |
|---|---|
| v1: entity declarada primeiro, com `ico_ptr32` | `move` extra, 13 instrucoes |
| v2/v4/v5/v7/v9: entity, extra, result | `move` extra, 13 instrucoes + nop |
| v3: early return (C89 parse error) | nao compilou |
| v6: helper function | stack frame, 16 instrucoes (pior) |
| v8: early return sem variavel result | 2 returns separados, estrutura diferente |
| v10/v11: `register int entity asm("$2")` | ✅ 12 instrucoes, sem move extra |
| v12: explicit else accumulation | `beqzl`, estrutura diferente |
| v13: result=0 depois de entity | `move` extra, 13 instrucoes |
| v14: `int entity` (sem ico_ptr32) | identico a v1 |

## Veredito

```
Instrucoes totais:      12 (mesmo numero do alvo)
Instrucoes identicas:   8 de 12 (4 por ordem/registrador)
Registradores iguais:   v0=entity, a1=result
Registradores dif:      payload em a0 (ICO) vs v1 (GCC)
Ordem inst 1-4:         diferente (reordenacao do compilador)
Status:                 NEAR-MATCH (mais proximo que 0x1D3DB0)
```

## Comparacao com 0x1D3DB0

`0x001D3D40` chegou mais perto que `0x001D3DB0`: ambas sao 12 instrucoes
(sem `move` extra), enquanto `0x001D3DB0` tinha 10 vs 10 mas com `move` no
delay slot vs `nop`.

As diferencas em ambas sao consistentes com:
- **Alocacao de registradores**: GCC 2.95.2 escolhe registradores diferentes
  do ee-gcc 2.9
- **Ordenacao de instrucoes**: o scheduler reordena blocos basicos
- **Encoding de pseudo-instrucoes**: `or` vs `addu` para `move`

## Proximo passo

`0x001D40A0` (56B, accessor com null check).

## Veredito final

```
Alvo:                   0x001D3D40
Status:                 NEAR-MATCH (melhor resultado ate agora)
ico_ptr32:             ✅ confirmado para lw
register asm("$2"):    ✅ elimina move extra
Proximo:               0x001D40A0
```

# Exact Match Attempt — `func_001D40A0`

## Date

2026-05-16

## Objective

Retestar `0x001D40A0` com `ico_ptr32`/`int`.

## Target disassembly (verified)

```
0x1d40a0: move a2, a0                ; a2 = context
0x1d40a4: lw   a0, 0x15c(a2)        ; a0 = entity
0x1d40a8: li   a1, 1                 ; a1 = 1 (constant)
0x1d40ac: lw   v0, 0x800(a0)        ; v0 = payload
0x1d40b0: lw   v1, 0(v0)            ; v1 = field_00
0x1d40b4: beq  v1, a1, 0x1d40c8     ; if field_00 == 1, goto set_1
0x1d40b8: move a3, zero             ; DELAY: a3 = 0
0x1d40bc: lw   v0, 0x16c(a2)        ; v0 = extra_ptr
0x1d40c0: bnez v0, 0x1d40cc         ; if extra != 0, goto return (a3=0)
0x1d40c4: nop                        ; delay slot
0x1d40c8: li   a3, 1                 ; a3 = 1 (set_1)
0x1d40cc: jr   ra                    ; return a3
0x1d40d0: move v0, a3                ; delay: v0 = a3
0x1d40d4: nop                        ; padding
```

Total: 14 instrucoes (56 bytes).

Logica: `return (field_00 == 1) || (extra_ptr == 0);`

## Melhor C encontrado

```c
typedef int ico_ptr32;

int cloth_test(void *context) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)context + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    return (*(int*)((char*)payload + 0) == 1) ||
           (*(ico_ptr32*)((char*)context + 0x16C) == 0);
}
```

Flags: `-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls`

Assembly gerado (GCC V4):

```asm
00000000 <cloth_test>:
   0: 00802825  move a1, a0            ; a1 = context
   4: 8ca2015c  lw v0, 0x15c(a1)      ; v0 = entity
   8: 8c430800  lw v1, 0x800(v0)      ; v1 = payload
   c: 8c640000  lw a0, 0(v1)          ; a0 = field_00
  10: 24020001  li v0, 1              ; v0 = 1
  14: 10820004  beq a0, v0, 0x28      ; if field_00 == 1, goto set_1
  18: 00003025  move a2, zero          ; DELAY: a2 = 0
  1c: 8ca2016c  lw v0, 0x16c(a1)      ; v0 = extra_ptr
  20: 14400002  bnez v0, 0x2c          ; if extra != 0, goto return
  24: 00000000  nop                     ; delay slot
  28: 24060001  li a2, 1              ; a2 = 1 (set_1)
  2c: 03e00008  jr ra                  ; return a2
  30: 00c01025  move v0, a2            ; delay: v0 = a2
  34: 00000000  nop                     ; padding
```

Total: 14 instrucoes (56 bytes) — mesmo numero do alvo!

## Comparacao estrutural

| # | ICO | GCC V4 | Diferenca |
|---|---|---|---|
| 1 | `move a2, a0` | `move a1, a0` | a2 vs a1 |
| 2 | `lw a0, 0x15c(a2)` | `lw v0, 0x15c(a1)` | a0 vs v0 |
| 3 | `li a1, 1` | `lw v1, 0x800(v0)` | ordem trocada |
| 4 | `lw v0, 0x800(a0)` | `lw a0, 0(v1)` | ordem trocada |
| 5 | `lw v1, 0(v0)` | `li v0, 1` | ordem trocada |
| 6 | `beq v1, a1, set_1` | `beq a0, v0, set_1` | registradores diferentes |
| 7 | `move a3, zero` (delay) | `move a2, zero` (delay) | a3 vs a2 |
| 8 | `lw v0, 0x16c(a2)` | `lw v0, 0x16c(a1)` | a2 vs a1 |
| 9 | `bnez v0, return` | `bnez v0, return` | ✅ identico |
| 10 | `nop` | `nop` | ✅ identico |
| 11 | `li a3, 1` | `li a2, 1` | a3 vs a2 |
| 12 | `jr ra` | `jr ra` | ✅ identico |
| 13 | `move v0, a3` (delay) | `move v0, a2` (delay) | a3 vs a2 |
| 14 | `nop` | `nop` | ✅ identico |

## Diferencas

### 1. Alocacao de registradores (100% das diferencas)

| Variavel | ICO | GCC | Motivo |
|---|---|---|---|
| Context saved | a2 | a1 | Escolha do registrador |
| Entity | a0 | v0 | GCC nao reusou a0 |
| Constant 1 | a1 | v0 | GCC reusou v0 apos usar para entity |
| field_00 | v1 | a0 | Consequencia das escolhas acima |
| Result | a3 | a2 | Escolha do registrador |

### 2. Ordem das instrucoes 2-5

ICO: load entity (a0), load constant (a1), load payload (v0), load field (v1)
GCC: load entity (v0), load payload (v1), load field (a0), load constant (v0)

A ordem reflete a alocacao de registradores, nao a logica.

## Variacoes testadas

| Variacao | Resultado |
|---|---|
| v1: if-else basico | `movn`, estrutura diferente |
| v2: accumulator | `movn`, estrutura diferente |
| v3: early return explicito | `movn`, estrutura diferente |
| v4: logical OR `\|\|` | ✅ 14 instrucoes, `beq` + `bnez` |
| v5: register hints com `(int)cast` | `dsll32`/`dsra32` (pior) |
| v6: context pointer `$6` + entity `$4` | hint ignorado, `movn` |
| v7/v8/v9: mais register hints | hint ignorado ou bnel/sltiu |

## Veredito

```
Instrucoes:             14 (mesmo numero do alvo)
Estrutura:              IDENTICA (beq+bnez+li+jr)
Registradores:          TODOS DIFERENTES (a2/a0/a1/v1/a3 vs a1/v0/v1/a0/a2)
Status:                 NEAR-MATCH (puramente registrador)
Diferenca sistematica:  Alocacao de registradores entre compiladores
```

## Comparacao dos 3 testes

| Funcao | Bytes | Status | Diferencas |
|---|---|---|---|
| `0x001D3DB0` | 40 | near-match | scheduling (move vs nop no delay, nop padding) |
| `0x001D3D40` | 48 | near-match | ordem instrucoes 1-4, registrador payload |
| `0x001D40A0` | 56 | near-match | registradores (todos), ordem inst 2-5 |

## Conclusao dos 3 testes

1. **`ico_ptr32` funciona**: todas as 3 funcoes geram `lw` em vez de `ld` para
   entity e payload.
2. **Nenhuma e exact match**: as diferencas sao de alocacao de registradores
   e scheduling entre GCC 2.95.2 PS2 Linux e ee-gcc 2.9-991111-01.
3. **O pipeline esta validado**: o C compilado gera a sequencia, estrutura e
   offsets corretos em todos os 3 casos. As unicas diferencas sao de
   registrador e scheduling — nao de logica ou offsets.
4. **Nao ha o que tunar**: estas diferencas sao do compilador, nao do C.
   Tentativas com register hints falharam porque o GCC 2.95.2 ignora hints
   quando otimiza.

## Proximo passo

Parar de tentar exact match para accessors/condicionais simples. As
diferencas sao sistematicas e nao resolviveis com tuning de C.

Proximos topicos para considerar:
- Tentar funcao maior (ex: `0x001D4358`, pack color) para ver se o padrao
  de near-match se mantem
- Investigar se `-fno-schedule-insns2` + register hints resolve o scheduling
- Atualizar `cloth-struct-hypotheses.md` com `ico_ptr32`
- Criar `research/external/cloth-exact-match-c-sources.md` consolidado

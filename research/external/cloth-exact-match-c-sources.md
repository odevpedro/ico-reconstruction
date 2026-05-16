# Cloth Exact Match C Sources

## Date

2026-05-16

## Purpose

Consolidar os C sources testados para as funcoes do cluster cloth,
com assembly esperado, variacoes tentadas e veredito de cada uma.

## Compiler flags

Todas as compilacoes usaram:

```
-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls
```

Montagem com:

```
mips64r5900el-ps2-elf-as -march=r5900 -mips3 -mgp64 -mabi=eabi
```

## Tipo provisorio

```c
typedef int ico_ptr32;
```

## Exact matches (3)

Estas funcoes produziram instruction-exact match no primeiro teste
sem necessidade de ajuste de registrador ou scheduling.

### func_001D3D70 (16B)

```c
int cloth_get_variant(void *context) {
    int entity = *(int*)((char*)context + 0x15C);
    int payload = *(int*)((char*)entity + 0x800);
    return *(int*)((char*)payload + 4);
}
```

```asm
lw v0, 0x15c(a0)
lw v1, 0x800(v0)
jr ra
lw v0, 4(v1)
```

Veredito: **exact match**.

### func_001D3D80 (24B)

```c
int cloth_payload_field0_is_zero(void *context) {
    int entity = *(int*)((char*)context + 0x15C);
    int payload = *(int*)((char*)entity + 0x800);
    return *(unsigned int*)((char*)payload + 0) < 1;
}
```

```asm
lw v0, 0x15c(a0)
lw v1, 0x800(v0)
lw v0, 0(v1)
jr ra
sltiu v0, v0, 1
nop
```

Veredito: **exact match**.

### func_001D3D98 (24B)

```c
int cloth_payload_state_is_two(void *context) {
    int entity = *(int*)((char*)context + 0x15C);
    int payload = *(int*)((char*)entity + 0x800);
    return (*(unsigned int*)((char*)payload + 0x48) ^ 2) < 1;
}
```

```asm
lw v0, 0x15c(a0)
lw v1, 0x800(v0)
lw v0, 0x48(v1)
xori v0, v0, 2
jr ra
sltiu v0, v0, 1
```

Veredito: **exact match**.

## Near-matches (3)

Estas funcoes produziram near-match. As diferencas sao de scheduling
e alocacao de registradores entre GCC 2.95.2 PS2 Linux e ee-gcc 2.9.

### func_001D3DB0 (40B)

Logica: se variant == 1, retorna payload->field_40; senao retorna 0.

```c
int cloth_test(void *context) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)context + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    int variant = *(int*)((char*)payload + 4);
    if ((variant ^ 1) == 0)
        return *(int*)((char*)payload + 0x40);
    return 0;
}
```

```asm
lw v1, 0x15c(a0)
lw v1, 0x800(v1)
lw v0, 4(v1)
xori v0, v0, 1
bnez v0, <ret_zero>
move v0, zero          ; GCC vs nop (ICO)
jr ra
lw v0, 0x40(v1)
jr ra
move v0, zero
nop                    ; padding extra (GCC)
```

Diferencas:
- Branch delay: `move v0, zero` (GCC) vs `nop` (ICO) — scheduling
- Target: `nop` padding (GCC) vs `move v0, zero` (ICO) — invertido
- Encoding `move`: `or` (GCC) vs `addu` (ICO)

Veredito: **near-match** (8/10 instrucoes identicas, 2 scheduler).

### func_001D3D40 (48B)

Logica: se extra_ptr != NULL e flag_08 == 0, retorna (state_id < 2);
senao retorna 0.

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

```asm
move a1, zero
lw v1, 0x16c(a0)
beqz v1, <ret>
lw v0, 0x15c(a0)       ; DELAY: entity
lw v1, 0x800(v0)       ; payload
ld v0, 8(v1)           ; flag_08 (64-bit)
bnez v0, <ret>
nop
lw v0, 0x48(v1)        ; state_id
slti a1, v0, 2
jr ra
move v0, a1
```

Diferencas do alvo:
- Ordem inst 1-4 invertida: GCC faz `move a1,0` + `lw extra` antes de
  carregar entity; ICO carrega entity primeiro
- Payload em v1 (GCC) vs a0 (ICO) — registro diferente
- Encoding `move`: `or` (GCC) vs `addu` (ICO)

Veredito: **near-match** (12 instrucoes, mesmo numero do alvo).

### func_001D40A0 (56B)

Logica: retorna 1 se field_00 == 1 OU extra_ptr == NULL; senao 0.

```c
int cloth_test(void *context) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)context + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    return (*(int*)((char*)payload + 0) == 1) ||
           (*(ico_ptr32*)((char*)context + 0x16C) == 0);
}
```

```asm
move a1, a0             ; salva contexto (ICO: move a2, a0)
lw v0, 0x15c(a1)       ; entity (ICO: lw a0, 0x15c(a2))
lw v1, 0x800(v0)       ; payload (ICO: lw v0, 0x800(a0))
lw a0, 0(v1)           ; field_00 (ICO: lw v1, 0(v0))
li v0, 1               ; 1 (ICO: li a1, 1)
beq a0, v0, <set_1>    ; if field_00 == 1 (ICO: beq v1, a1)
move a2, zero           ; DELAY: a2 = 0 (ICO: move a3, zero)
lw v0, 0x16c(a1)       ; extra (ICO: lw v0, 0x16c(a2))
bnez v0, <ret>          ; if extra != 0
nop
li a2, 1               ; a2 = 1 (ICO: li a3, 1)
jr ra
move v0, a2             ; DELAY (ICO: move v0, a3)
nop                     ; padding
```

Diferencas do alvo:
- Todos os registradores diferentes (a1 vs a2, v0 vs a0, v1 vs v0, etc.)
- Ordem load entity/payload/field/const diferente
- Estrutura e tipos de instrucao identicos

Veredito: **near-match** (14 instrucoes, estrutura identica).

## Funcoes bloqueadas

| Funcao | Motivo | Categoria |
|---|---|---|
| `0x001D27A8` | Runtime-dependent (a1 = stack) | blocked |
| `0x001D37C8` | Jump table `.word` vs `.dword` | blocked |
| `0x001D4348` | Thunk com GP-relative | blocked |
| `0x001D40D8`, `3DD8`, `4170` | Dependem de state resolver | blocked |
| `0x001D29B8`+ | Funcoes grandes (>500B) | needs context |

## Proximos candidatos

| Funcao | Bytes | Motivo |
|---|---|---|
| `0x001D4358` | 160 | pack color, sem branches, chamada direta |
| `0x001D3BF0` | 336 | primeira com float e chamadas |

## Padrao observado

```
ico_ptr32 (int)         → lw ✅
unsigned int             → lwu ❌ (nao usado pelo ICO)
void*                    → ld ❌ (64-bit, nao usado pelo ICO)
uint64_t / long long     → ld ✅ (para flag_08, campo real de 64 bits)
int para campos 32-bit   → lw ✅
```

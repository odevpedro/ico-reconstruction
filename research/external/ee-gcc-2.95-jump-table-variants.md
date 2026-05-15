# EE GCC 2.95 Jump Table Variants

## Date

2026-05-15

## Resumo executivo

Testes focados no MT3 (jump table) variando tipo C, ABI e flag `-mgp64`
para tentar reproduzir o padrão de 4 bytes usado pelo ICO.

## Variações testadas

| Variação | dispatch_int | dispatch_uint | dispatch_long |
|---|---|---|---|
| `-mgp64` | `.dword` 8-byte, zero-extend dsll+dsrl | idem | `.dword`, dsll×3 direto |
| `-mgp64 -mabi=eabi` | `.dword` 8-byte, zero-extend | idem | `.dword`, address-rel |
| **sem** `-mgp64` | `.dword` 8-byte, zero-extend | idem | `.dword`, dsll×3 |

## Resultado principal

**NENHUMA variação produziu tabela de 4 bytes.** O PS2 Linux GCC 2.95.2
sempre gera `.dword` (8-byte entries) para jump tables, independentemente
de:

- Tipo C: `int` / `unsigned int` / `unsigned long`
- Flag: `-mgp64` presente ou ausente
- ABI: default vs `-mabi=eabi`

## Detalhamento

### dispatch_int com `-mgp64`

```asm
dsll $2, $4, 32     ; zero-extend int to 64-bit
dsrl $2, $2, 32     ;
dla $4, .L10        ; table address
dsll $2, $2, 3      ; ×8 (pointer size)
daddu $2, $2, $4    ; table[index]
ld $3, 0($2)        ; load 64-bit pointer
j $3                ; dispatch
```

### dispatch_long com `-mgp64 -mabi=eabi`

```asm
dsll $2, $4, 3      ; ×8 diretamente (já é 64-bit)
ld $3, .L20($2)     ; load via PC-relative
j $3                ; dispatch
```

### ICO dispatcher (real)

```asm
sll $v1, $v1, 2     ; ×4 (32-bit entries)
addiu $v0, ...      ; table address
addu $v1, $v1, $v0  ;
lw $a0, 0($v1)      ; load 32-bit target
jr $a0              ; dispatch
```

## Interpretação

O PS2 Linux GCC 2.95.2 SEMPRE gera tabelas de 8 bytes para jump tables.
Isto ocorre porque:

1. O target `mipsEEel-linux` trata ponteiros como 64-bit (mesmo sem `-mgp64`)
2. O compilador nunca otimiza para armazenar ponteiros como 32-bit
3. Não há flag conhecida que force `.word` em vez de `.dword`

O ICO usa `.word` (4 bytes) porque o `ee-gcc 2.9-991111-01` tem uma
otimização que armazena ponteiros de função como 32-bit (possivelmente
porque todos os targets estão nos 2 GB inferiores do espaço de endereço).

Esta é a diferença de codegen mais clara entre os dois compiladores. Não
afeta a correção funcional, mas significa que scratches de funções com
jump table não terão matching exato.

## Próximo passo

Escolher função sem jump table do cluster cloth para primeiro scratch real
— a `func_001D3D70` (16 bytes) é a candidata ideal.

# First Scratch Results — `func_001D3D70`

## Date

2026-05-15

## Resumo executivo

Primeiro teste real de matching C contra o ICO usando GCC 2.95.2 PS2 Linux
com flags ICP. O resultado é **near-match**: a sequência de instruções,
offsets e estrutura são idênticos, mas o GCC gera `ld` (64-bit) para
carga de ponteiros enquanto o ICO usa `lw` (32-bit).

## C testado

```c
// Tentativa 1 — ponteiros explícitos (BEST)
int cloth_get_variant(void *context) {
    void *entity = *(void**)((char*)context + 0x15C);
    int *payload = *(int**)((char*)entity + 0x800);
    return payload[1];  // field_04 = int at offset 4
}
```

## Flags usadas

```
ICP: -O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls
```

ICP+EABI produziu **código idêntico** ao ICP.

## Assembly esperado (ICO)

```asm
lw $v0, 0x15C($a0)    ; load entity pointer
lw $v1, 0x800($v0)    ; load payload pointer
jr $ra                 ; return
lw $v0, 0x4($v1)      ; delay slot: load variant field
```

## Assembly gerado (GCC ICP)

```asm
ld $2, 348($4)         ; ld v0, 0x15C(a0)
#nop
ld $3, 2048($2)        ; ld v1, 0x800(v0)
jr $31
lw $2, 4($3)           ; lw v0, 4(v1)
```

## Diferenças

| Aspecto | ICO | GCC ICP | Impacto |
|---|---|---|---|
| Pointer load | `lw` (32-bit) | `ld` (64-bit) | **Principal diferença** |
| Hazard nop | Nenhum | `#nop` entre loads | Cosmético |
| Delay slot | Load preenchido | Load preenchido | ✅ Match |
| Offset 0x15C | ✅ | ✅ (348 = 0x15C) | ✅ Match |
| Offset 0x800 | ✅ | ✅ (2048 = 0x800) | ✅ Match |
| Offset 0x04 | ✅ | ✅ (4) | ✅ Match |
| Leaf function | ✅ (0 stack) | ✅ (0 stack) | ✅ Match |
| Return | `jr $ra` | `jr $31` | ✅ $31 = $ra |

## Hipótese de por que difere

O GCC 2.95.2 PS2 Linux com `-mgp64` trata ponteiros como 64-bit e gera
`ld` para carregá-los da memória. O ICO usa `lw` (32-bit) porque:

1. **Espaço de endereço inferior**: todos os ponteiros no ICO cabem em
   32 bits (limite de 2 GB). O ee-gcc 2.9-991111-01 otimiza usando `lw`.
2. **ABI EABI**: o `-mabi=eabi` pode tratar ponteiros como 32-bit para
   loads, mesmo com `-mgp64` ativo para aritmética.
3. **Otimização Sony**: o patch R5900 comercial pode incluir uma
   otimização de "32-bit pointer load when safe" que o PS2 Linux patch
   não tem.

Esta é a segunda diferença de codegen documentada (após jump table entries
de 8 vs 4 bytes). Ambas apontam para o mesmo padrão: o ee-gcc 2.9 trata
ponteiros como 32 bits em contextos onde cabem, enquanto o PS2 Linux GCC
trata consistentemente como 64 bits.

## Tentativa 2 — struct explícita

Falhou. O cálculo de `sizeof` para `EntityPayload` gerou offset errado
(348 = 0x15C em vez de 4 para o campo `variant`). Abordagem de struct
é arriscada sem definir exatamente os tipos de cada campo.

## Veredito

```txt
Sequência de instruções: MATCH (ld vs lw é diferença de largura, não de sequência)
Offsets: MATCH (0x15C, 0x800, 0x04)
Delay slot: MATCH (load no slot do jr)
Stack: MATCH (0 bytes)
Pipeline validado: SIM
Classificação: NEAR-MATCH
```

Diferença única: `ld` vs `lw` para carga de ponteiros.

Esta diferença é consistente com o padrão observado nas jump tables
(ICO usa 32 bits, GCC usa 64 bits). O ee-gcc 2.9-991111-01 aparentemente
tem uma propensão a usar valores de 32 bits quando possível, enquanto o
PS2 Linux GCC mantém 64 bits.

## Próximo scratch recomendado

`func_001D3D80` (24 bytes) — mesmo padrão de acesso, com `sltiu`
adicional. Verificar se o `ld` vs `lw` se repete.

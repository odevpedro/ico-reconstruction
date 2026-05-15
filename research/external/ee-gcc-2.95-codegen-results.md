# EE GCC 2.95 Codegen Results

## Date

2026-05-15

## Resumo executivo

Todos os 7 micro-targets foram compilados com 8 variações de flags usando
o GCC 2.95.2 PS2 Linux no container Docker i386. Os resultados foram
comparados com padrões reais do binário do ICO.

## Resultados por variação

| ID | Funcionou? | Notas |
|---|---|---|
| **ICP** | ✅ | Código limpo sem PIC. 64-bit pointers (ld/sd/daddu/dsubu). |
| **ICP+eabi** | ✅ | **Idêntico ao ICP** para todas as funções. EABI não muda código para funções simples. |
| **ICP+pic** | ✅ | **Gera código inchado**: 32 bytes de stack, save/restore $gp em toda função. Diferente do ICO. |
| **ICP+o32** | ❌ | `-mabi=o32` não suporta `-mips3` |
| **ICP+64** | ❌ | `-mabi=n64` não suporta `-mips3` |
| **ICP-gp64** | ✅ | **Idêntico ao ICP** para funções int. `-mgp64` não muda código de inteiros. |
| **ICP-nor5900** | ✅ | **Idêntico ao ICP** para funções simples. `-mcpu=r5900` não muda código sem loops. |
| **default** | ✅ | o32 PIC: 32-bit pointers (lw/sw/subu), `.cpload`, stack pesado. |

## Comparação micro-target a micro-target

### MT1 — store/load simples

| Variação | Código | Stack | Match ICO? |
|---|---|---|---|
| ICP/ICP+eabi | `j $31; sw $5,0($4)` | 0 | **Match funcional** (ICO: `jr $ra; sw ...`) |
| ICP+pic | `dsubu $sp,...; sd $28,...; ...; j $31; daddu $sp,...` | 32 | Diferente (ICO não tem PIC) |
| default | `j $31; sw $5,0($4)` com `.cpload` | 0 | `.cpload` não aparece no ICO |

**Veredito**: ICP sem PIC gera o mesmo padrão do ICO para stores simples.

### MT2 — Rev.043 pattern `[a1+0x30] -> [payload+0x04]`

**GCC ICP**:
```asm
lw $2,48($5)     # load [arg+0x30]  (offset 48 decimal = 0x30)
sw $2,4($4)      # store [payload+0x04]
```

**ICO** (de `cloth_payload_init_001d27a8.s`):
```asm
lw $v1, 0x30($s4)   # load de a1 (initializer_arg) + 0x30
sw $v1, 0x4($s6)    # store em payload + 0x04
```

**MATCH PERFEITO**. Mesmo padrão, mesmos offsets, mesma sequência.
Diferença apenas nos registradores (`$2` vs `$v1`, `$5` vs `$s4`).

### MT3 — Switch/jump table

**GCC ICP** (traduzido):
```asm
sltu $2, $4, 5          # bounds check
beq $2, $0, .Ldefault    # if >= 5 goto default
dsll $2, $4, 32          # zero-extend int to 64-bit
dsrl $2, $2, 32          #
dla $4, .Ltable           # table address (PIC-free)
dsll $2, $2, 3            # index * 8 (64-bit pointers)
daddu $2, $2, $4          # table[index]
ld $3, 0($2)              # load pointer
j $3                      # dispatch
```

**ICO dispatcher** (0x001d37c8):
```asm
sltiu $v0, $v1, 5        # bounds check
beqz $v0, .Lfallback      # if >= 5 goto fallback
sll $v1, $v1, 2           # index * 4 (32-bit pointers)
addiu $v0, $v0, ...       # table address
addu $v1, $v1, $v0        # table[index]
lw $a0, 0($v1)            # load word
jr $a0                    # dispatch
```

**Diferenças críticas**:

| Aspecto | GCC ICP | ICO |
|---|---|---|
| Index shift | `* 8` (dsll, 8-byte ptrs) | `* 4` (sll, 4-byte ptrs) |
| Load | `ld` (64-bit load) | `lw` (32-bit load) |
| 64-bit cleanup | `dsll+dsrl` (zero-extend) | **NÃO tem** |
| Table add | `dla` + `daddu` | `addiu` + `addu` |
| JR target | `j $3` | `jr $a0` |

**Interpretação**: O ICO trata o state_id como `unsigned int` e a jump table
como array de `unsigned int` (4 bytes). O GCC trata como `long` (8 bytes) por
causa de `-mgp64`. A diferença sugere que o ICO foi compilado com tratamento
32-bit para o state_id, possivelmente sem `-mgp64` para esta função específica,
ou com comportamento diferente do `ee-gcc 2.9`.

### MT4 — Struct pointer

**GCC ICP**:
```asm
ld $3, 40($4)     # load pointer (offset 40 = 0x28, struct has int + 8 ints)
lw $2, 0($3)      # dereference
j $31
```

**ICO** (do dispatcher `0x001d37e4`):
```asm
lw $v0, 0x15c($s2)   # load [context + 0x15c]
lw $s3, 0x800($v0)   # load [entity + 0x800]
```

**Diferença**: GCC usa `ld` (64-bit load) para ponteiro; ICO usa `lw` (32-bit load)
para offsets grandes. Ambos usam `lw` para o dado final.

### MT5 — Indirect call

**GCC ICP**:
```asm
dsubu $sp, $sp, 16
sd $31, 0($sp)
jal $31, $2        # jalr via register
move $4, $5        # delay slot: setup a0
ld $31, 0($sp)
jr $31
daddu $sp, $sp, 16
```

**ICO** (de `cb48_dispatcher_0013fc00.s`):
```asm
jalr $v0           # indirect call
daddu $a0, $s2, $zero  # delay slot: setup a0
```

**Similaridade**: Ambos usam `jalr` com setup de `a0` no delay slot. GCC
salva `$ra` na stack (porque precisa dele para retornar depois do call);
o ICO faz o mesmo em funções não-leaf.

### MT6 — Multi args

**GCC ICP** (6 args):
```asm
addu $2, $5, $6
addu $2, $2, $7
addu $2, $2, $8
jr $31
addu $2, $2, $9
```

**GCC default** (6 args, o32):
```asm
addu $2, $5, $6
addu $2, $2, $7
lw $3, 16($sp)    # 5o argumento da stack
lw $4, 20($sp)    # 6o argumento da stack
addu $2, $2, $3
jr $31
addu $2, $2, $4
```

**Diferença**: ICP (com `-mgp64`) passa 6 argumentos em registradores
(`$4-$9` = `$a0-$a5`). Default (o32 sem gp64) passa 4 primeiros em
registradores e resto na stack. O ICO usa EABI que pode passar mais
argumentos em registradores.

### MT7 — Float

**GCC ICP**:
```asm
c.lt.s $f12, $f13    # v < min
bc1t .L19            # if true, return min
c.lt.s $f0, $f12     # max < v
bc1t .L22            # if true, return max
j $31
mov.s $f0, $f12      # delay: return v
```

**Observação**: Usa `$f12` (primeiro arg float), `$f13` (segundo),
`$f14` (terceiro). Compatível com EABI float calling convention.
`-msingle-float` está ativo (não usa double).

## Conclusões por flag

### `-mabi=eabi` — EFEITO ZERO em funções simples
ICP vs ICP+EABI produziram **código idêntico**. O EABI só faz diferença
para struct returns, varargs, ou chamadas de biblioteca.

### `-fno-pic -mno-abicalls` — ESSENCIAL
ICP sem PIC vs ICP+pic: diferença DRÁSTICA. PIC adiciona 32 bytes de stack
e save/restore de `$gp` em CADA função, mesmo as triviais. O ICO **não
usa este padrão**.

### `-mgp64` — Muda ponteiros para 64-bit (efeito misto)
Afeta tamanho de jump table entries (4→8 bytes) e loads de ponteiros
(`lw`→`ld`). O ICO usa ambos (`ld`/`sd` em alguns lugares, `lw`/`sw` em
outros), sugerindo que nem todo código foi compilado com `-mgp64`
uniformemente, ou que o ee-gcc 2.9 tratava `int` como 32-bit mesmo com
`-mgp64` (diferente deste GCC).

### `-mcpu=r5900` — NENHUM EFEITO em código simples
ICP vs ICP-nor5900: idêntico para funções sem loops. O tuning R5900 só
afeta scheduling e loop optimization.

## Limitações dos testes

1. Micro-targets são **C simples** — funções reais do ICO têm loops, controle
   de fluxo complexo, e acesso a structs aninhadas que não foram testados
2. A função `dispatch` testou switch de 5 casos — a tabela real do ICO tem
   `lui+addiu` para endereço, não `dla`
3. Não foi testado struct return, alloca, ou longjmp
4. As flags `-mabi=o32` e `-mabi=n64` falharam com `-mips3` — este GCC não
   suporta essa combinação

## Veredito

```
MT1 (store/load):     ICP = MATCH funcional com ICO
MT2 (Rev.043 copy):   ICP = MATCH PERFEITO (mesmos offsets)
MT3 (jump table):     ICP = DIFERENTE (64-bit vs 32-bit entries)
MT4 (struct ptr):     ICP = CLOSE (ld vs lw para ponteiros)
MT5 (indirect call):  ICP = MATCH funcional
MT6 (multi args):     ICP = usa registradores extendidos ($a0-$a5)
MT7 (float):          ICP = MATCH funcional
```

O ICP (sem PIC, com `-mgp64 -mips3 -mcpu=r5900`) produz o código mais
próximo do ICO entre todas as variações testadas. As principais diferenças
residuais são:

1. **Tamanho de jump table entries**: GCC gera `.dword` (8-byte), ICO usa
   `.word` (4-byte) — diferença de tratamento de `int` vs `long` com `-mgp64`
2. **64-bit cleanup**: GCC insere `dsll+dsrl` para zero-extender int →

A diferença mais significativa está no MT3 (jump table): o PS2 Linux GCC
com `-mgp64` gera código de 64 bits "completo" (zero-extend, shift por 3,
ld de 8 bytes), enquanto o ICO usa código mais compacto (shift por 2, lw de
4 bytes). Isso sugere que o `ee-gcc 2.9-991111-01` trata `int` como 32-bit
para indexação de array mesmo com `-mgp64`, enquanto o PS2 Linux GCC promove
para 64-bit. Esta é a diferença de codegen mais importante documentada.

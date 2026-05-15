# EE GCC 2.95 Codegen Test Plan

## Date

2026-05-15

## Resumo executivo

Plano de testes de codegen para comparar o GCC 2.95.2 PS2 Linux (com patch
R5900) contra padrões reais do binário do ICO. O objetivo não é matching
perfeito, mas entender quais flags e configurações aproximam mais o estilo
de código gerado do observado no ICO.

O plano usa micro-targets C inspirados em padrões reais do jogo (load/store,
indirect call, jump table, struct copy) e compara o assembly gerado com
funções equivalentes do ICO.

## Objetivo

1. Verificar quais combinações de flags produzem código mais similar ao ICO
2. Identificar diferenças sistemáticas entre o PS2 Linux GCC e o `ee-gcc 2.9-991111-01`
3. Estabelecer uma linha de base para um futuro compiler package no decomp.me
4. Documentar o comportamento de ABI, prologue, e instruções do R5900 backend

## Toolchain alvo

| Componente | Localização |
|---|---|
| GCC 2.95.2 PS2 Linux | Container Docker `ps2-gcc-295` (i386) |
| Binário | `/tmp/ps2-gcc-295/ps2/bin/ee-gcc` |
| Flags de path | `-B/tmp/ps2/bin/ -B/tmp/ps2/lib/gcc-lib/mipsEEel-linux/2.95.2/` |
| Target | `mipsEEel-linux` |
| Assembly do ICO para comparação | Splat output em `/tmp/ico-cloth-full/asm/` |
| Disassembler | `mips64r5900el-ps2-elf-objdump` (ps2dev moderno) |

## Flags candidatas

### Configuração principal (ICP — ICO Candidate Profile)

Esta é a combinação mais próxima das flags do ICO:

```
-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls
```

### Variações a testar

| ID | Flags | Justificativa |
|---|---|---|
| **ICP** | `-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls` | Flags do ICO, com `-mcpu=r5900` no lugar de `-march=r5900` |
| **ICP+eabi** | ICP + `-mabi=eabi` | Testa EABI calling convention específica |
| **ICP+pic** | ICP sem `-fno-pic -mno-abicalls` | Mostra diferença com PIC ativado (default do target) |
| **ICP+o32** | ICP + `-mabi=o32` | Testa ABI o32 (default do target) |
| **ICP+64** | ICP + `-mabi=n64` | Testa N64 ABI para comparação |
| **ICP-gp64** | ICP sem `-mgp64` | Verifica se o ICO usa 32-bit GP registers |
| **ICP-nor5900** | ICP sem `-mcpu=r5900` | Mostra impacto do tuning R5900 vs genérico |
| **default** | Nenhuma flag além de `-O2` | Default do target (o32, mips2, mcpu=r5900, PIC) |

### Comando de teste (template)

```bash
docker run --rm -i ps2-gcc-295 bash << 'EOF'
B="-B/tmp/ps2/bin/ -B/tmp/ps2/lib/gcc-lib/mipsEEel-linux/2.95.2/"
F="-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls"
/tmp/ps2/bin/ee-gcc $B -S $F micro.c -o micro.s
cat micro.s
EOF
```

## Micro-targets C

### MT1 — Store/load simples

```c
// ICO pattern: escrita simples em struct via ponteiro
void store_int(int *p, int x) {
    *p = x;
}

int load_int(int *p) {
    return *p;
}

void store_short(short *p, short x) {
    *p = x;
}
```

**Padrão ICO esperado**: `sw $a1, 0($a0)` / `lw $v0, 0($a0)` / `jr $ra`

### MT2 — Cópia estruturada (inspirado no Rev.043)

```c
// ICO Rev.043 pattern: [a1+0x30] -> [payload+0x04]
struct payload {
    int field_00;
    int field_04;  // variant/mode field
    int field_08;
};

struct arg {
    int unused[12]; // 0x30 bytes to field
    int value;      // at offset 0x30
};

void copy_variant(struct payload *dst, struct arg *src) {
    dst->field_04 = src->value;
}
```

**Padrão ICO esperado** (de Rev.043):
```asm
/* 0x001d2850 */ lw $v1, 0x30($s4)   ; load [a1+0x30]
/* 0x001d2858 */ sw $v1, 0x4($s6)    ; store to [payload+0x04]
```

### MT3 — Switch/jump table pequena

```c
// ICO dispatcher pattern: 5-state jump table
int dispatch(int state, int a, int b) {
    switch (state) {
        case 0: return a + b;
        case 1: return a - b;
        case 2: return a * b;
        case 3: return a / b;
        case 4: return a % b;
        default: return 0;
    }
}
```

**Padrão ICO esperado** (do dispatcher `0x001d37c8`):
```asm
sltiu $v0, $v1, 5       ; bounds check
beqz $v0, default_label  ; if >= 5, fallback
sll $v1, $v1, 2         ; state_id * 4
addu $v1, $v1, $v0      ; + table base
lw $a0, 0($v1)          ; load jump target
jr $a0                  ; dispatch
```

### MT4 — Struct pointer/field access

```c
// ICO pattern: acesso a campos de struct via deslocamento
struct context {
    int a;
    int b;
    int c;
    int d;
};

struct entity {
    int flags;
    int data[8];
    struct context *ctx;
};

int read_context_field(struct entity *e) {
    return e->ctx->a;
}
```

**Padrão ICO esperado**: `lw $v0, offset($a0)` (load de ponteiro), depois
`lw $v0, 0($v0)` (dereferência), similar a `0x001d37e4`-`0x001d37e8`.

### MT5 — Chamada indireta (function pointer)

```c
// ICO callback pattern: indirect call via pointer
typedef int (*callback_t)(int);

int invoke_callback(callback_t cb, int arg) {
    return cb(arg);
}

int invoke_with_context(int (*cb)(void*, int), void *ctx, int arg) {
    return cb(ctx, arg);
}
```

**Padrão ICO esperado** (do callback dispatcher `0x0013fc00`):
```asm
jalr $v0              ; call via register
daddu $a0, $s2, $zero ; delay slot: setup a0
```

### MT6 — Argumentos múltiplos

```c
// ICO pattern: função com vários argumentos
// Similar a funções de callback com contexto + múltiplos params
int multi_args(void *ctx, int a, int b, int c, int d, int e) {
    return a + b + c + d + e;
}
```

**Padrão ICO esperado**: primeiros 4 args em `$a0-$a3`, args extras na stack
(`EABI convention` ou `o32`).

### MT7 — Float simples (condicional)

```c
// ICO cloth physics: comparação float
// NOTA: só compilar se o GCC aceitar sem soft-float
float clamp(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}
```

**Padrão ICO esperado**: `c.lt.s`, `bc1f`, movimentação de float.
O ICO usa `-msingle-float` e tem instruções float no cluster cloth.

## Métricas de comparação

Para cada micro-target compilado com cada variação de flags, comparar:

### 1. Prologue/epilogue

| Métrica | O que observar |
|---|---|
| Stack frame | Tamanho (`addiu $sp, $sp, -N`) |
| Saved registers | Quais registradores são salvos (`sd $ra, N($sp)`, `sd $s0, ...`) |
| Frame pointer | Usa `$fp` ou só `$sp`? |
| Epilogue | `jr $ra` antes ou depois do restore? |

### 2. Uso de registradores

| Métrica | O que observar |
|---|---|
| Argumentos | `$a0-$a3` (primeiros 4) vs stack (5+) |
| Retorno | `$v0` apenas ou `$v0-$v1` para 64-bit |
| Temp registers | `$t0-$t9` vs `$v0-$v1` para valores temporários |
| Saved registers | `$s0-$s7` — quantos e quais |
| GP register | `$gp` é referenciado? (PIC vs no-PIC) |

### 3. Delay slots

| Métrica | O que observar |
|---|---|
| Branch delay | Instrução no slot após `bne`/`beq`/`b` |
| Jump delay | Instrução no slot após `j`/`jr`/`jal`/`jalr` |
| Load delay | Instrução no slot após `lw`/`ld` |

### 4. Ordem de loads/stores

| Métrica | O que observar |
|---|---|
| Store after load | Ordem das operações de memória |
| Scheduling | Reordenação de instruções |
| Nops | Presença de `nop` para hazard resolution |

### 5. Jump table

| Métrica | O que observar |
|---|---|
| Formato | Tabela de endereços ou offsets |
| Localização | .rodata ou .text |
| Registrador base | `$v0` + deslocamento como no ICO? |

### 6. Constantes

| Métrica | O que observar |
|---|---|
| Imediatas grandes | Uso de `lui` + `addiu` vs `lui` + `ori` |
| Constantes float | Carregamento de float imediato |

### 7. ABI

| Métrica | O que observar |
|---|---|
| Stack alignment | 8-byte ou 16-byte? |
| Struct return | Em `$v0` ou ponteiro oculto? |
| Varargs | Como `va_list` é tratado |

## Micro-target inspirado na Rev.043

Este é o teste mais específico e relevante para o projeto:

```c
// Reproduz o padrão observado na Rev.043:
// 0x001d27a8(0) consome a0 (contexto/entidade) e a1 (initializer_arg)
// [initializer_arg + 0x30] -> [payload + 0x04]

struct payload_header {
    int unk00;
    int variant;   // offset 0x04 - field written by initializer
    int unk08;
    int unk0C;
    void *parent;  // offset 0x10
};

struct init_arg {
    char pad[0x30];  // padding até o field de interesse
    int variant_id;  // offset 0x30
};

struct entity {
    int id;
    struct payload_header *payload;  // armazenado em [entity + 0x800]
};

void cloth_init(struct entity *ent, struct init_arg *arg) {
    struct payload_header *p = ent->payload;  // load de [entity + 0x800?]
    if (p) {
        p->variant = arg->variant_id;         // [arg+0x30] -> [payload+0x04]
    }
}
```

Compilar com ICP+EABI e comparar o assembly contra:
- `/tmp/ico-cloth-full/asm/cloth_payload_init_001d27a8.s`

## Como comparar com ICO

Para cada micro-target compilado:

1. **Salvar output**: `mt1_icp.s`, `mt1_icp+eabi.s`, etc.
2. **Extrair função correspondente do ICO**: do splat output, localizar
   função com padrão similar (ex: `add` em `func_0010xxxx`)
3. **Comparar**:
   - Prólogo: `addiu $sp, $sp, -N` — mesmo N?
   - Registradores salvos: mesmos `$s0-$s7`, `$ra`?
   - Instruções: mesma sequência?
   - Epílogo: `jr $ra` com slot `addiu $sp`?
4. **Anotar diferenças**: cada divergência entre o GCC PS2 Linux e o ICO
5. **Classificar**:
   - `match`: idêntico ou trivial
   - `close`: mesma sequência, registradores diferentes
   - `different`: abordagem diferente
   - `missing`: ICO tem instrução que GCC não gera

### Exemplo de comparação (add)

Do GCC 2.95.2 (já testado em container):
```asm
add:
    j $31
    addu $2,$4,$5
```

Do ICO (função similar):
```asm
    jr $ra
    addu $v0, $a0, $a1
```

**Diferenças**: `j $31` vs `jr $ra` (mesmo encoding — `$31` = `$ra`).
`$2` vs `$v0`, `$4` vs `$a0`, `$5` vs `$a1` (mesmos registradores,
nomes diferentes). **Match funcional**.

## Resultados esperados

| Micro-target | ICP | ICP+EABI | ICP+o32 | ICP+nor5900 |
|---|---|---|---|---|
| MT1 — store/load | Close | Close | Close | Close |
| MT2 — struct copy | **Rev.043 alvo** | **Rev.043 alvo** | Pode diferir | Close |
| MT3 — jump table | **Comparar com dispatcher** | Idem | Idem | Pode diferir |
| MT4 — struct ptr | Close | Close | Pode diferir stack frame | Close |
| MT5 — indirect call | **Comparar com cb48** | **Comparar com cb48** | Pode diferir | Close |
| MT6 — multi args | **Ver stack args** | EABI vs o32 | **Stack layout** | **Stack layout** |
| MT7 — float | **Comparar com cloth sub** | Idem | Idem | Sem tuning |

### Hipóteses

1. **ICP (ICO Candidate Profile)** deve produzir o assembly mais próximo
   do ICO para a maioria dos casos
2. **ICP+EABI** pode ser crucial para calling convention — o ICO usa
   `-mabi=eabi`
3. **ICP+o32** (default) provavelmente difere na passagem de argumentos
4. **ICP+nor5900** pode mostrar diferenças em scheduling e loop optimization
5. O padrão `[a1+0x30] -> [payload+0x04]` (MT2) deve ser bem representado
   com ICP+EABI

## Limitações

1. **Sem `-march=r5900`**: o PS2 Linux GCC não tem a flag específica do SDK
2. **Target Linux**: o compilador espera Linux (glibc, syscalls), não bare-metal
3. **Patches do SDK**: faltam patches adicionais do `ee-gcc 2.9-991111-01`
4. **Newlib vs glibc**: diferenças de biblioteca padrão podem afetar chamadas
   (não relevante para micro-targets sem libcalls)
5. **Número limitado de micro-targets**: 7 testes não cobrem todo o codegen,
   mas são suficientes para identificar padrões principais
6. **Container necessário**: os testes precisam do Docker rodando — sem
   container, sem resultados

## Próximo passo recomendado

1. ✅ **Plano de codegen documentado** (este documento)
2. 🔄 **Rodar micro-targets no container i386** com todas as variações de flags
3. 🔄 **Comparar cada output com ICO** (do splat em `/tmp/ico-cloth-full/`)
4. 🔄 **Documentar resultados comparativos** em nota separada
5. 🔄 **Decidir se vale construir compiler package decomp.me** com base nos resultados
6. 🔄 **Runtime capture** (PCSX2) para resolver `a1` — pendente

## Veredito

```txt
Utilidade do plano para codegen: ALTA
Utilidade para compiler package decomp.me: MÉDIA-ALTA
Chance de ICP+EABI ser próximo do ICO: ALTA
Chance de matching perfeito sem patches SDK: BAIXA
Runtime continua necessário: SIM
```

O plano de codegen preenche a lacuna entre "temos um GCC parecido" e
"sabemos quão parecido ele é". Os micro-targets cobrem os padrões
específicos do ICO (jump table, indirect call, struct copy com offset)
e as variações de flags permitem isolar a combinação mais próxima.

Os resultados, mesmo sem matching perfeito, serão úteis para:
1. Criar um compiler package decomp.me informado
2. Documentar diferenças de codegen entre PS2 Linux e SDK comercial
3. Estimar a viabilidade de matching para funções específicas

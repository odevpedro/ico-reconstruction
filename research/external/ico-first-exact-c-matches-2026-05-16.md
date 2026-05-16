# ICO First Exact C Matches

## Date

2026-05-16

## Objective

Revisar o resultado anterior de `near-match` dos primeiros accessors cloth e
testar se a divergencia sistematica `ld` vs `lw` podia ser eliminada por uma
modelagem C diferente dos ponteiros do jogo.

## Scope

Incluido:

- recompilacao local com a imagem Docker `ps2-gcc-295`;
- uso do GCC 2.95.2 PS2 Linux como proxy experimental;
- montagem do assembly gerado com `mips64r5900el-ps2-elf-as`;
- comparacao instrucional dos accessors `0x001d3d70`, `0x001d3d80` e
  `0x001d3d98`;
- atualizacao conservadora do status de matching.

Excluido:

- alegar que o compilador exato do ICO foi recuperado;
- rebuild de ELF completo;
- matching de funcoes grandes;
- submissao ao decomp.me;
- distribuicao de bytes proprietarios alem de sequencias instrucionais curtas
  ja documentadas como offsets/comportamento.

## Sources Used

| Fonte | Uso |
|---|---|
| `research/external/first-scratch-func-001d3d70-results.md` | baseline anterior de near-match |
| `research/external/cloth-cluster-scratch-candidates.md` | ordem dos primeiros candidatos |
| `.local/extracted/SCUS_971.13.elf` | bytes locais para disassembly dos alvos |
| `splat/ee-ico-compile.sh` | wrapper do GCC 2.95.2 PS2 Linux via Docker |
| Docker image `ps2-gcc-295` | compilador experimental |
| `/home/peter/ps2dev/ee/bin/mips64r5900el-ps2-elf-as` | montagem do assembly gerado |
| `/home/peter/ps2dev/ee/bin/mips64r5900el-ps2-elf-objdump` | verificacao do objeto montado |

## Evidence Used

| Evidencia | Resultado |
|---|---|
| C com ponteiros `void*` | gera `ld` para carregar ponteiros, near-match anterior |
| C com enderecos do jogo modelados como `int` | gera `lw` para carregar ponteiros |
| Objdump do objeto montado para `0x001d3d70` | instrucoes iguais ao alvo |
| Objdump do objeto montado para `0x001d3d80` | instrucoes iguais ao alvo, incluindo `sltiu` |
| Objdump do objeto montado para `0x001d3d98` | instrucoes iguais ao alvo |
| Tentativa inicial para `0x001d3d40` e `0x001d40a0` | estrutura proxima, mas nao exact match |

## Byte-Level Or Instruction-Level Findings

### Key modeling correction

O C anterior tratava os ponteiros carregados de estruturas como `void*`:

```c
void *entity = *(void**)((char*)context + 0x15C);
```

Com `-mgp64`, o GCC 2.95.2 PS2 Linux gera `ld` para esse caso.

A modelagem que produz o padrao do ICO trata os ponteiros armazenados nas
estruturas como words de 32 bits:

```c
int entity = *(int*)((char*)context + 0x15C);
int payload = *(int*)((char*)entity + 0x800);
```

Isso gera `lw`, que corresponde ao binario do ICO.

### Exact match 1 — `0x001d3d70`

C testado:

```c
int cloth_get_variant(void *context) {
    int entity = *(int*)((char*)context + 0x15C);
    int payload = *(int*)((char*)entity + 0x800);
    return *(int*)((char*)payload + 4);
}
```

Objeto montado:

```asm
0x00000000: lw   v0,0x15c(a0)
0x00000004: lw   v1,0x800(v0)
0x00000008: jr   ra
0x0000000c: lw   v0,0x4(v1)
```

Alvo ICO:

```asm
0x001d3d70: lw   v0,0x15c(a0)
0x001d3d74: lw   v1,0x800(v0)
0x001d3d78: jr   ra
0x001d3d7c: lw   v0,0x4(v1)
```

Veredito: exact instruction match.

### Exact match 2 — `0x001d3d80`

C testado:

```c
int cloth_payload_field0_is_zero(void *context) {
    int entity = *(int*)((char*)context + 0x15C);
    int payload = *(int*)((char*)entity + 0x800);
    return *(unsigned int*)((char*)payload + 0) < 1;
}
```

Objeto montado:

```asm
lw     v0,0x15c(a0)
lw     v1,0x800(v0)
lw     v0,0x0(v1)
jr     ra
sltiu  v0,v0,1
nop
```

Alvo ICO:

```asm
0x001d3d80: lw     v0,0x15c(a0)
0x001d3d84: lw     v1,0x800(v0)
0x001d3d88: lw     v0,0x0(v1)
0x001d3d8c: jr     ra
0x001d3d90: sltiu  v0,v0,1
0x001d3d94: nop
```

Veredito: exact instruction match.

### Exact match 3 — `0x001d3d98`

C testado:

```c
int cloth_payload_state_is_two(void *context) {
    int entity = *(int*)((char*)context + 0x15C);
    int payload = *(int*)((char*)entity + 0x800);
    return (*(unsigned int*)((char*)payload + 0x48) ^ 2) < 1;
}
```

Objeto montado:

```asm
lw     v0,0x15c(a0)
lw     v1,0x800(v0)
lw     v0,0x48(v1)
xori   v0,v0,0x2
jr     ra
sltiu  v0,v0,1
```

Alvo ICO:

```asm
0x001d3d98: lw     v0,0x15c(a0)
0x001d3d9c: lw     v1,0x800(v0)
0x001d3da0: lw     v0,0x48(v1)
0x001d3da4: xori   v0,v0,0x2
0x001d3da8: jr     ra
0x001d3dac: sltiu  v0,v0,1
```

Veredito: exact instruction match.

## Tables

### Match status

| Function | Previous status | New status | Key change |
|---|---|---|---|
| `0x001d3d70` | near-match | exact instruction match | model pointers as 32-bit `int` words |
| `0x001d3d80` | near-match | exact instruction match | same |
| `0x001d3d98` | near-match | exact instruction match | same |

### Modeling comparison

| C model | Pointer load emitted | Match impact |
|---|---|---|
| `void *entity = *(void**)(...)` | `ld` | near-match only |
| `unsigned int entity = *(unsigned int*)(...)` | `lwu` | still not exact |
| `int entity = *(int*)(...)` | `lw` | matches ICO pattern |

## Confirmed

1. The previous `ld` vs `lw` mismatch is not fixed by changing compiler flags
   alone in this test; it is fixed by changing the C type model.
2. Treating stored game pointers as signed 32-bit words produces `lw`.
3. `0x001d3d70`, `0x001d3d80`, and `0x001d3d98` now have exact instruction
   matches under the tested pipeline.
4. The `j $31` syntax emitted by GCC assembles to `jr ra`.
5. A mesma correcao de tipo nao torna automaticamente `0x001d3d40` e
   `0x001d40a0` exact matches; elas exigem tuning adicional de C para ordem de
   instrucoes e registradores.

## Probable

1. Many ICO structure fields that hold pointers should be modeled as 32-bit
   address words during early matching, then wrapped with typedefs/macros only
   after the codegen behavior is stable.
2. The remaining first-scratch accessors may still be tractable, but they are
   not solved by the pointer-width correction alone.
3. This type-modeling correction is more actionable than searching for a new
   compiler immediately.

## Possible

1. A project-local typedef such as `typedef int ico_ptr32;` may help make
   decompilation attempts explicit without pretending these are host pointers.
2. Larger functions may need a mixed model: 32-bit address words for stored
   game pointers, regular integer/float fields for payload data.
3. Some remaining differences in later functions may still require the exact
   Sony compiler or handwritten asm.

## Unknown

1. Whether `0x001d3d40`, `0x001d3db0`, and `0x001d40a0` can be brought to
   exact match with local C tuning.
2. Whether this holds for non-leaf functions with stack frames and calls.
3. Whether an exact full-ELF rebuild is possible without EE GCC
   `2.9-991111-01`.

## Discarded

1. The earlier interpretation that `ld` vs `lw` is necessarily only a compiler
   version limitation. It is at least partly a C type-modeling issue.
2. Using `unsigned int` for stored pointers when exact `lw` is required:
   it emits `lwu`, not `lw`.
3. Treating these first accessors as merely near-match after this correction.

## Next Minimum Test

| Priority | Test | Reason |
|---|---|---|
| 1 | Tune C for `0x001d3d40` and `0x001d40a0` after the first naive attempt | solve register allocation/order differences |
| 2 | Create a tiny project-local C scratch source for exact matched accessors | preserve reproducible C inputs without generated proprietary data |
| 3 | Try first small non-leaf candidate `0x001d40d8` | test whether the approach scales beyond leaf accessors |

## Conservative Verdict

The project has crossed from "near-match experiments" into real decompilation
evidence for a small part of the cloth cluster. At least three tiny accessor
functions now have exact instruction matches when stored game pointers are
modeled as 32-bit signed address words.

This does not mean the hard functions are close yet. It does mean the
decompilation path is valid, and the next work should grow outward from these
exact matches rather than jumping directly to large runtime-dependent
functions.

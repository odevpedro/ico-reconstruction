# Cloth Exact Match Next Steps

## Resumo executivo

Os primeiros 3 exact C matches do cluster cloth foram obtidos ao modelar
ponteiros internos do ICO como `int` (32-bit) em vez de `void*`. Isso
eliminou a divergência sistemática `ld` vs `lw` que bloqueava 6 accessors.
O pipeline de compilação experimental (PS2 Linux GCC 2.95.2 ICP, montagem
com `mips64r5900el-ps2-elf-as`) está validado para funções accessor.
O próximo movimento é retestar as 3 funções restantes do primeiro lote
(`0x001D3DB0`, `0x001D3D40`, `0x001D40A0`) com a nova regra de tipos,
uma por vez, começando por `0x001D3DB0`.

## O que os 3 exact matches provaram

1. O pipeline `splat asm -> C mínimo -> GCC 2.95.2 ICP -> montagem ->
   comparação instrucional` **produz instruções idênticas** para accessors
   simples quando a modelagem de tipos está correta.
2. A divergência `ld` vs `lw` não é apenas um problema de compilador —
   é também um problema de modelagem de tipos em C.
3. Os offsets `+0x15C`, `+0x800`, `+0x04`, `+0x00`, `+0x48` estão
   **duplamente confirmados** (antes por near-match, agora por exact match).
4. `signed int` para carga de endereço gera `lw`; `unsigned int` gera `lwu`;
   `void*` gera `ld`. O ICO usa `lw` — endereços armazenados como signed
   32-bit.

## O que mudou sobre `ld` vs `lw`

| Hipótese anterior | Nova evidência |
|---|---|
| `ld` vs `lw` é necessariamente diferença de compilador (`ee-gcc 2.9` vs `PS2 Linux GCC 2.95.2`) | É pelo menos parcialmente diferença de modelagem de tipos — `void*` força `ld` em `-mgp64` |
| A divergência é absoluta e não contornável sem o compilador exato | Três funções foram convertidas de near-match para exact match apenas mudando `void*` para `int` |
| Todas as 6 funções testadas eram igualmente near-match | Agora há uma separação: 3 são exact match, 3 continuam near-match |

A correção não torna `0x001D3DB0`, `0x001D3D40` e `0x001D40A0`
automaticamente exact match — elas têm branches e/ou diferenças de ordem
de instruções que exigem tuning adicional de C.

## Regra provisória: `ico_ptr32`

```c
typedef int ico_ptr32;
```

`ico_ptr32` representa um endereço interno do jogo armazenado em 32 bits.
Não é um ponteiro do host. O tipo `int` (signed 32-bit) foi escolhido porque
produz `lw` (não `lwu`). O nome `ico_ptr32` deixa explícito que:
- é um endereço, não um inteiro arbitrário;
- tem 32 bits, não 64;
- a semântica de "ponteiro" só existe dentro do domínio do jogo.

Uso em structs provisórias:

```c
typedef struct EntityLike {
    char      pad_000[0x800];
    ico_ptr32 payload;    // +0x800
} EntityLike;
```

Cautelas:
- Não é tipo final — pode precisar de wrapper `unsigned` se o jogo usar
  `lwu` em outras funções.
- Não aplicar cegamente a todas as estruturas. Validar por função.
- Preferir `ico_ptr32` só depois de confirmed `lw` no alvo.

## Funções exatas já confirmadas

| Função | Bytes | Status | C usado |
|---|---|---|---|
| `0x001D3D70` | 16 | exact match | `int entity = *(int*)(ctx + 0x15C)` + `int payload = *(int*)(entity + 0x800)` |
| `0x001D3D80` | 24 | exact match | mesmo padrão + `sltiu` |
| `0x001D3D98` | 24 | exact match | mesmo padrão + `xori` + `sltiu` |

Estas 3 funções servem como gabarito de C para os próximos testes.

## Próximas funções para retestar

A ordem sugerida prioriza as que já foram testadas como near-match e
precisam apenas de reajuste com `ico_ptr32`:

| Ordem | Função | Bytes | Problema anterior | Objetivo |
|---|---|---|---|---|
| 1 | `0x001D3DB0` | 40 | `sltu` vs `sltiu`, register allocation | retestar com `ico_ptr32` + ajuste fino de C |
| 2 | `0x001D3D40` | 48 | `bne` vs `beqz`, register allocation | retestar com `ico_ptr32` + ajuste de condicional |
| 3 | `0x001D40A0` | 56 | `bne` vs `beq`, null check pattern | retestar com `ico_ptr32` + ajuste de branch |
| 4 | `0x001D4358` | 160 | não testado com `ico_ptr32` | primeiro candidato novo após retestes |
| 5 | `0x001D3BF0` | 336 | não testado com `ico_ptr32` | testar escalabilidade para funções com float |

Cada função deve ser testada em rodada separada, com documento próprio,
para manter a rastreabilidade.

## Funções a evitar por enquanto

| Função | Motivo |
|---|---|
| `0x001D27A8` (cloth_payload_init) | depende de runtime — `a1` = stack initializer, requer breakpoint para validar struct completa |
| `0x001D37C8` (cloth_dispatcher) | jump table com `.word` (4B); GCC gera `.dword` (8B); não contornável sem compilador exato ou linker script |
| `0x001D40D8`, `0x001D3DD8`, `0x001D4170` | dependem de `func_0013EB50`/`func_0013EBE0` (state resolver/iterator) — semântica do iterador não está clara |
| `0x001D4348` (thunk) | melhor manter como assembly (tail call `j` com GP-relative) |
| `0x001D29B8` em diante (setup, sim, subs) | grandes demais — requires contexto de structs validado primeiro |

## Atualização sugerida nas structs provisórias

O arquivo `research/external/cloth-struct-hypotheses.md` deve ser atualizado
com:

1. Substituir `void *entity` por `ico_ptr32 entity` (ou `int entity`) nos
   campos que armazenam endereços internos do jogo.
2. Manter `void*` para campos que são ponteiros do sistema (ex: callbacks
   em `DescriptorRecord.slot_58` — esses são endereços de função, não
   armazenados como 32-bit? Verificar).
3. Adicionar nota sobre a regra provisória `ico_ptr32` e sua justificativa.
4. Não renomear `ClothPayload` para nome definitivo ainda — semântica real
   ainda não confirmada.

A struct `Entity` (contém payload em +0x800) deve ficar:

```c
typedef int ico_ptr32;

struct Entity {
    char     pad_000[0x800];
    ico_ptr32 payload;    // +0x800 — endereço do ClothPayload como word 32-bit
};
```

A struct `EntityContext` deve ficar:

```c
struct EntityContext {
    char     pad_000[0x15C];
    ico_ptr32 entity;     // +0x15C — endereço da entidade como word 32-bit
    char     pad_160[0x10];
    ico_ptr32 extra_ptr;  // +0x16C — endereço auxiliar (opcional)
};
```

## Plano para fontes C experimentais

Criar um arquivo de referência `research/external/cloth-exact-match-c-sources.md`
contendo:

- código C mínimo de cada função testada (exact match, near-match, mismatch);
- structs provisórias usadas;
- flags de compilação (`-fno-pic -mno-abicalls -G0 -O2 ...`);
- assembly esperado (comentado, apenas offsets relativos);
- veredito de cada função.

Estrutura proposta por função:

```markdown
### func_001D3D70

```c
int cloth_get_variant(void *context) {
    int entity = *(int*)((char*)context + 0x15C);
    int payload = *(int*)((char*)entity + 0x800);
    return *(int*)((char*)payload + 4);
}
```

Assembly esperado:
```asm
lw v0,0x15c(a0)
lw v1,0x800(v0)
jr ra
lw v0,0x4(v1)
```

Veredito: **exact match**.

Flags: `-fno-pic -mno-abicalls -G0 -O2 -march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float`
```

Este arquivo serve como:
- fonte única de verdade para o que foi testado;
- insumo para scratches no decomp.me;
- referência para não reintroduzir `void*` acidentalmente.

Não versionar `.c` compiláveis ainda — manter apenas markdown até que
o conjunto de fontes esteja maduro.

## Classificação de resultados

| Categoria | Definição | Exemplos |
|---|---|---|
| **exact match** | Instruções idênticas, mesma ordem, mesmos registradores (exceto endereços absolutos) | `0x001D3D70`, `0x001D3D80`, `0x001D3D98` |
| **near-match** | Estrutura, offsets e fluxo idênticos; diferença em instrução específica (tipo de load, variante de branch) ou registrador | `0x001D3DB0`, `0x001D3D40`, `0x001D40A0` |
| **mismatch útil** | Estrutura similar mas diferenças que revelam informação sobre o compilador ou structs (ex: diferença sistemática em prólogo/epílogo) | Nenhum ainda |
| **blocked** | Impeditivo conhecido que não pode ser resolvido com tuning local de C (jump table `.word`, runtime dependency, thunk) | `0x001D37C8`, `0x001D27A8`, `0x001D4348` |

Uma função pode mover de `near-match` para `exact match` com tuning de C.
Uma função `blocked` só sai da categoria se o impeditivo for removido
(compilador correto para jump table, runtime para a1).

## Próximo passo sem emulador

1. **Retestar `0x001D3DB0`** com `ico_ptr32`/`int`. Criar
   `research/external/exact-match-attempt-func-001d3db0.md`.
2. Se `0x001D3DB0` virar exact match: retestar `0x001D3D40`.
3. Atualizar `cloth-struct-hypotheses.md` com a regra `ico_ptr32`.
4. Criar `research/external/cloth-exact-match-c-sources.md` consolidando
   os C sources testados.

## Próximo passo com PCSX2

1. Breakpoint no caller `0x001B7A74` — confirmar quem chama o caller de
   `0x001D27A8` e em que contexto.
2. Acumular mais hits de `0x001D27A8` — ver se `[a1 + 0x30]` varia entre
   diferentes inicializações de cloth.
3. Capturar outro `DescriptorRecord` além de `BARREL` (0x002A3924) —
   verificar se outros descritores usam callbacks em `+0x58` com a mesma
   estrutura.
4. Se possível: breakpoint em `0x001D3DB0`, `0x001D3D40`, `0x001D40A0` para
   confirmar que são chamadas durante gameplay e com que frequência.

## Riscos

1. **Falso positivo de compilador**: os exact matches não provam que o
   PS2 Linux GCC 2.95.2 é o compilador do ICO. Provam apenas que ele pode
   gerar as mesmas instruções para funções simples quando a modelagem de
   tipos está correta.
2. **Supergeneralização da regra `ico_ptr32`**: nem todo campo que parece
   ponteiro será `ico_ptr32`. Alguns podem ser `void*` legítimos (callbacks,
   ponteiros de sistema). Validar por função.
3. **Estagnação em accessors**: há risco de gastar tempo demais polindo
   accessors simples. O objetivo é provar o pipeline, não decompilar só
   accessors.
4. **Tuning excessivo de C**: se uma função precisar de C artificialmente
   complexo para casar, pode ser sinal de que o compilador ou as flags
   estão erradas.

## Veredito

```
Exact matches obtidos:     3 (0x001D3D70, 3D80, 3D98)
Near-matches pendentes:    3 (0x001D3DB0, 3D40, 40A0)
Regra ico_ptr32:          ✅ provisória, documentada
Pipeline validado:        ✅ para accessors
Funções bloqueadas:       9 mantidas (hold list unchanged)
Próximo movimento:        retestar 0x001D3DB0 com ico_ptr32
```

O projeto tem agora um pipeline funcional e uma regra de tipos que elimina
a principal divergência sistemática. O foco imediato é converter os 3
near-matches restantes do primeiro lote em exact matches, um por vez.

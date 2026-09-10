# Rev.163 — 8 arquivos .s corrigidos/expandidos byte-exact, inventário estático de alvos e GetEnemyDefLife byte-exact

## Title
Rev.163 — Correções byte-exact (4 ASM-ERR + 4 stubs), inventário estático de alvos de reconstrução e primeira conversão nova (GetEnemyDefLife)

## Date
2026-09-10

## Objective
Rodada estática sem runtime: fechar os 8 gaps restantes de `.s` não-byte-exact
(4 ASM-ERR + 4 trivial stubs), produzir um inventário de funções nomeadas
verificadas sem reconstrução (fila de decompilação), e executar o pipeline
completo de reconstrução numa função nova (item 5).

## Scope
- ELF USA `.local/extracted/SCUS_971.13.elf` (LOAD vaddr=0x00100000, off=0x1000).
- Montador: `toolchain/ee-gcc2.9-991111-01/bin/ee-gcc` (assembler only).
- Verificação: `.text` do `.o` assemblado == bytes do ELF no VA alvo (zero tolerância).
- Inventário via `docs/symbols/pal_usa_symbol_map.csv` + `capstone`.

## Results

### 1. 4 ASM-ERR (COP2/HPI `ld.b $w0`) → byte-exact via `.word 0x7ba80020`
`0x7ba80020` decodifica por capstone como `ld.b $w0,-0x58($zero)`. O ee-as 2.9
rejeita a forma mnemônica; o `.word` cru elimina o erro e mantém os bytes.

| Arquivo | VA | Tamanho | Status |
|---------|-----|---------|--------|
| `src/entity/asm/boyAI_sub_1562D4.s` | 0x1562D4 | 0x70 | byte-exact ✅ |
| `src/entity/asm/boyAI_sub_1562DC.s` | 0x1562DC | 0x74 | byte-exact ✅ |
| `src/entity/asm/boyAI_sub_1562E0.s` | 0x1562E0 | 0x12c | byte-exact ✅ |
| `src/entity/asm/boyAI_sub_1562E8.s` | 0x1562E8 | 0x124 | byte-exact ✅ |

### 2. 4 trivial stubs → expandidos e byte-exact
Nenhum era placeholder de 8 bytes genérico; os bytes reais foram preservados.

| Arquivo | VA | Bytes | Semântica real |
|---------|-----|-------|----------------|
| `isysGObjActiveLink.s` | 0x13EC40 | 8 | `jr $31; move $2,$5` → retorna a1 (accessor puro) |
| `isysGObjActiveDlLink.s` | 0x141160 | 8 | `jr $31; nop` → no-op literal |
| `isysGObjProcPause.s` | 0x13F808 | 8 | `jr $31; sw $0,0x18($4)` → zera ProcessNode.active (+0x18) |
| `boyAI_sub_14BB08.s` | 0x14BB08 | 0x14 | grava float em `*(*(gobj+0x670))+0x58` e zera `+0x54` |

Semântica refletida em `src/core/gobj_abi.h` e `src/core/isysgobj_semantic.c`
(`ico_semantic_isysGObjActiveLink` retorna `u32`; `ico_semantic_isysGObjActiveDlLink`;
`ico_semantic_isysGObjProcPause` zera `process->active`). CTest: **27/27 pass**.

### 3. Inventário estático de alvos (item 3)
Cruzamento do mapa PAL→USA (verified) contra `.s` existentes por basename e VA
(pipeline `TARGET_FUNCTIONS` + basenames `fn_<VA>`):

- **397 funções nomeadas verificadas** na região de jogo (0x101000-0x280000,
  fora do kernel EE) **sem `.s`**.
- **326 delas com 8..0x200 bytes** — candidatas diretas a conversão semântica C.
- Alvos "quentes" por família: `SetGirlClothDispSwitch` (0x1C3C38, 0x1D0),
  `GirlForceFieldGeo` (0x1C3C90, 0x178), `getReviveEnemyGObj` (0x1E7F38, 0x1A0),
  `subEnemyCollision` (0x15E2C8, 0x180), `HoldRope` (0x1E59A0, 0x154),
  `GetChainDirCorrectVal` (0x18DB90, 0x1EC), `actSt47aEnemy2` (0x21B750, 0x140),
  `getEnemyDefLife` (`GetEnemyDefLife` 0x1C11C0, 0x90), etc.
- Artefato: `research/elf/rev163-static-target-inventory.csv` (397 linhas).

### 4. Primeira conversão nova do pipeline completo (item 5)
`GetEnemyDefLife` @ 0x001C11C0, 144 B, conf=0.95 (PAL→USA verified).

- Acrescentado `src/entity/asm/GetEnemyDefLife.s`.
- **byte-exact: TRUE** (ee-gcc, `.text` do `.o` == ELF no VA, 144 bytes).
- Duas instruções exigiram `.word` devido ao remapeamento de registradores float
  64-bit do ee-as (f16→f32 em `mtc1`; `add.s` duplicado em par ímpar):
  - `0x44810800` (mtc1 $f16) e `0x46010000` (add.s com ft=1/fd=1).
- Semântica provável (fato byte-level; interpretação): `a0=*a0`, lê
  `gobj_entity->proc->type` em cascata (a0→+0x15c→+0x800→+0x20), se o tipo
  != 5 adiciona frame de 0x50 e chama 3 funções (`func_0x104488`, `func_0x105F00`,
  `CreateGObj`-family `func_0x243AD0`) e retorna 1; não-<> usa apenas o
  retorno `$f0` da função como "life" e retorna 1. NOMENCLATURA `GetEnemyDefLife`
  é do mapa PAL→USA (não verificado por callers).

## What is confirmed
- 8 arquivos `.s` que estavam na lista "not byte-exact" agora são byte-exact
  (4 ASM-ERR + 4 stubs) — método `assemble_and_verify`, zero tolerância.
- Contagem: **700 de 728 `.s` verificados** (612 pipeline + 88 fora do pipeline;
  os 4 novos `.s` deste rev fizeram fora-do-pipeline subir). 397 alvos inventariados.
- `GetEnemyDefLife.s` novos bytes coincidem 100% com o ELF USA.

## What is probable
- A função `func_0x243AD0` chamada em `GetEnemyDefLife` é da família GObj
  (não confirmado por símbolo neste rev).

## What is possible
- Converter um dos 326 alvos pequenos para C semântico com teste CTest nativo
  (mesmo padrão de `isysgobj_semantic`), a partir do novos `.s`.

## What is unknown
- Callers reais de `GetEnemyDefLife` (0 estático? não auditado neste rev).
- O que `func_0x104488`/`0x105F00` fazem (não nomeadas).

## What is discarded
- `mtc1`/`add.s` com nomes `$fN` para single-precision: o ee-as 2.9 remapeia
  (pareamento 64-bit) e não é confiável para byte-exactness → preferir `.word`.
- Regerar o mapa PAL→USA por `reconcile.py` sozinho: perde cobertura (rev162).

## Next minimum test
- `GetEnemyDefLife`: verificação de callers executada — **0 jal estáticos para
  0x1C11C0** (word 0x0C070470 não aparece no .text). Como não se pode confirmar
  o nome por xref duplo, o nome permanece candidato (PAL→USA, conf 0.95); a
  função pode ser disparada por pointer/vtable. Próximo teste: varredura de
  referências de data (carregar 0x1C11C0 via lui/addiu) antes de decidir.

## Conservative verdict
Rodada sem runtime concluída: 8 gaps de `.s` fechados byte-exact, inventário
estático publicado (397 alvos nomeados verificados sem `.s`), e o primeiro
`.s` novo de função nomeada gerado e verificado (`GetEnemyDefLife`, 144 B).
A fila de decompilação C semântico (item 5) agora tem alvos concretos.
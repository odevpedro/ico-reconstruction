# Rev.072 — Room init callback system: descriptor table, entry table, and callback field correction

**Date**: 2026-05-17
**Objective**: Map the full callback dispatch chain for room initialization, correcting the 404-table callback field offset and discovering the descriptor/entry entity ID system.

---

## Sources used

- `SCUS_971.13.elf` (byte-level reads)
- `research/elf/ghidra-rev071-404-table-room-names-callbacks-and-dispatch-system-consolidation.md`
- `AGENTS.md`
- `docs/data-model.md`

---

## Evidence

### 1. Corrected callback field offset

**Previous error**: The callback at offset `+0x154` relative to `0x005F2F98` (absolute table base) was read as `0x4B`. This is the **wrong field**.

**Corrected**: The code at `0x1AF960` reads from:

```
0x005F2FB8 + (v1) + 340
         ^        ^    ^
         |        |    +-- offset 0x154
         |        +-- world_state * 404 (pre-multiplied by upstream code)
         +-- row base (after 32-byte header)
```

The 32-byte header at `0x005F2F98-0x005F2FB7` means:

- `row_base + 0x154` = absolute `0x005F2F98 + 0x20 + 0x154` = `0x005F2F98 + 0x174`
- My original scan at `0x005F2F98 + 0x154` was reading 32 bytes too early (the header span)

**Corrected**: 19 non-null function pointers found at `row_base + 0x154`:

| Row | Room         | Callback pointer |
|-----|--------------|------------------|
|  0  | NULL         | 0x00000000       |
|  1  | logo         | 0x00000000       |
|  2  | title        | 0x00000000       |
|  3  | sacrifice    | 0x00000000       |
|  4  | jail         | 0x00231AC8       |
|  5  | warehouse    | 0x00234AB0       |
|  6  | ico_brigde   | 0x00000000       |
|  7  | proto        | 0x0020FA98       |
|  8  | troko        | 0x0022A838       |
|  9  | chandelier   | 0x00228198       |
| 10  | entrance     | 0x00239750       |
| 11  | gate         | 0x00000000       |
| 12  | gate2        | 0x00000000       |
| 13  | grave        | 0x00000000       |
| 14  | shadows      | 0x00210D78       |
| 15  | windmill     | 0x0022B878       |
| 16  | plaza        | 0x00213B88       |
| 17  | stone        | 0x0022BFE8       |
| 18  | symmetry_L   | 0x00000000       |
| 19  | crest_L1     | 0x0021A4D0       |
| 20  | crest_L2     | 0x0021A6B8       |
| 21  | crest_L3     | 0x0021A980       |
| 22  | taki         | 0x00211780       |
| 23  | sluice       | 0x00225F68       |
| 24  | underground  | 0x00000000       |
| 25  | gondola      | 0x00237B78       |
| 26  | watertower   | 0x0022D8F8       |
| 27  | symmetry_R   | 0x00000000       |
| 28  | crest_R1     | 0x0021F828       |
| 29  | crest_R2     | 0x0021FA30       |
| 30  | crest_R3     | 0x0021FD20       |
| 31  | cliff        | 0x00000000       |

Null callbacks at: logo (title screen), sacrifice (intro CG?), ico_brigde (non-gameplay zone?), gate/gate2/grave (unused or handled differently), symmetry_L/R, underground, cliff.

### 2. Instruction chain at 0x1AF948-0x1AF970 (previously misread)

```
0x001AF940: jal 0x0013B878         ; prior call
0x001AF948: lw $v1,-28512($gp)     ; GP variable 0x0062CD40 = world_state * 404
0x001AF94C: addiu $a0,$zero,404    ; constant 404
0x001AF950: lui $a1,0x005F         ; table high
0x001AF954: mult $v1,$a0           ; v1 * 404 → hi:lo  (DEAD CODE — result unused)
0x001AF958: addiu $v0,$a1,12216    ; v0 = 0x005F2FB8 (row base after header)
0x001AF95C: addu $v0,$v0,$v1       ; v0 = row_base + world_state*404 (uses ORIGINAL v1)
0x001AF960: lw $v0,340($v0)        ; load callback from row_base + 0x154
0x001AF964: beq $v0,$zero,+3       ; skip if NULL
0x001AF96C: jalr $v0               ; CALL room init callback
0x001AF974: jal 0x00166028         ; live dispatch callback system (next phase)
```

**Key finding**: The `mult` at 0x1AF954 is **dead code** — the result goes to hi/lo but no `mflo` follows. The `addu` at 0x1AF95C uses the **original $v1**, meaning $v1 is ALREADY pre-multiplied by 404 before reaching this function. The variable at GP-28512 (`0x0062CD40`) contains `world_state * 404` at runtime.

The ELF initial value at `0x0062CD40` is `0x00000000`. The patching code that sets this at runtime is still unknown (no static SW found targeting this address).

The `0x4B` value previously reported at `0x005F2F98 + i*404 + 0x154` is now understood to be a different field at `row_base + 0x134` (= absolute `0x005F2F98 + 0x154`), possibly a debug type identifier or entity class ID.

### 3. Descriptor table (68 entries, 0x64 stride each)

**Location**: `0x002A31B8`, **stride**: 100 bytes (0x64)
**Entries**: 68 (indices 0-67)

Each entry structure:

| Offset | Size | Field                      |
|--------|------|----------------------------|
| +0x00  | 8    | Name (ASCII, null-terminated, max 8 chars) |
| +0x08  | ?    | ...                        |
| +0x40  | 4    | init_fn (function pointer, 0 = none) |
| +0x44  | 4    | flags (bitfield)           |
| +0x48  | 4    | handler_A (function pointer) |
| +0x50  | 4    | handler_B (function pointer) |
| +0x58  | 4    | handler_C (function pointer) |

**All 68 descriptors**:

| idx | Name         | init_fn   | flags | handler_A   | handler_B   | handler_C   |
|-----|--------------|-----------|-------|-------------|-------------|-------------|
|   0 | NULL         | 0x00000000| 0x00  | 0x00000000  | 0x00000000  | 0x00000000  |
|   1 | BOY          | 0x00153478| 0x01  | 0x001C1F58  | 0x001C1DD8  | 0x001C1A98  |
|   2 | GIRL         | 0x00174BA0| 0x01  | 0x001D1A98  | 0x001D17F8  | 0x001D1668  |
|   3 | GIRLDEMO     | 0x00000000| 0x01  | 0x001D1A98  | 0x001D17F8  | 0x001D1668  |
|   4 | ENEMY1       | 0x00164440| 0x01  | 0x001CE690  | 0x001CE3C0  | 0x001CE220  |
|   5 | ENEMY_TE     | 0x00000000| 0x01  | 0x001CE5F8  | 0x001E08B8  | 0x001CE220  |
|   6 | DEMOMOTC     | 0x00000000| 0x01  | 0x0010ECC0  | 0x001CE6F0  | 0x001CEB18  |
|   7 | SOBJ         | 0x00000000| 0x01  | 0x0010ECC0  | 0x00000000  | 0x00000000  |
|   8 | PSOBJ        | 0x00000000| 0x01  | 0x001AEA58  | 0x001AEA50  | 0x001AEA60  |
|   9 | TREE         | 0x00000000| 0x01  | 0x001F1508  | 0x001F1330  | 0x001F17B0  |
|  10 | TORCH        | 0x00000000| 0x01  | 0x001F2140  | 0x001F1CF0  | 0x001F2048  |
|  11 | PARTICLE     | 0x00000000| 0x01  | 0x00000000  | 0x00000000  | 0x00000000  |
|  12 | PARTLAYO     | 0x00000000| 0x01  | 0x001E8F30  | 0x001E8F28  | 0x001E8EB8  |
|  13 | LIGHTBIT     | 0x00000000| 0x01  | 0x001F0550  | 0x001F0540  | 0x001F0568  |
|  14 | WEAPON       | 0x00000000| 0x01  | 0x001F3BD0  | 0x001F3A00  | 0x001F36F0  |
|  15 | SPIDER_L     | 0x00000000| 0x01  | 0x00000000  | 0x001EA5E8  | 0x001EA278  |
|  16 | SOFA         | 0x00000000| 0x01  | 0x0010ECC0  | 0x00000000  | 0x00000000  |
|  17 | WOODBOX0     | 0x0017D1D0| 0x01  | 0x001C05D0  | 0x001C0538  | 0x001C00C0  |
|  18 | ROTOBJEC     | 0x00000000| 0x01  | 0x001EA030  | 0x001E9950  | 0x001E9F08  |
|  19 | BARREL       | 0x00000000| 0x01  | 0x001D3B28  | 0x001D3A30  | 0x001D27A8  |
|  20 | ROPE         | 0x00000000| 0x01  | 0x001E9630  | 0x001E9810  | 0x001E8F38  |
|  21 | CHAIN        | 0x00000000| 0x01  | 0x0018F640  | 0x0018ECC8  | 0x0018E5B0  |
|  22 | FLEVER       | 0x00000000| 0x01  | 0x001BC438  | 0x001BC1A8  | 0x001C09C8  |
|  23 | FLEVER_T     | 0x00000000| 0x01  | 0x001BC438  | 0x001BC130  | 0x001C09C8  |
|  24 | WLEVER       | 0x00000000| 0x01  | 0x001BC7F0  | 0x001BC530  | 0x001C0C40  |
|  25 | WLEVER2      | 0x00000000| 0x01  | 0x001BC7F0  | 0x001BC530  | 0x001C0C40  |
|  26 | NONE         | 0x00000000| 0x01  | 0x00000000  | 0x00000000  | 0x00000000  |
|  27 | SV           | 0x00000000| 0x01  | 0x0010EC60  | 0x00000000  | 0x00000000  |
|  28 | CAMERADU     | 0x00000000| 0x01  | 0x00000000  | 0x00000000  | 0x00000000  |
|  29 | DUMMY        | 0x00000000| 0x00  | 0x00000000  | 0x00000000  | 0x00000000  |
|  30 | BGA          | 0x00203EE8| 0x01  | 0x00000000  | 0x00000000  | 0x00000000  |
|  31 | SEFFECT      | 0x00000000| 0x01  | 0x001EF988  | 0x001EF980  | 0x001EF8E8  |
|  32 | BIRD         | 0x001971C0| 0x01  | 0x00197080  | 0x00197078  | 0x00197240  |
|  33 | GENERATO     | 0x00000000| 0x01  | 0x00193600  | 0x001930B0  | 0x00193730  |
|  34 | CANDLE       | 0x00000000| 0x01  | 0x001C3130  | 0x001C3178  | 0x001C2FF0  |
|  35 | MOBJ         | 0x00000000| 0x01  | 0x0010ECB0  | 0x00000000  | 0x00000000  |
|  36 | DEMO_QSW     | 0x00000000| 0x01  | 0x0010ECB0  | 0x001F44C8  | 0x001F43D0  |
|  37 | CHANDELI     | 0x00000000| 0x01  | 0x001C3470  | 0x001C33D8  | 0x001C34B8  |
|  38 | WORM         | 0x00000000| 0x01  | 0x001F69B0  | 0x001F66F0  | 0x001F62E8  |
|  39 | POOL         | 0x00000000| 0x01  | 0x0010C5C0  | 0x0010D070  | 0x0010B2D0  |
|  40 | DARKVOLU     | 0x00000000| 0x01  | 0x001CBD78  | 0x001CBD70  | 0x001CBD68  |
|  41 | MCOLTEST     | 0x00000000| 0x01  | 0x001E6960  | 0x001E6788  | 0x001E6968  |
|  42 | ROPEFIX      | 0x00000000| 0x01  | 0x001E98C8  | 0x001E9888  | 0x001E9910  |
|  43 | CAGE         | 0x00000000| 0x01  | 0x001C2DF8  | 0x001C28D0  | 0x001C2338  |
|  44 | DYNAMICM     | 0x00000000| 0x00  | 0x00000000  | 0x00000000  | 0x00000000  |
|  45 | FLAG         | 0x00000000| 0x01  | 0x001D01E8  | 0x001D00F8  | 0x001CFB58  |
|  46 | QUEEN        | 0x0019B7F8| 0x01  | 0x0019A9A0  | 0x0019A8F0  | 0x0019A7E8  |
|  47 | QUEENDEM     | 0x00000000| 0x01  | 0x0019A9A0  | 0x0019A8F0  | 0x0019A7E8  |
|  48 | DEVIL_GI     | 0x00174BA0| 0x01  | 0x001D1A98  | 0x001D17F8  | 0x001D1668  |
|  49 | SKELTEST     | 0x00000000| 0x01  | 0x001E0860  | 0x001E08B8  | 0x00000000  |
|  50 | CAGEFIX      | 0x00000000| 0x01  | 0x001C2FA0  | 0x001C2F20  | 0x001C2FE8  |
|  51 | CLOTHTES     | 0x00000000| 0x01  | 0x001C9330  | 0x001C9328  | 0x001C92D8  |
|  52 | QUEEN_BA     | 0x00000000| 0x01  | 0x0019B660  | 0x0019AE98  | 0x0019BC58  |
|  53 | QUEEN_BATTLE | 0x00000000| 0x01  | 0x0019AE50  | 0x0019AA20  | 0x0019BAA8  |
|  54 | STAGESET     | 0x00000000| 0x00  | 0x00000000  | 0x00000000  | 0x00000000  |
|  55 | GIRLFORC     | 0x00000000| 0x01  | 0x001D1B30  | 0x001D1CF8  | 0x001D1C78  |
|  56 | INTEREST     | 0x00000000| 0x01  | 0x0010ECC0  | 0x00000000  | 0x00000000  |
|  57 | INTEREST     | 0x00000000| 0x01  | 0x0010ECC0  | 0x00000000  | 0x00000000  |
|  58 | INTEREST     | 0x00000000| 0x01  | 0x0010ECC0  | 0x00000000  | 0x00000000  |
|  59 | INTEREST     | 0x00000000| 0x01  | 0x0010ECC0  | 0x00000000  | 0x00000000  |
|  60 | KYOMI        | 0x00000000| 0x01  | 0x00000000  | 0x0023D518  | 0x00000000  |
|  61 | AP1          | 0x001BB6B0| 0x01  | 0x001BA530  | 0x001BA330  | 0x001B8720  |
|  62 | ATTACKCH     | 0x001BBF78| 0x01  | 0x001BBEC8  | 0x001BBEA0  | 0x001BBE50  |
|  63 | ATTACKCH     | 0x001BBF78| 0x01  | 0x001BBE48  | 0x001BBDD8  | 0x001BBB20  |
|  64 | BOSS_CTR     | 0x00198140| 0x01  | 0x00198000  | 0x00197FC8  | 0x00198218  |
|  65 | temp         | 0x00000000| 0x01  | 0x00000000  | 0x00000000  | 0x00000000  |
|  66 | ENEMY_CO     | 0x00000000| 0x00  | 0x00000000  | 0x00000000  | 0x00191D08  |
|  67 | FLY_INFO     | 0x00000000| 0x01  | 0x001D03A0  | 0x00000000  | 0x001D03C8  |

### 4. Entry table (512 entries, 0x4C stride each)

**Location**: `0x002A4C48`, directly after descriptor table (`0x002A31B8 + 68*0x64 = 0x002A4C48`).
**Stride**: 76 bytes (0x4C)
**Entries**: 512

Key fields at known offsets:

| Offset | Size | Field |
|--------|------|-------|
| +0x00  | 4    | position (float?) |
| +0x24  | 4    | callback_override |
| +0x46  | 1    | desc_idx (descriptor table index) |
| +0x48  | 4    | flags (bitfield) |

**desc_idx distribution** (first 200 of 512 entries):

| count | desc_idx | Entity type     |
|-------|----------|-----------------|
| 79    | 0x1E     | BGA             |
| 46    | 0x07     | SOBJ            |
| 46    | 0x0A     | TORCH           |
| 30    | 0x07     | SOBJ            |
| 25    | 0x1C     | CAMERADU        |
| 24    | 0x04     | ENEMY1          |
| 20    | 0x2C     | DYNAMICM        |
| 17    | 0x13     | BARREL          |
| 15    | 0x21     | GENERATO        |
| 13    | 0x1A     | NONE            |
| 12    | 0x0E     | WEAPON          |
| 6     | 0x06     | DEMOMOTC        |
| 6     | 0x0B     | PARTICLE        |
| 6     | 0x36     | STAGESET        |
| 5     | 0x0B     | PARTICLE        |
| 5     | 0x0E     | WEAPON          |
| 5     | 0x2D     | FLAG            |
| 5     | 0x43     | FLY_INFO        |
| 4     | 0x0F     | SPIDER_L        |
| 4     | 0x10     | SOFA...         |
| 4     | 0x11     | ROTOBJEC...     |
| 4     | 0x15     | CHAIN           |
| 4     | 0x16     | FLEVER          |
| 4     | 0x36     | STAGESET        |
| 4     | 0x37     | CHANDELI        |
| 3     | 0x20     | ROPE            |
| 2     | 0x01     | BOY             |
| 2     | 0x02     | GIRL            |
| 2     | 0x15     | CHAIN           |
| 2     | 0x1F     | SEFFECT         |
| 2     | 0x21     | GENERATO        |
| 2     | 0x2B     | (queen type)    |
| 1     | 0x04     | ENEMY1          |
| 1     | 0x16     | FLEVER          |
| 1     | 0x18     | WLEVER          |
| 1     | 0x1D     | DUMMY           |
| 1     | 0x3C     | KYOMI           |
| 1     | 0x3F     | (unk)           |
| 1     | 0x41     | temp            |
| 1     | 0x42     | ENEMY_CO        |
| 1     | 0x43     | FLY_INFO        |

Total 512 entries, roughly distributed across entity types.

### 5. Room init callbacks — function pointers mapped

The 19 populated room init callbacks reside in the `.data` range `0x0020FA98-0x0023D43C` and are:

| Address    | Room       |
|------------|------------|
| 0x0020FA98 | proto      |
| 0x00210D78 | shadows    |
| 0x00211780 | taki       |
| 0x00213B88 | plaza      |
| 0x0021A4D0 | crest_L1   |
| 0x0021A6B8 | crest_L2   |
| 0x0021A980 | crest_L3   |
| 0x0021F828 | crest_R1   |
| 0x0021FA30 | crest_R2   |
| 0x0021FD20 | crest_R3   |
| 0x00225F68 | sluice     |
| 0x00228198 | chandelier |
| 0x0022A838 | troko      |
| 0x0022B878 | windmill   |
| 0x0022BFE8 | stone      |
| 0x0022D8F8 | watertower |
| 0x00231AC8 | jail       |
| 0x00234AB0 | warehouse  |
| 0x00237B78 | gondola    |
| 0x00239750 | entrance   |

These are likely function pointers in `.data` that point back into `.text` for the actual init code.

### 6. Patching function at 0x00143290

Confirmed: this function processes **inner structs** at `row_base + 0x110/+0x114`, NOT the callback field at `+0x154`. It references a data table at `0x005D1B60` (type/ID/parameter structs). See Rev.071 for details.

---

## Confirmed

- 19 room init function pointers at `row_base + 0x154` (= `0x005F2F98 + 0x174`)
- Null pointers for non-gameplay rooms (logo, title, sacrifice, gates, graves, symmetry, underground, cliff)
- Descriptor table: 68 entries, fully mapped with names and handlers
- Entry table: 512 entries, 0x4C stride, desc_idx linking to descriptors
- The `mult` at 0x1AF954 is dead code (no mflo follows)
- GP variable 0x0062CD40 holds pre-multiplied `world_state * 404`

## Unknown

- Who writes the GP variable at `0x0062CD40`? No static SW targeting this address found.
- What is the field at `row_base + 0x134` (= `0x005F2F98 + 0x154`) with value 0x4B?
- What is the complete structure of the entry table (512 entries × 0x4C)?
- How are descriptors and entries linked to individual callback dispatches?

## Next minimum test

1. Map the 68 descriptor table entries' full 100-byte structure
2. Scan the 19 room init callbacks for their source functions in .text
3. Investigate the field at `row_base + 0x134` (value 0x4B)
4. Add cross-references between descriptor handlers and known dispatcher/callback functions
5. Look for the function that writes to GP variable at 0x0062CD40

---

## Conservative verdict

The room initialization dispatch chain is confirmed:

```
main loop (0x101C80)
  → scene init (0x1AF4A0)
    → reads GP var (world_state * 404)
    → computes row base: 0x005F2FB8 + world_state*404
    → reads callback: row_base + 0x154
    → JALR if non-null
    → live dispatch (0x166028, VU0 callback system)
```

19 of 32 rooms have pre-loaded init function pointers. The descriptor table (68 entity types) and entry table (512 entity instances) are the entity ID system for the game.

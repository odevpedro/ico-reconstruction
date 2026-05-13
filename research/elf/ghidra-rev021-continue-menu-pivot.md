# rev.021 - Continue Menu String Deception + Static Call Graph Deep Dive

## Date
2026-05-13

## Critical Discovery: No ASCII "Continue" Menu Text

The long search for a "Continue" ASCII string in the menu was based on a false premise:
the Continue/Yes/No menu text in ICO is NOT stored as ASCII text in the ELF data section.
It is rendered as **pixel art inside TM2 texture files** (TIM2 image format).

### Evidence

The string at VA `0x00555db6` was misidentified as "Continue" but actually reads:
```
"ContinueAnimation:illegal Animation No."
```
This is a **debug/error assertion string** (animation validation failure message),
not the menu text option.

The actual menu textures referenced in the ELF data section:
- `yesno_p1.tm2` through `yesno_p6.tm2` (VA `0x0062372d`) - Yes/No button textures
- `conti_p*.tm2` or similar - Continue menu textures (referenced in PAC files)

This explains why all previous breakpoints on ASCII string references hit zero times
during the death/continue flow: the code that draws the Continue menu never references
ASCII text for the menu options themselves.

## Key Functions Analyzed

### 0x0012a710 / 0x0012a618 - ContinueAnimation Error Logger

| Address | Role |
|---------|------|
| `0x0012a618` | Function entry (probable start after prologue search) |
| `0x0012a710` | Internal address referencing "ContinueAnimation" error string |
| `0x0013a0f8` | Called function - 247 callers, display/rendering setup |
| `0x001a6e28` | Called function - string formatting |

**Purpose**: This function is a **display configuration/validation routine** that:
- Loops through game objects via vtable dispatch
- Loads and validates TM2 textures
- Logs animation errors with debug strings

It is NOT the Continue menu trigger.

**Callers of 0x0012a618 (4 total)**:
- `0x00198710` - called by 8 functions in 0x0019xxxx region
- `0x001d37c8` - entity state dispatcher (sole vtable-based caller)
- `0x001ebc10` - display manager path
- `0x001ebcd0` - display manager path

### 0x001d37c8 - Entity State Dispatcher (Jump Table Based)

This function is a **jump table dispatch** on entity state ID:
```asm
0x001d37e4:  lw      $2,348($18)      ; load entity from $18+0x15c
0x001d37e8:  lw      $19,2048($2)     ; load vtable pointer from entity+0x800
0x001d37ec:  addiu   $17,$19,0x40     ; $17 = vtable + 0x40
0x001d37f0:  lw      $3,8($17)        ; load state ID from vtable+0x48
0x001d37f4:  sltiu   $2,$3,5          ; bounds check: state ID < 5
0x001d37f8:  beq     $2,$0,target     ; out of bounds?
0x001d37fc:  lui     $2,0x62          ; jump table base: 0x00628fb0
0x001d3800:  sll     $3,$0,2          ; $3 = 0 (shift of $0)
0x001d3804:  addiu   $2,$2,-28752     ; 0x00628fb0
0x001d3808:  addu    $3,$3,$2         ; $3 = &table[0]
0x001d380c:  lw      $4,0($3)          ; load function pointer
0x001d3810:  jr      $4               ; jump to handler
```

**Jump table at `0x00628fb0`**: entries are not standard function addresses.
These look like **compressed state indices** or **relative offsets**, not absolute
JAL targets. The function handles 5 states (indices 0-4) through this table.

**Called JAL targets**:
- `0x001f2148` - state handler 0
- `0x001d2538` - state handler 1 (sets state to 1, calls 0x001d29b8)
- `0x001d2540` - state handler 2 (calls 0x001d29b8)
- `0x00105f00` - state handler 3+ (camera/system call)

**Sole caller**: `0x001d3a30` - this function has **ZERO static callers**, confirming
it is invoked exclusively through a function pointer table (vtable dispatch at runtime).

### 0x001d3a30 - Entity Update Loop (Vtable-Only Caller)

This function has **no static call sites** (found 0 JAL instructions targeting it).
It is called through a vtable entry at `entity+0x808` (offset +0x8 within the vtable
at entity+0x800).

Disassembly pattern:
```asm
0x001d3a30:  lw      $3,348($17)      ; load entity from $17+0x15c
0x001d3a54:  lw      $18,2048($3)     ; load vtable from entity+0x800
0x001d3a58:  lw      $2,0($18)        ; load vtable[0] (type check)
0x001d3a5c:  beq     $2,$19,target    ; type mismatch?
0x001d3a70:  jal     0x001d29b8       ; call handler
0x001d3a88:  jal     0x001d2bf0       ; call handler
0x001d3ac0:  jal     0x001c05a8       ; state transition?
0x001d3ae4:  jal     0x00102858       ; animation system
0x001d3b04:  jal     0x001d37c8       ; recursive state dispatch
```

**Pattern**: This is the **main entity update loop**. It reads entity+0x800 vtable,
dispatches through vtable[0] (type), then vtable[1] (state handler), and
recursively calls 0x001d37c8 for state-based behavior updates.

## 0x0019xxxx Cluster Analysis (State Machine Region)

60 unique functions in the 0x00190000-0x001FFFFF range call the state resolver
`0x0013eb50`. Top functions by call frequency:

| Function | Calls to 0x0013eb50 |
|----------|---------------------|
| `0x001e29e8` | 6 |
| `0x001921f8` | 2 |
| `0x001925e0` | 4 |
| `0x001bc870` | 4 |
| `0x00199f80` | 4 |
| `0x00199c30` | 3 |
| `0x001e3fc8` | 3 |
| `0x00197b30` | 2 |
| `0x00198338` | 2 |
| `0x0019ae98` | 2 |

The `0x0019xxxx` cluster is the **entity behavior/state machine region**. This is where
gameplay logic (enemy AI, character actions, event triggers) is implemented.

### 0x00199c30 - Entity Behavior Handler (0 Static Callers)

This function has **zero static callers** - it is also vtable-dispatched. Full disassembly
shows it:
- Calls `0x0013eb50` with state ID 0x35 (3 times)
- Calls `0x0013ebe0` (sister function, 2 times)
- Calls `0x00203aa0` - entity query/retrieval
- Uses entity+0x800 (vtable) and entity+0x15c (entity pointer)
- Has floating-point math (lwc1, mtc1, cvt.w.s)
- References data at `0x0056c610` (string area near `conti_p1`)
- Sets state via `xori` (0x25 mask) - likely state flag operations
- Calls `0x00165f88`, `0x001a6a30`, `0x00118a68`, `0x0013eae8`, `0x00198678`

### 0x00199f80 - Entity Behavior Handler (0 Static Callers)

Also **zero static callers** - vtable-dispatched. Full disassembly shows:
- Calls `0x00203aa0` - entity query
- Calls `0x0013eb50` with state IDs 0x2e, 0x34, 0x35 (4 total calls)
- Calls `0x0013ebe0` (sister function)
- Uses entity offsets +0x15c (entity), +0x800 (vtable), +0x4a0 (model), +0x4ac (float), +0x4cc (state/counter)
- Checks global at `0x005690a0` (beq $3, $2, 0x25)
- Sets bit 0 at `0x0056a3cc` (`andi $2,$2,0x1`)
- Calls `0x0019ba60`, `0x001a6a30`, `0x00104508`, `0x00106b70`, `0x0017e5b0`
- Has complex branching for animation/callback handling

## Shadow String References (Not Yorda Capture Related)

All "shadow" references in the ELF data section are about:
- Shadow rendering/blending settings ("Shadow Blend 1/64", "Shadow Color BGR")
- Shadow tool debug strings
- Shadow model files (bridge_shadow, curtain_shadow, shadow_test.bga)
- Deja shadow cage animations (deja_shadow_cage_c5-c10)
- Shadow model loading ("Illegal Shadow ID number", "ReadShadowModelFile")

None of these are related to the Yorda shadow capture mechanic.

## New Breakpoint Strategy (No ASCII Strings)

Given the discovery that menu text is in TM2 textures, the previous breakpoint
strategy targeting ASCII string references was fundamentally flawed for the
continue menu specifically.

### Recommended Breakpoints (Updated)

The most promising new candidates based on static analysis:

1. **`0x001d3a30`** - Entity update loop entry (but called via vtable, not JAL)
   - Set as WRITE breakpoint on `entity+0x808` containing `0x001d3a30`
   - OR set as READ breakpoint on entity pointer while Yorda is captured

2. **`0x001d37c8`** - Entity state dispatcher (has one static caller 0x001d3a30)
   - Break on entry when entity is Yorda or shadow creature

3. **Memory search for continue state variable**:
   - Search for integer that changes from 0 to 1 when death occurs
   - Set READ/WRITE breakpoints on the discovered address

4. **Vtable write analysis**:
   - 0x001d3a30 is written to vtable through `sw` instructions in entity creation
   - Find who allocates entities and writes vtable entries

5. **DATA.DF overlay analysis** (preferred for static-only approach):
   - Extract the continue-menu overlay segment from DATA.DF
   - Apply relocations and analyze the menu state machine
   - This is likely where the actual Yes/No flow lives

### What NOT to breakpoint (Deprioritized)

| Address | Reason |
|---------|--------|
| `0x0012a710` | Debug/error string only - not menu text |
| `0x0011a520` | Too generic - runs during New Game menu |
| `0x0013a0f8` | Display setup - 247 callers, too broad |
| `0x00555db6` | Debug assertion string, not menu option |

## Call Graph Summary

```
Main Game Loop (0x0010xxxx)
    |
    +-- 0x00196520
    |       |
    |       +-- 0x00198710
    |               |
    |               +-- 0x0012a7a0 (entity link/unlink)
    |               +-- 0x0012a618 (TM2 display/validation)
    |                       +-- 0x0013a0f8 (rendering setup)
    |                       +-- 0x001a6e28 (string format)
    |                               +-- 0x0012a710 (references "ContinueAnimation" ERROR string)
    |
    +-- [VTABLE DISPATCH] --> 0x001d3a30 (entity update, 0 static callers)
            |
            +-- 0x001d29b8 (handler)
            +-- 0x001d2bf0 (handler)
            +-- 0x001c05a8 (state transition)
            +-- 0x00102858 (animation)
            +-- 0x001d37c8 (state dispatcher - jump table on entity+0x808)
                    |
                    +-- Reads entity+0x800 (vtable base)
                    +-- Reads entity+0x808+8 = entity+0x848 (state ID)
                    +-- Dispatch to 0x00628fb0[jump_table_index]
                    +-- Each handler handles states 0-4

Vtable-only functions (0 static callers, must be dispatch targets):
    0x001d3a30 -> called via entity+0x808
    0x00199c30 -> called via entity vtable
    0x00199f80 -> called via entity vtable
```

## Next Steps (No Runtime Required)

1. **Analyze the jump table at `0x00628fb0`** - understand what states 0-4 represent
   for different entity types
2. **Trace vtable writes** - find entity allocation code that writes function pointers
3. **Analyze `0x0019xxxx` top callers** (`0x001e29e8`, `0x00199f80`, `0x00199c30`)
   to map the entity behavior system
4. **Search for "capture" or "yorda"** references in code to find capture event handler
5. **Extract and analyze DATA.DF overlay** for menu-related code (overlay likely
   contains the actual continue menu game logic separate from ELF)
6. **Look for PAD/input handler** that reads controller during pause state

## Project Status

- **Continue menu text**: DISCOVERED to be in TM2 textures, not ASCII
- **Breakpoint strategy**: PIVOT required - stop targeting ASCII strings
- **Entity system**: MAPPED - vtable-based dispatch at entity+0x800
- **State machine**: MAPPED - 60 functions in 0x0019xxxx call state resolver
- **Vtable mechanics**: CONFIRMED - 0x001d3a30, 0x00199c30, 0x00199f80 all have zero static callers (vtable only)
- **Progress**: ~40% - still in reverse engineering phase
- **New direction**: Focus on DATA.DF overlay analysis for menu code, or memory search during gameplay
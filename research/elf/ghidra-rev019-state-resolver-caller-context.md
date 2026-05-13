# Ghidra State Resolver Caller Context Analysis - rev.019

## Date
2026-05-13

## Environment
- Target: `SCUS_971.13.elf` from local ICO USA copy
- Method: manual MIPS disassembly analysis (Ghidra not locally available)
- ELF load base: 0x00100000, file offset base: 0x1000

## Scope
Following the state-transition dispatch analysis from rev.018, this pass opens the context of the two confirmed caller functions of `0x0013eb50` and traces the return value consumption to understand how state IDs map to gameplay state.

## Direct Caller Functions of 0x0013eb50

### Function 1: 0x00199f80 (parent of 0x0019a138 and 0x0019a144)

**Function prologue:**
```
0x00199f80: addiu $sp,$sp,-0x0130
```

**Behavior:**
- Stack frame: 0x130 bytes
- Loads entity/object from `0x015c($3)` - indirect pointer dereference pattern
- Calls `0x00203aa0` (unknown - likely file or scene loader)
- Checks condition against `0x90a0($gp)` (global state flag)
- Calls `0x0013eb50` with ID `0x2e` (before 0x9a138 context)
- After returning from `0x0013eb50`, the code at `0x0019a000` loads `0x015c($2)` which is the object pointer - the return from `0x0013eb50` is used as an array index to look up a field within an object structure
- Then calls `0x0013eb50` again with ID `0x35` at `0x0019a010`
- The two consecutive calls at `0x0019a138` and `0x0019a144` with IDs `0x34` and `0x35` are part of the same logical block - this is a state ID pair being resolved in sequence
- Function returns via `jr $2` - using v0 as return target (tail-call pattern or continuation)
- Calls `0x0013ebe0` (sister function to `0x0013eb50`) at `0x0019a048` with a different object member at offset `0x0610`

**State ID context observed:**
- ID `0x2e`: 46 in decimal - appears in conditional branch context before 0x34/0x35
- ID `0x34`: 52 in decimal
- ID `0x35`: 53 in decimal - both in same function block, sequential

**Interpretation:** This function is a state dispatcher that resolves two consecutive state IDs (0x34, 0x35) by looking up object fields. The `+0x800` offset used (`lw $2,0x0800($3)`) suggests a fixed structure layout within entity objects. The use of `0x0610` for the sister function call (`addiu $20,$21,0x0610`) confirms the entity structure has multiple indexed sub-objects.

### Function 2: 0x0017bb98 (parent of 0x0017bd38)

**Function prologue:**
```
0x0017bb98: addiu $sp,$sp,-0x00f0
```

**Behavior:**
- Stack frame: 0xf0 bytes
- Uses VU/coprocessor operations heavily (c1 instructions with funct=0x34, 0x27, 0x1c, 0x01)
- Loads from `0x0020($17)`, `0x0024($17)`, `0x0028($17)`, `0x002c($17)` - array of VU floats at offsets 0x20, 0x24, 0x28, 0x2c from $17
- Uses GP-relative addressing pattern with `0x4248($gp)` - multiple floating-point constant loads
- Calls `0x00104508` at `0x0017bbf0` - render or transformation function
- At `0x0017bc14`: `sltiu $2,$3,0x0009` followed by `beq $2,$0,target` - checks if value is less than 9
- Calls `0x0013eb50` with ID `0x11` at `0x0017bd38`

**State ID context:**
- ID `0x11`: 17 in decimal

**Interpretation:** This function is part of a rendering or animation system that deals with VU float operations. The state ID `0x11` is queried in the context of float/comparison operations and conditional branching. This suggests the state table might be linked to animation, camera, or visual state rather than purely gameplay logic.

## Return Value Analysis

The return value from `0x0013eb50` is placed in v0 ($2) after the `jal`. The callers use this return value as:

1. **Array/index into object structure**: In `0x0019a000`, the return is used as `lw $3,0x015c($2)` - dereferencing an object pointer at index equal to the resolved state
2. **Return target for continuation**: At `0x0019a018` and `0x0019a050`, `jr $2` is used - the function returns via the state resolver result, suggesting this is part of a state machine or continuation chain
3. **Conditional branch input**: At `0x0017bc18`, the return appears to feed into a conditional branch path

## Key Findings

1. **0x0013eb50 is called from 150+ locations** - it is a central resolver used throughout the codebase, not a death-menu-specific function. This makes it a high-value anchor but also means breakpoints on it will fire constantly.

2. **The function operates on entity objects**: The `+0x15c`, `+0x800` offsets and the pattern of loading entity pointers from registers suggests this is part of the entity/actor system.

3. **State IDs observed:**
   - `0x11`: 17 decimal - used in VU/floating-point context (0x0017bd38 caller)
   - `0x2e`: 46 decimal - in file/loading context (0x00199f98 caller)
   - `0x34`: 52 decimal - sequential with 0x35 (0x0019a138 caller)
   - `0x35`: 53 decimal - sequential with 0x34 (0x0019a144 caller)

4. **Sister function 0x0013ebe0**: Called from a similar number of locations with the same instruction encoding pattern. Both functions likely form a pair for reading/writing state data or operating on two different state tables.

5. **No direct callers of the callers were found via jal**: The upstream callers of 0x0019a138 and 0x0017bd38 are not found via simple jal pattern matching - they are likely called via:
   - Function pointer tables
   - Indirect register-based calls (jalr)
   - Or their entry points are not reachable via the jal instruction pattern

## Next Validation Step

The most productive path forward is NOT more disassembly of this function, but rather:

1. **PCSX2 memory breakpoint on the global state flags** `DAT_006321c0` and `DAT_00633ca0` - if these globals can be monitored, the state transitions become observable

2. **PCSX2 execution breakpoint on 0x00199f80** - this function is more isolated than 0x0013eb50 itself and will fire less frequently. It is the logical entry point for the state ID pair 0x34/0x35.

3. **PCSX2 execution breakpoint on 0x0017bb98** - this function is the logical entry point for ID 0x11 and involves VU operations, suggesting it is tied to a specific animation or camera state.

4. **Look for the global table structure** at 0x006a93d0 (used when `DAT_006321c0 == 0` for IDs < 0x43) - this is a flat lookup table that could be examined for content patterns.

## Alternative Approach: Look for State ID constants in data section

Given that 0x0013eb50 handles IDs up to 0x43 in the fast path and beyond in the slow path, the state IDs could be:
- Animation IDs
- Scene/room IDs
- Enemy/object type IDs
- UI state IDs
- Event trigger IDs

To determine which, the next step should be to examine the data referenced by the state tables (0x006a93d0 and 0x00633ca0) for recognizable patterns.

## Notes

This revision is metadata-only:
- no extracted ELF payloads
- no copyrighted asset content
- observations derived from raw MIPS instruction patterns and known address context
- findings represent inference from disassembly, not confirmed behavior
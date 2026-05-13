# rev.020 — UI String Context and Caller Analysis

## Date
2026-05-13

## Scope
Automated analysis of UI string references and caller context without runtime testing. Goal: document known string locations and their code references to understand the Continue/Yes/No menu path.

## UI String Locations

### String at VA 0x005539a1
- **Value**: `"--- loop continues infinitely ."`
- **Interpretation**: This is likely the `continues` string used in the Continue menu (Japanese text translated to English approximation)
- **ELF offset**: 0x4e39a1
- **Code references**: None found via lui pattern scan

### String at VA 0x005551f0
- **Value**: `pac_continueTag` (null-terminated)
- **ELF offset**: 0x4e51f0
- **Code references via lui pattern**:
  - `0x00116f5c`
  - `0x00117310`
  - `0x0012850c`
  - `0x001310ec`
- **Interpretation**: Tag/marker string used to reference Continue menu state or object

### String at VA 0x00555db6
- **Value**: `Continue` (null-terminated)
- **ELF offset**: 0x4e5db6
- **Code references via lui pattern**:
  - `0x0012a710`
- **Interpretation**: Direct ASCII string for the menu option

## Function 0x0011a520 Analysis (pac_continueTag loader)

### Function Signature
- **Entry**: 0x0011a520
- **Stack frame**: -0x60 bytes
- **Called by**: 0x0011a794 (single caller)

### Disassembly Summary

```asm
0x0011a520: addiu $sp,$sp,-0x0060  ; prologue
0x0011a524: lui   $2,0x0067        ; load upper 0x67xxxxxx
0x0011a534: jr    $5               ; return via $5 (continuation pattern)
0x0011a540: addiu $18,$2,0x2fd0    ; compute 0x00672fd0 in $18
0x0011a550: ori   $17,$17,0xffff    ; mask $17
0x0011a554: lw    $7,0x002c($18)   ; load from computed address
0x0011a558: lw    $6,0x0028($18)
0x0011a568: bne   $2,$3,0x0011a5b0  ; conditional branch
0x0011a570: lui   $4,0x0055        ; *** references string section ***
0x0011a578: jal   0x001a6e28        ; call to 0x001a6e28 (string operations)
0x0011a57c: addiu $4,$4,0x51f0      ; *** completes string VA 0x005551f0 ***
0x0011a580: lui   $16,0x0055       ; *** references string section ***
0x0011a588: addiu $16,$16,0x4fd0
0x0011a58c: jal   0x001ad768       ; call to function
0x0011a5a0: jal   0x00263ff0       ; call to function
0x0011a5b8: jal   0x0011a2a8        ; recursive/local call
0x0011a5f8: jal   0x0011a338        ; recursive/local call
0x0011a658: sw    $3,0xb358($28)   ; store to global (GP-relative)
0x0011a65c: jr    $19               ; return via $19
0x0011a660: addiu $29,$29,0x0060   ; epilogue
```

### Key Observations

1. **Continuation pattern**: Function uses `$5`, `$16`, `$19` as return targets (not standard `jr $ra`). This suggests the function is part of a state machine or continuation chain.

2. **String construction**: Lines 0x0011a570-0x0011a57c build the `pac_continueTag` VA:
   - `lui $4,0x0055` loads upper 16 bits of 0x0055xxxx
   - `addiu $4,$4,0x51f0` adds lower 16 bits to get 0x005551f0

3. **Multi-path function**: The `bne $2,$3,0x0011a5b0` creates two execution paths:
   - Path A: when $2 != $3 (goes to 0x0011a5b0)
   - Path B: when $2 == $3 (falls through)

4. **Global state modification**: `sw $3,0xb358($28)` stores to GP-relative address, modifying a global counter/state

5. **Recursive calls**: Function calls `0x0011a2a8` and `0x0011a338` - these are nearby functions, suggesting a utility module

## Caller Function 0x0011a794

- Calls `0x0011a520` via `jal`
- Only caller of the Continue-tag loader
- This function likely handles the Continue menu display or state transition

## State Resolver Call Clusters (146 callers of 0x0013eb50)

| Cluster (range) | Count | Notes |
|-----------------|-------|-------|
| 0x0019xxxx | 38 | Most calls - likely entity/actor system |
| 0x0017xxxx | 19 | Second cluster - game logic |
| 0x001exxxx | 14 | Third cluster |
| 0x001fxxxx | 15 | Fourth cluster |
| 0x0014xxxx | 11 | Fifth cluster |
| 0x0015xxxx | 11 | Sixth cluster |
| 0x0016xxxx | 11 | Seventh cluster |

The concentration in 0x0019xxxx suggests that region handles most state transitions in the game.

## Entity Structure Offsets Documented

From previous analysis, confirmed entity structure offsets:

- **+0x15c**: Entity pointer dereference
- **+0x800**: State-indexed field (used after 0x0013eb50 return)
- **+0x610**: Sister function offset (used with 0x0013ebe0)
- **+0x4a0**: Model/animation pointer
- **+0x4ac**: Float value (VU-related)
- **+0x4cc**: State/counter value

## Next Steps (No Runtime Required)

1. **Document the entity structure** with confirmed offsets
2. **Analyze function 0x0011a338** (called from 0x0011a520) - likely part of same utility module
3. **Map the string reference chain**: 0x00116f5c, 0x00117310, 0x0012850c, 0x001310ec all reference `pac_continueTag` area
4. **Analyze 0x0012a710** which references the `Continue` string directly
5. **Build call graph** for the 0x0019xxxx cluster (38 callers) to understand entity state machine

## Notes

This revision is metadata-only:
- Observations from static analysis
- No extracted copyrighted content
- All data derived from ELF structure and instruction patterns
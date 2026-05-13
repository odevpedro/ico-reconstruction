# Proof of Concept - rev.015 - String Extraction

## Date
2026-05-12

## Proof of Concept Success

Successfully extracted readable strings from SCUS_971.13.elf.

## Findings

### Camera Data Files (.gcm)
The ELF contains references to camera data files:

| Address | String |
|---------|--------|
| 0x00291a50 | camdata/sacrifice.gcm |
| 0x00291a70 | camdata/boss.gcm |
| 0x00291a90 | camdata/boss.gcm |
| 0x00291ab0 | camdata/sacrifice.gcm |
| 0x00291ad0 | camdata/13c4demo.gcm |
| 0x00291af0 | camdata/25a4demo.gcm |
| 0x00291b10 | camdata/NULL.gcm |
| 0x00291b30 | camdata/athletic.gcm |
| ... | ... |

### Technical Details
- **Executable:** SCUS_971.13.elf
- **Total Functions:** 3426
- **String Data Location:** Around 0x00291000-0x0029c000
- **File Format:** GCM (PlayStation 2 graphics/camera data)

## Proof of Concept Validation
1. ✅ ELF successfully loaded in Ghidra
2. ✅ String data extracted
3. ✅ Camera files identified
4. ✅ No modification yet - extraction only

## Next Steps for Modification
1. Identify a non-critical string
2. Modify string in binary
3. Test if game runs with modified binary

## Project Progress
- Total Revisions: 15
- Functions identified: 3426
- Top functions analyzed
- Strings accessible

## Conclusion
The reconstruction infrastructure is working. We can:
1. Analyze the ELF structure
2. Identify functions and their relationships
3. Extract string data
4. Prepare for modifications

This is the foundation for future proof of concept modifications.
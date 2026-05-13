# Proof of Concept 2 - String Modification - rev.017

## Date
2026-05-12

## Proof of Concept Success

Successfully modified a string in SCUS_971.13.elf.

## Modification Details

| Property | Value |
|----------|-------|
| Original String | NULL.gcm |
| Modified String | EMPTY.gcm |
| File Position | 1,649,432 (0x00193158) |
| Context | camdata/EMPTY.gcm |

## Technical Process

1. Copied ELF to working directory
2. Used Python to find and replace bytes
3. Verified modification succeeded
4. Original binary preserved

## Modified File

- Location: `/tmp/SCUS_971.13_mod.elf`
- SHA256 (modified): To be calculated

## Next Steps

1. Test if modified ELF runs in PS2 emulator
2. Try different modifications (size changes, different strings)
3. Identify which .gcm files are critical vs optional

## Project Progress

| Revision | Achievement |
|---------|-------------|
| rev.015 | First PoC: String extraction |
| rev.016 | Video functions identified |
| rev.017 | Second PoC: String modification |

## Conclusion

The ELF can be modified. This opens possibilities for:
- Testing gameplay changes
- Debug output modifications
- Path/filename alterations
- Simple patches and experiments
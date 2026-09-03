# Unmatched Objects Priority

Date: 2026-05-23

## Objective

Rank the current residual PAL->USA objects so the next offline pass can target the highest-value, highest-probability cases first.

## Criteria

- Larger objects get higher impact weight.
- Objects with SRCFILE.TXT provenance get a recoverability bonus.
- Objects close to already solved MAIN.MAP neighbors get a neighborhood bonus.

## Top Candidates

| Rank | Object | Funcs | Priority | Recoverability | Impact | Source files | Nearest known |
|------|--------|-------|----------|----------------|--------|--------------|---------------|
| 1 | end.o | 52 | 169 | 59 | 110 | Basic.c | e3.o |
| 2 | box.o | 44 | 166 | 68 | 98 | debug.c | attackCheckBoundary.o;boy.o |
| 3 | girl_act.o | 60 | 148 | 8 | 140 | - | fieldCollision.o |
| 4 | st13c.o | 42 | 143 | 54 | 89 | debug.c | st13b.o |
| 5 | GifPacket.o | 38 | 140 | 59 | 81 | debug.c | GsBase.o |
| 6 | st10r.o | 28 | 125 | 67 | 58 | debug.c | st10l.o;st13a.o |
| 7 | particleEffect.o | 19 | 101 | 59 | 42 | debug.c | multiBgaManager.o |
| 8 | pool.o | 14 | 99 | 67 | 32 | debug.c | motionManager2.o;quaternion.o |
| 9 | way_util.o | 39 | 91 | 8 | 83 | - | script.o |
| 10 | st05c.o | 14 | 88 | 59 | 29 | debug.c | st05d.o |
| 11 | st09a.o | 14 | 87 | 59 | 28 | st01b.c | st10l.o |
| 12 | backStage.o | 8 | 84 | 67 | 17 | debug.c | StageManager.o;charFileManager.o |
| 13 | Primitive.o | 12 | 82 | 54 | 28 | debug.c | MicroCode.o |
| 14 | windField.o | 10 | 80 | 59 | 21 | debug.c | weapon.o |
| 15 | DisplayFont.o | 5 | 78 | 67 | 11 | debug.c | BgAnimation.o;DisplayList.o |
| 16 | st04a.o | 30 | 77 | 9 | 68 | - | st04b.o |
| 17 | deja.o | 6 | 72 | 59 | 13 | debug.c | e3.o |
| 18 | queen.o | 23 | 58 | 8 | 50 | - | stage_orient.o |
| 19 | gobj.o | 23 | 57 | 9 | 48 | - | gobj_dl.o |
| 20 | thread.o | 23 | 56 | 9 | 47 | - | shockdriver.o |

## Notes

- Residual objects ranked: 123
- Residual functions represented: 1185
- Objects with source provenance: 14
- Objects with a known neighbor within 5 positions: 123

## Next Step

Attack the top-ranked objects in order, but only with offline methods first: source-provenance joins, neighbor offset propagation, and multi-anchor validation.

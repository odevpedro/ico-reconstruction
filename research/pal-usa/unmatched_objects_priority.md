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
| 1 | girl_act.o | 60 | 148 | 8 | 140 | - | fieldCollision.o |
| 2 | end.o | 52 | 119 | 9 | 110 | - | e3.o |
| 3 | box.o | 44 | 115 | 17 | 98 | - | attackCheckBoundary.o;boy.o |
| 4 | st13c.o | 42 | 97 | 8 | 89 | - | st13b.o |
| 5 | way_util.o | 39 | 91 | 8 | 83 | - | script.o |
| 6 | GifPacket.o | 38 | 90 | 9 | 81 | - | GsBase.o |
| 7 | st04a.o | 30 | 77 | 9 | 68 | - | st04b.o |
| 8 | st10r.o | 28 | 75 | 17 | 58 | - | st10l.o;st13a.o |
| 9 | queen.o | 23 | 58 | 8 | 50 | - | stage_orient.o |
| 10 | gobj.o | 23 | 57 | 9 | 48 | - | gobj_dl.o |
| 11 | thread.o | 23 | 56 | 9 | 47 | - | shockdriver.o |
| 12 | st00a.o | 22 | 55 | 9 | 46 | - | st01b.o |
| 13 | st03t.o | 21 | 53 | 9 | 44 | - | st02a.o |
| 14 | st25a.o | 20 | 52 | 9 | 43 | - | script.o |
| 15 | particleEffect.o | 19 | 51 | 9 | 42 | - | multiBgaManager.o |
| 16 | op.o | 15 | 50 | 16 | 34 | - | e3.o;st01b.o |
| 17 | pool.o | 14 | 49 | 17 | 32 | - | motionManager2.o;quaternion.o |
| 18 | st08b.o | 16 | 49 | 16 | 33 | - | st07a.o;st10l.o |
| 19 | act.o | 18 | 49 | 9 | 40 | - | adpcm_init.o |
| 20 | staticBlur.o | 17 | 49 | 8 | 41 | - | streamMotionManager.o |

## Notes

- Residual objects ranked: 123
- Residual functions represented: 1185
- Objects with source provenance: 0
- Objects with a known neighbor within 5 positions: 123

## Next Step

Attack the top-ranked objects in order, but only with offline methods first: source-provenance joins, neighbor offset propagation, and multi-anchor validation.

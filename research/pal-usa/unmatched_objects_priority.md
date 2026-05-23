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
| 1 | end.o | 52 | 172 | 62 | 110 | Basic.c | deja.o;st00a.o |
| 2 | girl_act.o | 60 | 148 | 8 | 140 | - | fieldCollision.o |
| 3 | particleEffect.o | 19 | 101 | 59 | 42 | debug.c | particleLayout.o |
| 4 | way_util.o | 39 | 91 | 8 | 83 | - | script.o |
| 5 | windField.o | 10 | 88 | 67 | 21 | debug.c | weapon.o;windManager.o |
| 6 | st05c.o | 14 | 83 | 54 | 29 | debug.c | st04r.o |
| 7 | backStage.o | 8 | 76 | 59 | 17 | debug.c | charFileManager.o |
| 8 | st04a.o | 30 | 76 | 8 | 68 | - | st04c.o |
| 9 | DisplayFont.o | 5 | 70 | 59 | 11 | debug.c | BgAnimation.o |
| 10 | st03t.o | 21 | 59 | 15 | 44 | - | st00a.o;st04c.o |
| 11 | queen.o | 23 | 59 | 9 | 50 | - | lightning.o |
| 12 | thread.o | 23 | 56 | 9 | 47 | - | gobj.o |
| 13 | st08b.o | 16 | 50 | 17 | 33 | - | st08a.o;st09a.o |
| 14 | gamesys.o | 19 | 48 | 9 | 39 | - | haveParentSimpleObj.o |
| 15 | act.o | 18 | 46 | 6 | 40 | - | way_llf.o |
| 16 | st17a.o | 17 | 44 | 9 | 35 | - | st13d.o |
| 17 | op.o | 15 | 43 | 9 | 34 | - | st00a.o |
| 18 | Shadow.o | 11 | 41 | 8 | 33 | - | Primitive.o |
| 19 | act_bird.o | 12 | 36 | 9 | 27 | - | gv.o |
| 20 | mv_videodec.o | 9 | 35 | 17 | 18 | - | mv_strfile.o;mv_vobuf.o |

## Notes

- Residual objects ranked: 81
- Residual functions represented: 715
- Objects with source provenance: 6
- Objects with a known neighbor within 5 positions: 77

## Next Step

Attack the top-ranked objects in order, but only with offline methods first: source-provenance joins, neighbor offset propagation, and multi-anchor validation.

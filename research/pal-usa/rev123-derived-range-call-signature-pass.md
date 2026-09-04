# Rev.123 — derived-range call-signature pass

## Scope

Static PAL→USA analysis only. This extends the Rev.121 candidate generator;
it does not modify `pal_usa_symbol_map.csv` and does not promote a seed.

## Evidence tiers

- **Direct seeds:** 362 verified `raw_sha1`, `op_seq_hash`, or `norm_sha1`
  PAL→USA pairs.
- **Derived anchors:** 443 function rows that have both an `op_seq_hash` plus
  size match and belong to a range currently classified `HIGH`, with at least
  two validated functions in that range.

The second tier is deliberately weaker: the function match is constrained by
an object-relative offset. It is allowed only to generate a call-signature
candidate, never to promote a PAL→USA address by itself.

## Method

`recover_call_signatures.py --include-high-range-anchors` keeps the direct
and derived counts separate, writes a separate output filename, and still
requires a same-ordinal sequence of at least two resolved `jal` targets plus
two functions agreeing on one object offset before any later review.

## Result for the priority objects

| Object | Functions | Functions with at least two resolved callees | USA function candidate | Shared offset |
|---|---:|---:|---:|---:|
| `girl_act.o` | 60 | 3 | 0 | 0 |
| `end.o` | 52 | 0 | 0 | 0 |
| `GifPacket.o` | 38 | 0 | 0 | 0 |

The three `girl_act.o` functions with two resolved callees are
`motGirlHand200` (`0x00168960`), `subGirlBrain_Attract` (`0x0016B4D0`), and
`ClipTwinVector` (`0x0016E0C0`). None has a matching USA call signature under
this method. Their presence is coverage information only, not a mapping.

The generated per-function counts and report are retained as
`call_signature_candidates_with_high_range_anchors.csv` and
`call_signature_recovery_with_high_range_anchors.md`.

## Consequence

No direct seed was added, no derived anchor was upgraded, and no target-object
function was mapped. The next static improvement needs an independent source
of callee identity (for example, direct raw/op matches in additional objects
or validated cross-version function boundaries), not a looser call-signature
threshold.

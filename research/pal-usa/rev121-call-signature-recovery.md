# Rev.121 — bounded PAL→USA call-signature recovery

## Scope

Static PAL→USA analysis only. The method reads the local PAL `MAIN.MAP` and
`SCES_507.60`, the local USA ELF, USA fingerprints, and only the existing
reliable PAL→USA seeds (`raw_sha1`, `op_seq_hash`, and `norm_sha1`). No
runtime capture was used and no symbol-map row was modified.

## Method

`tools/pal_usa_reconcile/recover_call_signatures.py` extracts `jal` targets
from each PAL function and resolves them only when the target already has a
reliable PAL→USA seed. A USA candidate must contain the same resolved target
sequence at the same call ordinals. A result remains a candidate: it needs
both a later structural check and at least two functions sharing one object
offset before it can become a seed.

This deliberately excludes fuzzy names, `same_va` fallbacks, and a single
resolved call. It avoids using a weak caller/callee relation as a new address
mapping.

## Result

The 362 current reliable callee seeds do not give sufficient coverage in the
three priority objects:

| Object | PAL functions | Functions with at least two resolved callees | Seed/candidate promoted |
|---|---:|---:|---|
| `girl_act.o` | 60 | 0 | none |
| `end.o` | 52 | 0 | none |
| `GifPacket.o` | 38 | 0 | none |

The generated per-function counts are retained in
`research/pal-usa/call_signature_candidates.csv`; the concise run result is
`research/pal-usa/call_signature_recovery.md`. These are negative results,
not evidence that the PAL functions lack calls in general: their callees are
not among the current reliable seed set.

## Consequence

No PAL→USA map, object offset, source provenance, or function name was
promoted by this revision. The next useful static prerequisite is to expand
the reliable callee seed set outside these objects, then rerun this bounded
method and independently verify any shared-offset result.

# Rev.120 — SRCFILE provenance audit and priority-object recovery

## Scope

Static PAL→USA analysis only. Inputs are the local PAL `MAIN.MAP`,
`SRCFILE.TXT`, and `SCES_507.60`, plus the local USA ELF and existing
fingerprint sets. No runtime session was used.

## Confirmed correction: `SRCFILE.TXT` is not address-only provenance

`SRCFILE.TXT` contains 4,943 address/source records, but its address space
cannot be inferred safely from its presence in the PAL image alone.

- Direct `MAIN.MAP` PAL-address lookup produced 123 address collisions and
  only one exact function-name agreement.
- PAL-symbol lookup produced 262 address collisions and 140 exact-name
  agreements.
- Address-only attachment therefore creates contradictory source ownership
  (for example, a `GifPacket.o` address resolving to a different GifPacket
  function name).

`join_srcfile.py` now clears prior source fields on every run and attaches
provenance only when both the address and normalized function name agree.
This is a confirmed filtering rule, not a conclusion about the original
address domain of every SRCFILE record.

## Priority-object results

| Object | PAL functions | Exact new anchors | Result |
|---|---:|---:|---|
| `girl_act.o` | 60 | 0 | No seed recovered |
| `end.o` | 52 | 0 | No seed recovered |
| `GifPacket.o` | 38 | 0 | No seed recovered |

The bounded recovery checked `raw_sha1` and `op_seq_hash` only at 13,822
pre-fingerprinted USA function starts. It found no target-object candidate
that validates by both structural sequence and function size.

Existing target labels in `pal_usa_symbol_map.csv` are not promoted: they are
mostly `fuzzy` candidates or `same_va` fallbacks, neither of which supplies an
independent, object-consistent seed. Neighbor offsets also do not establish a
shared offset across any of the three objects.

## Native-port check

After the mandatory toolchain pre-flight, `cmake --build native/build` and
`ctest --test-dir native/build --output-on-failure` passed 15/15 tests. The
current GIF packet bridge remains a host-side model; this revision does not
claim that `GifPacket.o` has been mapped or reconstructed byte-exactly.

## Next evidence needed

The next static method should compare call-target relationships of a PAL
function against already verified PAL→USA callee pairs, then require multiple
independent anchors within a single object. A candidate obtained that way must
still pass byte/structural validation before entering the seed set.

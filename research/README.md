# Research Notes

This directory is reserved for original research notes and subsystem observations.

Do not commit extracted game files or copyrighted content here. Notes should document what was observed, how it was observed, and what remains uncertain.

Suggested future organization:

```text
research/
  iso-layout/
  elf/
  external/
  assets/
  rooms/
  collision/
  animation/
  camera-events/
  actors/
```

Use `docs/research-methodology.md` as the standard for evidence labels and observation format.

## Current Orientation Notes

The current active technical focus remains the verified ELF callback/cloth
chain, especially the dispatcher at `0x001d37c8`, the callback at
`0x001d3a30`, and the registration path through `0x0013f7a8`.

Recent external/tooling notes should be read before starting new tooling work:

| Note | Purpose |
|---|---|
| `research/external/sotc-tooling-relevance-survey.md` | Evaluates `Fantaskink/SOTC` as a tooling/process reference for ICO. |
| `research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md` | Independently revalidates dispatcher/callback byte-level anchors with Rabbitizer. |
| `research/external/ico-splat-minimal-experiment.md` | Confirms a minimal `splat64[mips]` split path for the ICO USA ELF. |
| `research/external/ico-splat-promoted-ranges-experiment.md` | Confirms selected verified ranges can be promoted into separate asm files. |
| `research/external/ico-splat-adjacent-promoted-ranges-experiment.md` | Confirms the next adjacent verified ranges can also be promoted cleanly. |

Important caution:

- SOTC is useful as a workflow reference (`splat`, Rabbitizer/spimdisasm,
  Ninja/diff, SDK segmentation, compiler packaging).
- SOTC is **not** evidence for ICO gameplay or cloth semantics.
- Any tooling result must be validated against local ICO bytes before it can
  influence project conclusions.

The next external/tooling follow-up should continue promoting only verified
functions/ranges and should check boundaries before each promotion. Non-text/DVP
regions remain conservative `databin` blobs unless a dedicated overlay analysis
is requested.

The latest promoted-range follow-up is:

```txt
research/external/ico-splat-adjacent-promoted-ranges-experiment.md
```

It promoted `0x0013fc00`, `0x001d27a8`, and `0x001d3b28` while preserving full
ELF coverage. The result is useful as tooling progress, but it does not resolve
the `a1` source for `0x001d27a8`; that still needs runtime capture.

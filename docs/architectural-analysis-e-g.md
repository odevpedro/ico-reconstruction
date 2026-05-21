# Architectural Analysis E-G

> Revision: rev.100
> Status: Stable
> Evidence baseline: validated local documentation, byte-exact assembly set, and research notes already in this repository

## Summary

This analysis is the follow-up to `Architectural Analysis A-D`.

It does not introduce new runtime evidence. Its job is to formalize the
current architecture of the project after the validated `isysGObj*` / `iosOm*`
promotion work and the creation of the evidence index.

The current conservative conclusion is:

- the project now has a stable evidence chain for the core object/dispatch
  block;
- the repository is better served by explicit evidence control than by more
  narrative drift;
- runtime work still matters, but it is a separate phase with clear open gaps.

## E. Current Validated Architecture

| Layer | Current State | Evidence Level | Notes |
|------|---------------|----------------|-------|
| Narrative docs | Local-only, not tracked by git | Confirmed | `docs/historia.md` and `docs/prompt_persona_ico_reconstruction.md` stay out of the repo history |
| Evidence control | Explicit factual index exists | Confirmed | `docs/evidence-index.md` now records sources, tools, commits, artifacts, and gaps |
| Byte-exact assembly | `src/core/asm/` contains 36 exact helpers | Confirmed | The `isysGObj*` / `iosOm*` block is mechanically validated against the ELF |
| Earlier Path B set | 38/38 total functions exact | Confirmed | 26 via `.s`, 12 via C |
| Runtime halfword front | Fast path and second caller remain open | Inferred / Partial | `0x00166DFC` and `0x0016828C` still need direct runtime capture |
| Documentation sync | Backlog and architecture log are aligned | Confirmed | The current project state is now represented consistently across the main docs |

## F. Decision Matrix

| Option | Benefit | Cost | Verdict |
|--------|---------|------|---------|
| Continue static doc consolidation | Improves auditability and reduces drift | Low | Recommended |
| Promote more exact asm from the same style of work | Increases validated surface area without runtime dependency | Medium | Recommended when the next candidate is small and mechanically stable |
| Open new runtime work immediately | Can close open gaps | High | Only when an emulator session is available |
| Re-open blind string or asset swapping | Low chance of resolving the current gaps | High | Not recommended |
| Expand narrative history further | Improves readability, not evidence | Low | Allowed only if it stays clearly separate from the evidence index |

## G. Recommendation

The next safe architecture phase is a split-track model:

1. Keep the evidence index as the canonical control document.
2. Treat the narrative history as local-only and non-normative.
3. Use runtime only for the remaining open probes.
4. Use static work for documentation consistency, note consolidation, and
   exact assembly promotion when the target is mechanically small.

## Known Black Boxes

These remain unresolved and should not be renamed semantically without new evidence:

- `0x00166DFC` fast path;
- `0x0016828C` / `0x00168294` second caller context;
- field-level semantics inside the `isysGObj*` / `iosOm*` structures;
- any UI or gameplay meaning not directly supported by xref or runtime capture.

## Conservative Verdict

The project has moved from broad investigation into a structured split:

- exact bytes where the evidence is strong;
- explicit gaps where the evidence is incomplete;
- local narrative kept separate from the repository record.

That is the right architectural shape for the current phase.

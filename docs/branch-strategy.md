# Branch Strategy

## Main / Master — Decompilation and Reconstruction

This branch is the source of truth for the technical reconstruction of ICO.

It contains:

- byte-exact assembly reconstruction;
- C reconstruction when available;
- PAL→USA reconciliation;
- symbol/source-file mapping;
- runtime evidence;
- architecture documentation;
- scoring tools;
- verified research notes.

This branch must remain conservative and evidence-driven.

## Native Port — Experimental PC Port

The `native-port` branch exists to explore the long-term goal of making a native PC port possible.

It may contain:

- platform abstraction layers;
- PS2 API stubs;
- SDL/input experiments;
- filesystem replacement;
- rendering/audio/timing prototypes;
- native runtime scaffolding;
- port-readiness documentation.

This branch may move faster and include experiments, but it must not rewrite historical evidence from the decomp branch.

## Merge Direction

Preferred direction:

`main/master -> native-port`

Rare direction:

`native-port -> main/master`

Only merge back to `main/master` if the change improves decomp/reconstruction itself, such as:

- better headers;
- clearer struct definitions;
- better documentation;
- non-port-specific tools.

## Rule

Decomp proves what ICO was.

Native-port explores how ICO could run natively on PC.

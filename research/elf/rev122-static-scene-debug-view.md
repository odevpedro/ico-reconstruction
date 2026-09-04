# Rev.122 — Native static scene debug view

## Scope

`native-port` host-debug feature only. It consumes the existing semantic
`KanbanSceneLoader` and `IsysGObjRuntime`; it does not execute PS2 callbacks,
decode proprietary assets, or alter the byte-exact sources.

## Confirmed boundary used

The view follows the existing native model of the confirmed chain:

```text
SceneEntryRecord + SceneGObjDescriptor
  -> KanbanSceneLoader::initSceneGObj(sceneId)
  -> IsysGObjRuntime primary lists
  -> GIF placeholder command per created scene GObj
```

The ordered primary-list traversal reflects the semantic `isysGObj*` runtime
model. Every debug item exposes only direct model/ABI values:

- `sceneId` supplied to `initSceneGObj`;
- source `descriptorIndex`;
- raw `GObj+0x0C` as `gobj.type`;
- `listId`, `sortKey`, and host semantic-pool `handle`.

`gobj.type` is deliberately not renamed to a game-facing entity type. The
handle is a native semantic-pool identifier, not an original EE pointer.

## Host presentation contract

`renderStaticSceneDebugView` emits untextured GIF placeholders and sends the
associated label plus position to a caller-provided host label sink. The
current render backend has no text rasterizer, so labels are overlay metadata,
not ICO UI text. List bands make list membership visible; within each band,
objects retain the runtime list order.

## Explicit non-claims

- Static entry records are not assigned to rooms or world states by the USA
  fixture; therefore this feature selects a native `sceneId`, not a verified
  original world-state dataset.
- The placeholders have no ICO model, texture, material, transform, or world
  position.
- It does not claim visual fidelity or gameplay execution.

## Validation

`scene_loader_test` constructs two GObjs for scene `7` in lists `2` and `1`.
It verifies list-1 is presented first despite being allocated second; validates
the complete list-2 label; and validates the two GIF placeholder positions and
label-sink calls. Full native CTest passed 16/16.

## Next evidence needed

To choose a real original room/world state, correlate runtime
`init_scene_gobj` entry indices with the entry-table records. To add spatial
placement, recover only directly evidenced transform/coordinate fields from
that record path before connecting them to this debug view.

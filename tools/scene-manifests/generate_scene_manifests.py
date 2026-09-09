#!/usr/bin/env python3
"""Generate one native scene manifest per canonical room from extracted PAL assets.

For every room stNNN found in the unpacked PAL .p2o tree (the subfolder
`object/sdf/<room>/model/`), pick the DF that holds the largest model set for
that room (the canonical room DF), then materialize a host-side scene layout:

    native/assets/scene/rooms/<room>/
        <room>.manifest    SceneAssetStore manifest (scene id 0x0F)
        pieces/            the room's .p2o meshes (copied from <canonical DF>)
        texture/           the room's .tm2 textures (copied from <canonical DF>)

The manifest binds scene id 0x0F to the room's pieces purely as host test data
(the world_state-to-scene mapping is NOT confirmed; 0x0F is the dominant
world_state from the runtime capture). This seam is consumed by the native
SceneAssetStore/KanbanSceneLoader demo path.

Assembly here is metadata-only for the manifest; the .p2o/.tm2 bytes are copied
byte-for-byte from already-extracted local assets and remain gitignored
(*.p2o, *.tm2) — never committed.

Usage:
  generate_scene_manifests.py --p2o-root /tmp/pal-p2o-all --tm2-root /tmp/pal-unpacked \
      --out native/assets/scene/rooms [--room st02a] [--scene-id 0x0F] [--dry-run]

Writes:
  <out>/catalog.json           summary of generated rooms (metadata only)
  <out>/<room>/{manifest,pieces/,texture/}
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import sys
from pathlib import Path

DEFAULT_P2O_ROOT = Path("/tmp/pal-p2o-all")
DEFAULT_TM2_ROOT = Path("/tmp/pal-unpacked")
DEFAULT_OUT = Path("native/assets/scene/rooms")
DEFAULT_SCENE_ID = "0x0F"


def canonical_rooms(p2o_root: Path) -> dict[str, tuple[str, int]]:
    """Return {room: (canonical_df_name, p2o_count)} where the canonical DF is
    the one containing the room's largest model set."""
    rooms: dict[str, tuple[str, int]] = {}
    for d in sorted(p2o_root.iterdir()):
        if not (d.is_dir() and d.suffix.upper() == ".DF"):
            continue
        sdf = d / "object" / "sdf"
        if not sdf.is_dir():
            continue
        for room in sorted(p.name for p in sdf.iterdir()
                           if p.name.lower().startswith("st")
                           and (p / "model").is_dir()):
            cnt = len(list((sdf / room / "model").glob("*.p2o")))
            if cnt == 0:
                continue
            prev = rooms.get(room)
            if prev is None or cnt > prev[1]:
                rooms[room] = (d.name, cnt)
    return rooms


def copy_dir(src: Path, dst: Path, ext: str) -> int:
    """Copy files of one extension from src to dst. Returns copied count."""
    dst.mkdir(parents=True, exist_ok=True)
    n = 0
    for f in sorted(src.glob(f"*.{ext}")):
        shutil.copy2(f, dst / f.name)
        n += 1
    return n


def copy_textures(tm2_root: Path, canon_df: str, dst: Path) -> int:
    """Copy the union of .tm2 from every object/sdf/*/texture within the
    canonical DF (share-floor textures like st06a/st17a are referenced by the
    room's water/grass effects but live in sibling sdf subdirs)."""
    sdf_root = tm2_root / f"{canon_df}.DF" / "object" / "sdf"
    if not sdf_root.is_dir():
        return 0
    seen: set[str] = set()
    n = 0
    for sub in sorted(sdf_root.iterdir()):
        tex = sub / "texture"
        if not tex.is_dir():
            continue
        for f in sorted(tex.glob("*.tm2")):
            if f.name in seen:
                continue
            seen.add(f.name)
            shutil.copy2(f, dst / f.name)
            n += 1
    return n


_TM2_NAME = re.compile(rb"[A-Za-z0-9_]{2,32}")

def referenced_tm2_names(piece: Path) -> set[str]:
    """Approximate the material-name table a room piece will reference: scan
    the .p2o for material/path-like printable tokens (the room format stores
    `...texture\\<name>` backslash paths; some writers inline `<name>.tm2` or a
    bare material name). Return candidate names; the caller keeps only those
    that resolve to a real <name>.tm2 somewhere in the unpacked tree."""
    data = piece.read_bytes()
    names = {m.group(0).decode("ascii") for m in _TM2_NAME.finditer(data)}
    return set(
        n for n in names
        if re.fullmatch(r"[A-Za-z0-9_]{2,32}", n)
        and not n.startswith(("PS2O", "SUM", "OBJH"))
    )


def global_tm2_index(tm2_root: Path) -> dict[str, Path]:
    """Build {basename: first found path} over ALL unpacked texture dirs."""
    index: dict[str, Path] = {}
    for df in sorted(tm2_root.glob("*.DF")):
        sdf = df / "object" / "sdf"
        if not sdf.is_dir():
            continue
        for sub in sdf.iterdir():
            tex = sub / "texture"
            if not tex.is_dir():
                continue
            for f in tex.glob("*.tm2"):
                index.setdefault(f.name, f)
    return index


def fill_missing_textures(pieces_dir: Path, texture_dir: Path,
                          tm2_root: Path) -> int:
    """Copy any .tm2 referenced by the room's pieces but missing from the
    room's texture dir, resolving the name across every unpacked DF."""
    present = {f.name for f in texture_dir.glob("*.tm2")}
    index = global_tm2_index(tm2_root)
    missing: set[str] = set()
    for p in pieces_dir.glob("*.p2o"):
        for name in referenced_tm2_names(p):
            if name + ".tm2" not in present:
                missing.add(name)
    n = 0
    # The runtime defaults pieces with no material table to the stage texture
    # st0_a; make sure it exists in every room even when no piece names it.
    missing.add("st0_a")
    for name in sorted(missing):
        src = index.get(name + ".tm2")
        if src is None:
            continue
        shutil.copy2(src, texture_dir / src.name)
        present.add(src.name)
        n += 1
    return n


def write_manifest(room: str, canon_df: str, p2o_root: Path, tm2_root: Path,
                   out_dir: Path, scene_id: str) -> dict:
    model_src = (p2o_root / canon_df / "object" / "sdf" / room / "model")
    room_dir = out_dir / room
    pieces_dir = room_dir / "pieces"
    texture_dir = room_dir / "texture"

    n_p2o = copy_dir(model_src, pieces_dir, "p2o")
    n_tm2 = copy_textures(tm2_root, canon_df, texture_dir)
    n_miss = fill_missing_textures(pieces_dir, texture_dir, tm2_root)

    pieces = sorted(f.name for f in pieces_dir.glob("*.p2o"))
    lines = [
        f"# {room} room — scene composition manifest (native-port host data).",
        "#",
        "# Auto-generated by tools/scene-manifests/generate_scene_manifests.py",
        f"# from {canon_df} (object/sdf/{room}/model/, largest model set for this room).",
        "#",
        "# This file is NOT extracted game data and does not claim to reproduce the",
        "# original scene loader's resource table. sceneId is host test data bound,",
        "# conservatively, to the dominant runtime world_state 0x0F.",
        "",
        "texturedir texture",
        "piecesdir  pieces",
        "",
        f"scene {scene_id}",
    ]
    for p in pieces:
        lines.append(f"  {p}")
    lines.append("end")
    manifest_path = room_dir / f"{room}.manifest"
    manifest_path.write_text("\n".join(lines) + "\n")

    return {
        "room": room,
        "df": canon_df,
        "p2o": n_p2o,
        "tm2": n_tm2,
        "tm2_fill": n_miss,
        "manifest": str(manifest_path.relative_to(out_dir)),
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--p2o-root", type=Path, default=DEFAULT_P2O_ROOT)
    ap.add_argument("--tm2-root", type=Path, default=DEFAULT_TM2_ROOT)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--room", action="append",
                    help="only generate this room (repeatable)")
    ap.add_argument("--scene-id", default=DEFAULT_SCENE_ID,
                    help="scene id to bind in generated manifests")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    if not args.p2o_root.is_dir():
        print(f"error: p2o root not found: {args.p2o_root}", file=sys.stderr)
        return 1
    if not args.tm2_root.is_dir():
        print(f"error: tm2 root not found: {args.tm2_root}", file=sys.stderr)
        return 1

    rooms = canonical_rooms(args.p2o_root)
    want = set(args.room) if args.room else set(rooms)
    unknown = want - set(rooms)
    if unknown:
        print(f"error: unknown room(s): {', '.join(sorted(unknown))}", file=sys.stderr)
        return 1

    out_dir = args.out
    summary = []
    for room in sorted(want & set(rooms)):
        canon_df, cnt = rooms[room]
        if args.dry_run:
            summary.append({"room": room, "df": canon_df, "p2o": cnt})
            continue
        summary.append(write_manifest(room, canon_df, args.p2o_root,
                                      args.tm2_root, out_dir, args.scene_id))
        print(f"{room:6s} {canon_df:12s} {cnt:4d} p2o -> {out_dir / room}")

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "catalog.json").write_text(json.dumps(summary, indent=2))
    print(f"catalog -> {out_dir / 'catalog.json'} ({len(summary)} rooms)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
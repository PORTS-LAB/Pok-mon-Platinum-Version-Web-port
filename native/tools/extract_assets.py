#!/usr/bin/env python3
"""Extract an asset manifest from YOUR owned NDS ROM.

Reads the NDS header + FNT (name table) + FAT (allocation table) and writes
assets/manifest.json: gamecode, title, and every file's path/offset/size/sha.
Commits NO rom bytes — only metadata. The wasm build fetches file bytes
from the user's own ROM at build time (see Makefile `assets` target).

Usage:
    python3 tools/extract_assets.py "Pokemon - Platinum Version (USA).nds" [out.json]
"""
import hashlib
import json
import os
import struct
import sys


def u32(b, o):
    return struct.unpack_from("<I", b, o)[0]


def u16(b, o):
    return struct.unpack_from("<H", b, o)[0]


def parse_fnt(fnt):
    """Returns list of (dir_id, full_path_prefix). Then caller maps files."""
    ndirs = u16(fnt, 6) & 0x0FFF
    if ndirs < 1 or ndirs > 4096:
        raise ValueError("implausible dir count: %d" % ndirs)
    dirs = []
    for i in range(ndirs):
        base = i * 8
        dirs.append({
            "subtab": u32(fnt, base),
            "first_id": u16(fnt, base + 4),
            "parent": u16(fnt, base + 6),
        })
    # Resolve full path per dir via parent chain (root parent field == ndirs).
    paths = {}

    def dirpath(i, depth=0):
        if i in paths:
            return paths[i]
        if depth > 64:
            raise ValueError("dir cycle at %d" % i)
        if i == 0:
            paths[i] = ""
            return ""
        # find this dir's name: scan parent's subtable for subdir id == i
        p = dirs[i]["parent"] & 0x0FFF
        parent_path = dirpath(p, depth + 1)
        name = find_subdirname(fnt, dirs[p]["subtab"], i)
        full = (parent_path + "/" + name).strip("/") if parent_path else name
        paths[i] = full
        return full

    for i in range(ndirs):
        dirpath(i)
    return dirs, paths


def find_subdirname(fnt, subtab, want_id):
    o = subtab
    while True:
        if o >= len(fnt):
            raise ValueError("subtable overrun")
        t = fnt[o]
        o += 1
        if t == 0x00:
            break
        is_dir = bool(t & 0x80)
        ln = t & 0x7F
        name = fnt[o:o + ln].decode("latin-1")
        o += ln
        if is_dir:
            sid = u16(fnt, o) & 0x0FFF
            o += 2
            if sid == (want_id & 0x0FFF):
                return name
    raise ValueError("subdir id %d not found" % want_id)


def list_files(fnt, dirs, paths):
    files = []  # (file_id, path)
    for i, d in enumerate(dirs):
        o = d["subtab"]
        fid = d["first_id"]
        prefix = paths[i]
        while True:
            t = fnt[o]
            o += 1
            if t == 0x00:
                break
            if t & 0x80:  # subdir entry: name + id
                o += (t & 0x7F) + 2
                continue
            ln = t & 0x7F
            name = fnt[o:o + ln].decode("latin-1")
            o += ln
            full = (prefix + "/" + name).strip("/") if prefix else name
            files.append((fid, full))
            fid += 1
    return files


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    src = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else "assets/manifest.json"
    with open(src, "rb") as f:
        rom = f.read()
    if len(rom) < 0x200:
        raise SystemExit("file too small to be an NDS rom")
    title = rom[0x000:0x00C].decode("latin-1").rstrip("\x00")
    gamecode = rom[0x00C:0x010].decode("latin-1")
    fnt_off, fnt_len = u32(rom, 0x040), u32(rom, 0x044)
    fat_off, fat_len = u32(rom, 0x048), u32(rom, 0x04C)
    if not (fnt_off and fat_off and fnt_off + fnt_len <= len(rom)):
        raise SystemExit("bad FNT/FAT offsets — not a clean NDS rom?")
    fnt = rom[fnt_off:fnt_off + fnt_len]
    nfiles = fat_len // 8
    dirs, paths = parse_fnt(fnt)
    entries = []
    for fid, path in list_files(fnt, dirs, paths):
        if fid >= nfiles:
            continue
        start = u32(rom, fat_off + fid * 8)
        end = u32(rom, fat_off + fid * 8 + 4)
        if end < start or end > len(rom):
            continue
        blob = rom[start:end]
        entries.append({
            "path": path,
            "offset": start,
            "size": end - start,
            "sha256": hashlib.sha256(blob).hexdigest()[:16],
        })
    entries.sort(key=lambda e: e["path"])
    man = {
        "title": title,
        "gamecode": gamecode,
        "rom_size": len(rom),
        "rom_sha256": hashlib.sha256(rom).hexdigest(),
        "file_count": len(entries),
        "files": entries,
    }
    os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
    with open(out, "w") as f:
        json.dump(man, f, indent=1)
    print("%s %s files=%d -> %s" % (gamecode, title, len(entries), out))


if __name__ == "__main__":
    sys.exit(main())

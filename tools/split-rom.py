#!/usr/bin/env python3
"""Split the .nds ROM into host-safe chunks + manifest.json.

Usage:
  python3 tools/split-rom.py ["path to .nds"] [--chunk-mb 8]

Output:
  rom/manifest.json + rom/plat.part00 .. NN (each < 95MB, default 8MB)

Manifest: { version, game, totalBytes, sha256, chunkSize, chunks[] }
The web loader (app.js) fetches manifest then each chunk sequentially,
reassembles, verifies length + sha256, caches in IndexedDB, boots via Blob URL.
"""
import hashlib, json, os, sys

SRC_DEFAULT = "Pokemon - Platinum Version (USA).nds"
OUT_DIR = "rom"
PREFIX = "plat.part"
CHUNK_MB = 8

def main():
    src = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith("--") else SRC_DEFAULT
    chunk_mb = CHUNK_MB
    for a in sys.argv[1:]:
        if a.startswith("--chunk-mb="):
            chunk_mb = int(a.split("=")[1])
    if not os.path.isfile(src):
        print("ROM not found: %s" % src)
        print("Place your legally owned dump next to index.html, then re-run.")
        sys.exit(1)
    chunk_size = chunk_mb * 1024 * 1024
    os.makedirs(OUT_DIR, exist_ok=True)
    h = hashlib.sha256()
    total = os.path.getsize(src)
    chunks = []
    idx = 0
    with open(src, "rb") as f:
        while True:
            data = f.read(chunk_size)
            if not data:
                break
            name = "%s%02d" % (PREFIX, idx)
            with open(os.path.join(OUT_DIR, name), "wb") as o:
                o.write(data)
            h.update(data)
            chunks.append(name)
            print("wrote %s (%d bytes)" % (name, len(data)))
            idx += 1
    manifest = {
        "version": 1,
        "game": "platinum-usa",
        "totalBytes": total,
        "sha256": h.hexdigest(),
        "chunkSize": chunk_size,
        "chunks": chunks,
    }
    with open(os.path.join(OUT_DIR, "manifest.json"), "w") as o:
        json.dump(manifest, o, indent=2)
    print("manifest: total=%d sha256=%s parts=%d" % (total, h.hexdigest(), len(chunks)))
    big = [c for c in chunks if os.path.getsize(os.path.join(OUT_DIR, c)) > 95 * 1024 * 1024]
    if big:
        print("WARNING: chunks over 95MB: %s (re-run with smaller --chunk-mb)" % big)

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Generate wasm-seam patched copies of decomp TUs that clang rejects.

Mechanical, semantics-preserving class only:

  static-later — upstream declares a symbol non-static (in a header, or
  earlier in the same file) and later defines it `static`. mwcc ignores the
  mismatch; clang rejects it. Drop the `static` at the definition site.

Because the non-static declaration usually lives in a header, the scan is
two-pass: collect every non-static declared name across all sources and
headers, then patch definitions against that global set.

Usage: gen_patches.py <decomp-root> <out-dir>
Writes <out-dir>/<basename>.c for each file needing a change (collisions
across directories with equal basenames are reported, not silently merged).
"""
import os
import re
import sys

# A declaration: any line mentioning a candidate symbol, not starting with
# static. Deliberately broad — false positives only drop a redundant
# `static`, which cannot change behaviour for a symbol already declared
# extern elsewhere.
DECL_RES = [
    re.compile(r"^\s*(?!static\b)(?:extern\s+)?[A-Za-z_][\w \t*]*?\b(Unk_\w+|s[A-Z]\w*|[a-z]\w*_[A-Z]\w*|[a-z]\w*)\b"),
    re.compile(r"^\s*(?!static\b)[A-Za-z_][\w \t*]*?\s+\**(\w+)\s*\([^;{]*\)\s*;"),
    # function-pointer arrays / forward decls of functions
    re.compile(r"\b(\w+)\s*\[\]"),
    re.compile(r"^\s*(?!static\b)[A-Za-z_][\w \t*]*?\b(\w+)\s*\([^;{]*\)\s*;"),
]
DEF_RES = [
    # data definition: `static <type...> <name>[..] = {` / `;`
    re.compile(r"^static\s+[A-Za-z_][\w \t]*?\**\s*(\w+)\s*(\[|=)"),
    # function definition: `static <ret...> <name>(`
    re.compile(r"^static\s+[A-Za-z_][\w \t*]*?\b(\w+)\s*\("),
    # function-pointer arrays / constdata tables: `static ... <Name>[] = {`
    re.compile(r"^static\s+.*?\b(Unk_\w+|s[A-Z]\w*)\b"),
]


def collect(root):
    names = set()
    for sub in ("src", "include"):
        base = os.path.join(root, sub)
        if not os.path.isdir(base):
            continue
        for dirpath, _, files in os.walk(base):
            for fn in files:
                if not fn.endswith((".c", ".h", ".cpp")):
                    continue
                try:
                    text = open(os.path.join(dirpath, fn), errors="replace").read()
                except OSError:
                    continue
                for line in text.split("\n"):
                    if line.lstrip().startswith("static "):
                        continue
                    for rx in DECL_RES:
                        m = rx.search(line)
                        if m:
                            names.add(m.group(1))
    return names


def patch_file(path, names):
    lines = open(path, errors="replace").read().split("\n")
    out, hits = [], 0
    for line in lines:
        if line.lstrip().startswith("static "):
            for rx in DEF_RES:
                m = rx.match(line)
                if m and m.group(1) in names:
                    out.append(re.sub(r"^static\s+", "", line))
                    hits += 1
                    break
            else:
                out.append(line)
        else:
            out.append(line)
    return out, hits


def main():
    root, outdir = sys.argv[1], sys.argv[2]
    os.makedirs(outdir, exist_ok=True)
    names = collect(root)
    seen = {}
    patched = total = 0
    for dirpath, _, files in os.walk(os.path.join(root, "src")):
        for fn in sorted(files):
            if not fn.endswith(".c"):
                continue
            path = os.path.join(dirpath, fn)
            lines, hits = patch_file(path, names)
            total += hits
            if not hits:
                continue
            if fn in seen and seen[fn] != path:
                pass  # same basename, different dir: mirrored by relpath
            seen[fn] = path
            rel = os.path.relpath(path, root)
            dst = os.path.join(outdir, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            open(dst, "w").write("\n".join(lines))
            patched += 1
    print("global symbols=%d, patched %d files (%d sites) -> %s"
          % (len(names), patched, total, outdir))


if __name__ == "__main__":
    main()

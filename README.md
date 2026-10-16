# Pokémon Platinum — Web Port

Click **Start**, no upload needed. The ROM is served from this site's own `./rom/` chunks and boots in-browser via DeSmuME-wasm (Desmond, single-threaded — no special headers, no BIOS).

## Run locally

```bash
python3 tools/split-rom.py "Pokemon - Platinum Version (USA).nds"
python3 -m http.server 8000
# open http://localhost:8000
```

Click **Start Game**. First load downloads ~128MB with progress, caches in IndexedDB, boots to title. Second visit boots instantly.

## Deploy — Vercel

```bash
vercel --prod
```

`vercel.json` sets correct MIME + long cache for `/rom/*` and `/vendor/*.wasm`. Hobby static-file cap is 100MB/file — chunks are 8MB each so deploys pass. Bandwidth note: ~128MB per first-time visitor.

## Deploy — GitHub Pages

```bash
git init && git add index.html style.css app.js vendor/desmond.min.js vendor/desmond.wasm vercel.json rom/manifest.json rom/plat.part* tools/
git commit -m "platinum web port"
# create repo, push to main, enable Settings → Pages → Deploy from branch
```

Notes:
- Each chunk is 8MB — under GitHub's 100MB/file hard limit. The raw 128MB `.nds` is gitignored and never pushed.
- `.nojekyll` is included so Pages serves `_`-prefixed/wasm files untouched.
- No `COOP/COEP` headers needed (single-threaded wasm), so plain Pages works.

## Controls

Arrows/WASD move · X=A · Z=B · S=X · A=Y · Q=L · W=R · Enter=Start · Shift=Select · hold W = mic blow · bottom screen = stylus · gamepad supported.

## Files

- `index.html` / `style.css` / `app.js` — landing → loader → game, chunk fetch with progress, IndexedDB cache, Blob-URL boot, touch buttons (synthetic key events), fallback local-file boot
- `vendor/desmond.min.js` + `vendor/desmond.wasm` — emulator (self-hosted, side-by-side required)
- `rom/manifest.json` + `rom/plat.part*` — game data (built by `tools/split-rom.py`, never commit the raw `.nds`)
- `vercel.json`, `.nojekyll`, `.gitignore`

## True native port (in progress)

`native/` is the start of a real WASM port (decomp core + HAL instead of
emulation). Scaffold + verified tooling only — not playable yet. See
`native/README.md` for the roadmap, then `make -C native check`.
# Pok-mon-Platinum-Version-Web-port

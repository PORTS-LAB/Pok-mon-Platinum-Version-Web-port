#!/usr/bin/env bash
# Fetches pret/pokeplatinum at BUILD time. Never vendored (large history,
# and it only builds against a ROM you provide yourself).
set -euo pipefail
DEST="${1:-native/third_party/pokeplatinum}"
if [ -d "$DEST/.git" ]; then
  echo "exists: $DEST"
  exit 0
fi
mkdir -p "$(dirname "$DEST")"
git clone --depth 1 https://github.com/pret/pokeplatinum.git "$DEST"
echo "cloned -> $DEST"
echo "Next: follow $DEST/INSTALL.md (toolchain + place your ROM), then wire its core into src/."

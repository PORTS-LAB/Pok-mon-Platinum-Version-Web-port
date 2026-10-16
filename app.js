/* Pokemon Platinum web boot: chunked ROM -> Blob URL -> desmond-player.loadURL */
(function () {
  'use strict';
  var MANIFEST_URL = './rom/manifest.json';
  var IDB_DB = 'poke-platinum';
  var IDB_STORE = 'rom';
  var IDB_KEY = 'rom-v1';

  var $ = function (id) { return document.getElementById(id); };
  var landing = $('screen-landing'), loader = $('screen-loader'), game = $('screen-game');
  var btnStart = $('btn-start'), btnContinue = $('btn-continue');
  var phase = $('loader-phase'), fill = $('loader-fill'), text = $('loader-text');
  var btnCancel = $('btn-cancel'), btnResume = $('btn-resume');
  var landingStatus = $('landing-status'), toast = $('toast');
  var errModal = $('error-modal'), errHuman = $('error-human'), errTech = $('error-tech');

  var aborter = null;
  var aborted = false;
  var lastManifest = null;
  var memChunks = null; // resume within session

  function show(el) { el.hidden = false; }
  function hide(el) { el.hidden = true; }
  function say(t) { toast.textContent = t; }
  function fail(human, tech) {
    errHuman.textContent = human;
    errTech.textContent = String(tech || '');
    show(errModal);
  }
  $('btn-dismiss').onclick = function () { hide(errModal); };
  $('btn-retry').onclick = function () { hide(errModal); startFlow(); };

  // ---------- IndexedDB tiny wrapper ----------
  function idb() {
    return new Promise(function (res, rej) {
      var q = indexedDB.open(IDB_DB, 1);
      q.onupgradeneeded = function () { q.result.createObjectStore(IDB_STORE); };
      q.onsuccess = function () { res(q.result); };
      q.onerror = function () { rej(q.error); };
    });
  }
  function idbGet(db, key) {
    return new Promise(function (res, rej) {
      var tx = db.transaction(IDB_STORE, 'readonly').objectStore(IDB_STORE).get(key);
      tx.onsuccess = function () { res(tx.result || null); };
      tx.onerror = function () { rej(tx.error); };
    });
  }
  function idbPut(db, key, val) {
    return new Promise(function (res, rej) {
      var tx = db.transaction(IDB_STORE, 'readwrite').objectStore(IDB_STORE).put(val, key);
      tx.onsuccess = function () { res(); };
      tx.onerror = function () { rej(tx.error); };
    });
  }

  // ---------- manifest + download ----------
  function fmtMB(n) { return (n / 1048576).toFixed(1) + ' MB'; }

  async function fetchManifest() {
    var r = await fetch(MANIFEST_URL, { cache: 'no-cache' });
    if (!r.ok) throw new Error('manifest fetch ' + r.status + ' at ' + MANIFEST_URL);
    return r.json();
  }

  async function downloadChunked(manifest, onProg) {
    var total = manifest.totalBytes;
    var out = new Uint8Array(total);
    var loaded = 0;
    var t0 = performance.now();
    memChunks = [];
    aborted = false;
    aborter = new AbortController();

    for (var i = 0; i < manifest.chunks.length; i++) {
      if (aborted) throw new DOMException('aborted', 'AbortError');
      var name = manifest.chunks[i];
      phase.textContent = 'Downloading… chunk ' + (i + 1) + '/' + manifest.chunks.length;
      var resp = await fetch('./rom/' + name, { signal: aborter.signal });
      if (!resp.ok) throw new Error('ROM chunk missing: ./rom/' + name + ' (' + resp.status + ')');
      var len = +resp.headers.get('content-length') || manifest.chunkSize;
      var reader = resp.body ? resp.body.getReader() : null;
      var buf;
      if (!reader) {
        buf = new Uint8Array(await resp.arrayBuffer());
        out.set(buf, loaded); loaded += buf.length;
        onProg(loaded / total, loaded, total, t0);
      } else {
        var off = loaded;
        for (;;) {
          var part = await reader.read();
          if (part.done) break;
          out.set(part.value, off); off += part.value.length; loaded += part.value.length;
          onProg(loaded / total, loaded, total, t0);
          if (aborted) { try { reader.cancel(); } catch (e) {} throw new DOMException('aborted', 'AbortError'); }
        }
      }
      memChunks.push(i);
    }
    return out;
  }

  function progUI(frac, loaded, total, t0) {
    var pct = (frac * 100).toFixed(1);
    fill.style.width = pct + '%';
    var el = (performance.now() - t0) / 1000;
    var speed = el > 0.5 ? loaded / el : 0;
    var eta = speed > 0 ? Math.max(0, (total - loaded) / speed) : 0;
    text.textContent = pct + '% · ' + fmtMB(loaded) + ' / ' + fmtMB(total) +
      (speed ? ' · ' + fmtMB(speed) + '/s · ETA ' + Math.ceil(eta) + 's' : '');
  }

  function looksLikeLFS(bytes) {
    var head = '';
    for (var i = 0; i < Math.min(64, bytes.length); i++) head += String.fromCharCode(bytes[i]);
    return head.indexOf('version https://git-lfs') === 0;
  }

  async function verify(bytes, manifest) {
    if (bytes.length !== manifest.totalBytes) {
      throw new Error('size mismatch: got ' + bytes.length + ', expected ' + manifest.totalBytes);
    }
    if (looksLikeLFS(bytes)) {
      throw new Error('ROM is a Git LFS pointer, not the game. Re-run tools/split-rom without LFS.');
    }
    if (manifest.sha256 && window.crypto && crypto.subtle) {
      phase.textContent = 'Verifying…';
      var d = await crypto.subtle.digest('SHA-256', bytes);
      var hex = Array.prototype.map.call(new Uint8Array(d), function (b) {
        return ('0' + b.toString(16)).slice(-2);
      }).join('');
      if (hex !== manifest.sha256) throw new Error('SHA-256 mismatch (got ' + hex.slice(0, 16) + '…, expected ' + manifest.sha256.slice(0, 16) + '…). Cache cleared.');
    }
  }

  function boot(bytes) {
    return new Promise(function (res, rej) {
      var player = document.getElementById('player');
      if (!player || typeof player.loadURL !== 'function') {
        rej(new Error('emulator not ready (vendor/desmond.min.js failed to define <desmond-player>)'));
        return;
      }
      var blob = new Blob([bytes], { type: 'application/octet-stream' });
      var url = URL.createObjectURL(blob);
      phase.textContent = 'Starting…';
      text.textContent = 'Booting Platinum…';
      try {
        player.loadURL(url, function () {
          hide(loader); hide(landing); show(game);
          say('Running — click the game once if there is no sound. Hold W = mic.');
          maybeShowTouch();
          res();
        });
        setTimeout(function () {
          if (!loader.hidden) { /* still loading; desmond gives no error hook */ }
        }, 20000);
      } catch (e) { rej(e); }
    });
  }

  // ---------- main flow ----------
  async function startFlow() {
    hide(errModal);
    hide(landing); show(loader); hide(game);
    hide(btnResume);
    fill.style.width = '0%'; text.textContent = '…';
    try {
      if (typeof WebAssembly === 'undefined') throw new Error('WebAssembly unsupported in this browser.');
      var manifest = await fetchManifest();
      lastManifest = manifest;
      var db = await idb().catch(function () { return null; });
      if (db) {
        var hit = await idbGet(db, IDB_KEY).catch(function () { return null; });
        if (hit && hit.bytes && hit.bytes.byteLength === manifest.totalBytes && hit.sha256 === manifest.sha256) {
          phase.textContent = 'Loading from device (fast)…';
          text.textContent = 'Cache hit — booting…';
          await boot(new Uint8Array(hit.bytes));
          return;
        }
      }
      var bytes = await downloadChunked(manifest, progUI).catch(async function (e) {
        if (e && e.name === 'AbortError') { show(btnResume); throw e; }
        throw e;
      });
      await verify(bytes, manifest);
      if (db) {
        try {
          var copy = bytes.slice().buffer;
          await idbPut(db, IDB_KEY, { bytes: copy, sha256: manifest.sha256, savedAt: Date.now() });
        } catch (e) { console.warn('IDB cache put failed', e); }
      }
      await boot(bytes);
    } catch (e) {
      if (e && e.name === 'AbortError') {
        phase.textContent = 'Paused';
        text.textContent = 'Download paused — press Resume.';
        show(btnResume);
        return;
      }
      console.error(e);
      show(landing);
      var msg = String((e && e.message) || e);
      if (/manifest fetch 404|chunk missing/i.test(msg)) {
        fail('ROM files are missing on this deploy.', msg + '\n\nFix: run tools/split-rom, commit rom/chunk-*.bin + rom/manifest.json (each <95MB), redeploy. Or use Advanced → pick a local .nds below.');
      } else if (/SHA-256|size mismatch/i.test(msg)) {
        try { var d = await idb(); d && await idbPut(d, IDB_KEY, null); } catch (x) {}
        fail('Download corrupted — cache cleared.', msg);
      } else {
        fail('Could not start the game.', msg);
      }
    }
  }

  btnStart.onclick = startFlow;
  btnCancel.onclick = function () { aborted = true; try { aborter && aborter.abort(); } catch (e) {} };
  btnResume.onclick = function () { hide(btnResume); startFlow(); };

  // Continue button if cache exists
  (async function () {
    try {
      var m = await fetchManifest(); lastManifest = m;
      var db = await idb();
      var hit = db && await idbGet(db, IDB_KEY);
      if (hit && hit.bytes && hit.bytes.byteLength === m.totalBytes) {
        show(btnContinue);
        btnContinue.onclick = startFlow;
        landingStatus.textContent = 'Cached copy found — Start boots instantly.';
      } else {
        landingStatus.textContent = 'First run downloads ~' + fmtMB(m.totalBytes) + ' once, then caches it.';
      }
    } catch (e) {
      landingStatus.textContent = 'Could not read rom/manifest.json — deploy the rom/ chunks first. (' + e.message + ')';
    }
  })();

  // fallback local file -> same boot path + cache it
  $('fallback-rom').addEventListener('change', async function (ev) {
    var f = ev.target.files && ev.target.files[0];
    if (!f) return;
    hide(landing); show(loader);
    phase.textContent = 'Reading local file…';
    try {
      var bytes = new Uint8Array(await f.arrayBuffer());
      if (lastManifest) { try { await verify(bytes, lastManifest); } catch (e) { console.warn(e); } }
      var db = await idb().catch(function () { return null; });
      if (db && lastManifest && bytes.length === lastManifest.totalBytes) {
        try { await idbPut(db, IDB_KEY, { bytes: bytes.slice().buffer, sha256: lastManifest.sha256, savedAt: Date.now() }); } catch (e) {}
      }
      await boot(bytes);
    } catch (e) { console.error(e); show(landing); fail('Local file failed to boot.', e.message || e); }
  });

  // ---------- toolbar ----------
  $('btn-fullscreen').onclick = function () {
    var el = document.documentElement;
    if (document.fullscreenElement) { document.exitFullscreen(); }
    else if (el.requestFullscreen) { el.requestFullscreen(); }
  };
  $('btn-mic').onclick = function () {
    var p = document.getElementById('player');
    try { p.enableMicrophone(); say('Microphone enabled — blow to trigger mic events.'); }
    catch (e) { say('Mic blocked — hold W / 🎤 instead.'); }
  };
  $('btn-controls').onclick = function () {
    var t = $('touch');
    t.hidden = !t.hidden;
  };

  // ---------- touch buttons -> synthetic key events for desmond ----------
  function sendKey(keyCode, down) {
    var type = down ? 'keydown' : 'keyup';
    var e;
    try {
      e = new KeyboardEvent(type, { bubbles: true, cancelable: true });
      Object.defineProperty(e, 'keyCode', { value: keyCode });
      Object.defineProperty(e, 'which', { value: keyCode });
    } catch (x) {
      e = document.createEvent('Event');
      e.initEvent(type, true, true);
      e.keyCode = keyCode;
    }
    window.dispatchEvent(e);
    try { if (typeof window.onkeydown === 'function' && type === 'keydown') {} } catch (x) {}
  }
  function maybeShowTouch() {
    if (('ontouchstart' in window) || navigator.maxTouchPoints > 0) show($('touch'));
  }
  document.querySelectorAll('#touch [data-key]').forEach(function (b) {
    var code = +b.getAttribute('data-key');
    var down = function (e) { e.preventDefault(); try { b.setPointerCapture(e.pointerId); } catch (x) {} sendKey(code, true); };
    var up = function (e) { e.preventDefault(); sendKey(code, false); };
    b.addEventListener('pointerdown', down);
    b.addEventListener('pointerup', up);
    b.addEventListener('pointercancel', up);
    b.addEventListener('pointerleave', function (e) { if (e.buttons === 0) sendKey(code, false); });
    b.addEventListener('contextmenu', function (e) { e.preventDefault(); });
  });

  window.addEventListener('error', function (e) {
    console.warn('window.onerror', e.message);
  });
})();

# rsaShit — Design Document

## Purpose

Provide a compact, developer-ready design for `rsaShit`, a minimal interactive RSA CTF toolkit. The tool must be small,
deterministic, and easy to extend. The UX is a short wizard-style REPL rather than complex CLI flags.
Performance-critical math uses GMP. Public API surface should avoid raw pointers and use RAII.

---

## Goals

* Minimal, focused UX: wizard-style REPL for quick CTF workflows.
* High performance for big-integer operations via GMP.
* Safe modern C++ patterns: RAII, `std::vector`, `std::string`, `std::unique_ptr`.
* Small codebase and clear module boundaries for fast iteration.
* Extensible attacks pipeline and easy external-tool integration (gmp-ecm, msieve).

---

## Non-Goals

* Full PKI or TLS feature set. No network by default.
* Reimplement ECM / QS. External tools or libraries are used for heavy factoring.
* GUI. This is a terminal-focused tool.

---

## High-level flow (REPL)

1. Start: `rsaShit`
2. Prompt sequence:

    * `enter N (or "idk"):`
    * `enter q (or "idk"):`
    * `enter e (or "idk"):`
    * `enter c (or "idk"):`
    * `choose attack ("l" for list, "idk" for auto):`
3. Attack executes; wizard prints stage-by-stage status.
4. On success: prints `p`, `q`, `d` and offers to decrypt `c`, save results, or continue.
5. Session persists last 8 items and a `~/.rshit/history.log` file.

---

## Accepted input formats

* Decimal (digits) or hex (`0x...`).
* `file:path` to load value from file (hex or decimal text).
* Literal `idk` or blank for unknown.
* Single reprompt if parsing fails; otherwise treat as `idk`.

---

## Modules and responsibilities

### `main` / `repl`

* Starts REPL, orchestrates prompts and choice handling.
* Maintains `SessionState`.
* Provides `help`, `quit` and basic I/O.
* Uses `linenoise-ng` if available, otherwise fallback `getline`.

### `SessionState`

* Tracks loaded items (label, n, e, c, p, q, notes).
* Keeps last 8 items; exposes lookup by numeric label or index.
* Handles `history.log` writes.

### `BigInt` (wrapper around `mpz_t`)

* RAII: init/clear in ctor/dtor.
* Construct from `uint64_t`, `string` (dec/hex), `vector<uint8_t>` (big-endian), or `mpz_t` move.
* Methods: `to_dec()`, `to_hex()`, `bit_length()`, `is_even()`, `cmp()`, `gcd()`, `powm()`, `invert()`, `sqrt_floor()`
  etc.
* Operator overloads for `+,-,*,/,%,==,!=,<,>` returning `BigInt` or `bool` as appropriate.
* Reuse internal temporaries for hot loops where possible.

### `RSAKey` / `RSAOps`

* Holds `BigInt n, e, d, p, q` (optional fields may be empty).
* Methods: `attempt_compute_d()`, `decrypt(c)`, `encrypt(m)`, `derive_pq_from_n_and_q()`.
* `from_pq(p,q,e)` factory.

### `attacks` namespace

Each attack is a function that accepts `SessionState` and the target label and returns `AttackResult`.

* `trial_division(target, limit)` — small primes up to limit.
* `fermat(target, max_iters)` — Fermat’s method.
* `pollard_rho(target, seed, max_iters)` — randomized attempts.
* `pollard_pminus1(target, B)`.
* `wiener_cf(target)` — continued fraction small-d attack.
* `low_e_broadcast(targets[])` — Håstad style CRT solver.
* `common_modulus(target, other_targets[])`.
* `external_ecm_msieve(target)` — spawn external tools if installed; parse output.

`AttackResult` contains: `success bool`, `p BigInt`, `q BigInt`, `d BigInt (optional)`, `log string`.

### `utils`

* Parsers for decimal/hex/file inputs.
* Encoding helpers: hex↔bytes, base64 decode/encode, attempt text heuristics.
* Timers and logging helpers.
* JSON serializer for `export`.

### `ext` (external tool wrappers)

* Check installed binaries and versions (`gmp-ecm`, `msieve`).
* Spawn processes with timeouts and capture stdout.
* Parse common output formats to extract found factors.

---

## Auto pipeline (exact order)

1. If `q` provided try `gcd(n,q)` and derive `p`.
2. Quick sanity checks: `n` even, tiny, trivial.
3. Trial division small primes (configurable cutoff, default 1e6).
4. GCD checks against loaded items.
5. Pollard-Rho with multiple seeds (default seeds: 7, 23, 101). Timebox per seed.
6. Pollard-p-1 with increasing B bounds.
7. Fermat for close primes (timeboxed iterations).
8. Wiener's continued fraction.
9. Low-e broadcast if multiple targets available.
10. External `gmp-ecm` then `msieve` (if present). Timeouts configurable.

Stop the pipeline at first success. Write a concise, timestamped log entry for each stage.

---

## UX details and prompts

* Prompts must be single-line; immediate feedback.
* Use minimal but clear messages for each stage.
* On success display both hex (`0x...`) and decimal.
* For plaintext output attempt these decoders in order: raw bytes->utf8, latin1, base64.
* On `save` create `out/<label>_key.pem` and `out/<label>_plain.txt` when applicable.

---

## File layout

```
src/
  main.cpp
  repl.cpp/.hpp
  session.cpp/.hpp
  bigint.cpp/.hpp
  rsa.cpp/.hpp
  attacks/
    trial.cpp
    pollard.cpp
    pminus1.cpp
    fermat.cpp
    wiener.cpp
    lowe.cpp
    external.cpp
  utils/
    parse.cpp
    encodings.cpp
  ext/
    ecm_wrapper.cpp
    msieve_wrapper.cpp
CMakeLists.txt
tests/
  vectors/
  harness.cpp
docs/
  design.md
```

---

## Build & Dependencies

* C++17 minimum. Prefer C++20.
* Dependencies: `gmp` (runtime & dev), optional `linenoise-ng`, `nlohmann/json` for JSON.
* Build: `cmake` + `make` / `ninja`.
* Example build steps:

  ```bash
  mkdir build && cd build
  cmake .. -DCMAKE_BUILD_TYPE=Release
  cmake --build . -- -j$(nproc)
  ```

---

## Testing

* Include canonical CTF vectors: small RSA, Wiener-vulnerable, low-e broadcast, common modulus.
* Unit tests for `BigInt` wrappers: conversion, powm, invert, gcd.
* Integration test running `auto` pipeline against vectors with expected outcomes.
* CI: run unit tests and a subset of integration tests. Skip long ECM/MSieve steps in CI.

---

## Security & Safety

* Do not execute untrusted code. External tools run in a spawned process with a timeout.
* Validate file paths used in `file:` inputs. Reject paths that attempt directory traversal for save operations.
* Avoid exposing network interfaces. If a `serve` command is added later, disable by default.

---

## Coding conventions

* Prefer `snake_case` for filenames and `CamelCase` for types.
* Public APIs must not expose raw `mpz_t` pointers. Use `BigInt` wrapper.
* Keep functions short and single-responsibility.
* Add comments for non-obvious math choices and timeouts.

---

## Milestones

1. **Skeleton (week 0.5)**: REPL, `BigInt` wrapper, basic parsing, trial & pollard-rho attacks, logging. (Minimal
   external deps)
2. **Core attacks (week 1)**: Add Fermat, Pollard-p-1, Wiener, low-e. `auto` pipeline implemented.
3. **External tools (week 2)**: Integrate `gmp-ecm` and `msieve` wrappers. Add timeouts and parse outputs.
4. **Polish (week 3)**: Save/export, plaintext heuristics, history, tests, CI.
5. **Optional**: multi-threading, tab-completion, UI polish.

---

## Open decisions (to resolve before coding)

* Use `gmp` or header-only `boost::multiprecision`? (Recommendation: GMP for speed.)
* Choice of line-editing library: `linenoise-ng` vs fallback. (Recommendation: `linenoise-ng`.)
* Trial division cutoff default and timeouts per stage.

---

## Example attack pseudocode (Wiener)

```cpp
AttackResult wiener(const RSAKey &k) {
  // compute continued fractions of e/n, examine convergents
  // for each convergent k/d try to solve for phi and factor
}
```

---

## Logging format (history.log)

One JSON object per line. Example:

```json
{
  "ts": "2025-11-09T19:15:00+03:00",
  "label": "ctf1",
  "stage": "pollard-rho",
  "duration_ms": 1450,
  "result": "found",
  "p": "0x...",
  "q": "0x..."
}
```

---

## Developer notes

* Keep code review strict about exposing raw mpz structures.
* Favor clarity over micro-optimizations unless profiling shows need.
* Document external tool versions used in `docs/`.

---

End of design document.

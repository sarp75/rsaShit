# rsaShit

Proving that RSA is **shit**, one attack at a time.

## Features (current)

- GMP-backed BigInt wrapper to save your time (RAII around `mpz_t`).
- REPL commands for attacks.
- Implemented attacks:
    - `lowe` (Håstad low exponent broadcast) & demo.
    - `wiener` small-d attack & self-test.
    - `cmod` common modulus attack & self-test.
    - `fermat` for close prime factors & self-test.
    - `hi` responds back with `hello`.
- Parsing for decimal / hex (`0x...`). Extended parsing (file:, idk) skeleton in place.

## Planned / Roadmap

- Trial division, Pollard-Rho, Pollard p−1 implementations.
- Auto pipeline executor (ordered stages per design doc).
- SessionState with history ring buffer & `~/.rshit/history.log` JSON lines.
- External tool wrappers (`gmp-ecm`, `msieve`).
- Enhanced parsing (file:, idk, base64 decode, plaintext heuristics).
- Export/save (`out/<label>_key.pem`, plaintext extraction).
- Test harness & CI.

## Build Requirements

- C++20 compiler (Clang / GCC).
- GMP development headers & library.
- CMake ≥ 3.16.

### Install dependencies

macOS (Homebrew):

```bash
brew install gmp cmake
```

Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential cmake libgmp-dev
```

Windows:

```text
Fuck if I know. Use WSL or vcpkg to get GMP and CMake.
```

### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Binary: `build/rsaShit` (or `cmake-build-debug/rsaShit` if using an IDE like CLion).

### Run

```bash
./build/rsaShit
```

You enter the interactive REPL. Type `help` for commands.

## REPL Commands (current)

| Command           | Purpose                                                        |
|-------------------|----------------------------------------------------------------|
| `help`            | list commands                                                  |
| `quit` / `exit`   | leave REPL                                                     |
| `lowe`            | wizard for low exponent broadcast (enter e, count, N[i], C[i]) |
| `lowe-demo`       | tiny built-in low exponent example                             |
| `wiener`          | wizard: enter N, e for small-d recovery                        |
| `wiener-selftest` | generate a small vulnerable key and verify Wiener attack       |
| `cmod`            | common modulus attack wizard (n, e1, e2, c1, c2)               |
| `cmod-selftest`   | self-test for common modulus                                   |
| `fermat`          | Fermat factorization wizard (n, optional max iterations)       |
| `fermat-selftest` | self-test with close primes                                    |

## Usage Examples

### Håstad Low Exponent Broadcast

```text
> lowe
enter e> 3
enter count of targets (>= e)> 3
enter N[0]> 0x1000003f
enter C[0]> 169846473
enter N[1]> 0x1000005b
enter C[1]> 169650249
enter N[2]> 0x1000006f
enter C[2]> 169510089
low-e recovered m: 12345 (0x3039)
```

### Wiener Attack

```text
> wiener
enter N> 0x1a2b3c4d5e6f708192a3b4c5d6e7f8091a2b3c4d5e6f708192a3b4c5d6e7f8091
enter e> 123456
wiener success: recovered d=123456789
```

### Common Modulus Attack

```text
> cmod
enter N> 0x1000003f
enter e1> 3
enter e2> 7
enter C1> 123456789
enter C2> 234567890
cmod success: recovered m=12345 (0x3039)
```

### Fermat Attack

```text
> fermat
enter N> 1234567890123456
enter max iterations (default 1000000)>
fermat success: p=0x1000003f, q=0x5b
```

## License

**MIT License**, I guess. Do whatever you want with it, I don't care.

## Safety Notes

- Do not run against untrusted remote inputs without review.

## Quick Troubleshooting

| Issue                   | Fix                                                                                               |
|-------------------------|---------------------------------------------------------------------------------------------------|
| `GMP not found`         | Install dev headers (brew install gmp / apt install libgmp-dev). Re-run CMake.                    |
| REPL shows no output    | Run directly from local terminal, not via heredoc tools; ensure build succeeded.                  |
| Attack gives no success | Verify preconditions (e.g., gcd(e1,e2)=1 for common modulus, p≈q for Fermat, small d for Wiener). |

## Example Build + Run (Copy/Paste)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/rsaShit
```

Enjoy proving that RSA is **absolute shit**!
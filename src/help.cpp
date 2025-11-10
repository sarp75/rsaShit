#include "help.hpp"
#include <iostream>

namespace help {
    void print_general() {
        std::cout << R"(
rsaShit - RSA attack toolkit
============================

Available commands:
  help [command]  - show this help or detailed help for a command
  quit / exit     - exit the program
  show            - show session state
  hi              - say hello

RSA Attacks:
  lowe            - low exponent broadcast attack (Håstad)
  wiener          - wiener's attack for small private exponent d
  cmod            - common modulus attack
  fermat          - fermat factorization for close primes
  rho             - pollard's rho factorization
  coppersmith     - coppersmith small root attack for partial message recovery
  pminus1         - pollard's p-1 factorization

type 'help <command>' for detailed info on a specific attack.
examples: 'help rho', 'help fermat', 'help wiener'
)";
    }

    void print_command_help(const std::string &cmd) {
        if (cmd == "rho") {
            std::cout << R"(
rho - Pollard's Rho Factorization Attack
=========================================

WHEN TO USE:
  - when you have a composite number N = p*q
  - when the factors are not too large (< 10^15 or so)
  - works best when factors are of moderate size
  - general-purpose factorization method

HOW IT WORKS:
  - uses floyd's cycle detection (tortoise and hare)
  - generates pseudo-random sequence: f(x) = x^2 + c mod N
  - finds collisions that reveal factors via gcd
  - tries multiple c values and starting points for robustness

USAGE:
  > rho
  enter N> <composite_number>
  max iters (dec, default 1000000)> [press Enter for default or specify budget]

PARAMETERS:
  - N: the composite number to factor
  - max iters: iteration budget (default 1M, increase for larger numbers)

EXAMPLE:
  > rho
  enter N> 143
  max iters (dec, default 1000000)>
  rho factor: 11 (0xb)

NOTES:
  - will fail on prime numbers (as expected - no factors exist)
  - increase max iters for larger composites
  - typically fast for numbers with factors < 10^12
)";
        } else if (cmd == "fermat") {
            std::cout << R"(
fermat - Fermat Factorization Attack
=====================================

WHEN TO USE:
  - when N = p*q where p and q are CLOSE to each other
  - bob accidentally chose primes that are near each other
  - extremely fast when primes are close (within ~10% of each other)

HOW IT WORKS:
  - represents N as difference of squares: N = a^2 - b^2 = (a-b)(a+b)
  - starts with a = ceil(sqrt(N))
  - checks if a^2 - N is a perfect square
  - increments a until factors are found

USAGE:
  > fermat
  enter N> <composite_number>
  max iters (dec, default 1000000)> [press Enter for default]

PARAMETERS:
  - N: composite number with close prime factors
  - max iters: how many values of a to try

EXAMPLE:
  > fermat
  enter N> 1000036009
  max iters (dec, default 1000000)>
  fermat success: p=0xf4243, q=0xf4261

NOTES:
  - very fast when p ≈ q (close primes)
  - slow/ineffective when p and q are far apart
  - use rho instead if factors are not close
)";
        } else if (cmd == "wiener") {
            std::cout << R"(
wiener - Wiener's Small Private Exponent Attack
================================================

WHEN TO USE:
  - when bob uses a small private exponent d
  - specifically when d < (1/3) * N^(1/4)
  - bob tried to speed up decryption by using small d

HOW IT WORKS:
  - builds continued fraction expansion of e/N
  - tests convergents as candidates for d
  - verifies by checking if resulting p,q multiply to N

USAGE:
  > wiener
  enter N> <modulus>
  enter e> <public_exponent>

PARAMETERS:
  - N: the RSA modulus
  - e: the public exponent

EXAMPLE:
  > wiener
  enter N> 1000036009
  enter e> 71428571
  wiener success: p=..., q=..., d=...

NOTES:
  - only works for unusually small d values
  - most real rsa systems don't use small d
  - automatic - no iteration budget needed
)";
        } else if (cmd == "lowe") {
            std::cout << R"(
lowe - Low Exponent Broadcast Attack (Håstad)
==============================================

WHEN TO USE:
  - when the same message m is sent to e different recipients
  - all recipients use the same small public exponent e
  - each recipient has different modulus N_i
  - you intercept all e ciphertexts

HOW IT WORKS:
  - uses chinese remainder theorem (CRT)
  - combines c_i = m^e mod N_i for all i
  - reconstructs m^e mod (N_1 * N_2 * ... * N_e)
  - takes e-th root to recover m

USAGE:
  > lowe
  enter e> <public_exponent>
  enter count of targets (>= e)> <number_of_ciphertexts>
  enter N[0]> <first_modulus>
  enter C[0]> <first_ciphertext>
  ... (repeat for all targets)

PARAMETERS:
  - e: the common public exponent (e.g., 3)
  - count: number of intercepted ciphertexts (must be >= e)
  - N[i]: each recipient's modulus
  - C[i]: each ciphertext

EXAMPLE:
  > lowe-demo
  (runs built-in demonstration)

NOTES:
  - requires at least e different ciphertexts
  - all must encrypt the same message m
  - moduli must be pairwise coprime
  - only works when m^e < product of all N_i
)";
        } else if (cmd == "cmod") {
            std::cout << R"(
cmod - Common Modulus Attack
=============================

WHEN TO USE:
  - when the same message m is encrypted with same modulus N
  - but two different public exponents e1 and e2
  - you have both ciphertexts c1 and c2
  - e1 and e2 must be coprime (gcd(e1, e2) = 1)

HOW IT WORKS:
  - uses extended euclidean algorithm to find s, t
  - such that: s*e1 + t*e2 = 1
  - computes m = c1^s * c2^t mod N
  - exploits the linear combination of exponents

USAGE:
  > cmod
  enter n> <modulus>
  enter e1> <first_exponent>
  enter e2> <second_exponent>
  enter c1> <first_ciphertext>
  enter c2> <second_ciphertext>

PARAMETERS:
  - n: the shared RSA modulus
  - e1, e2: two different public exponents (must be coprime)
  - c1, c2: ciphertexts of the same message under e1 and e2

EXAMPLE:
  > cmod-selftest
  (runs built-in test)

NOTES:
  - e1 and e2 must be coprime
  - both ciphertexts must encrypt the same plaintext
  - works even without knowing the private key
  - demonstrates why you shouldn't reuse moduli with different exponents
)";
        } else if (cmd == "coppersmith") {
            std::cout << R"(
coppersmith - Coppersmith Small Root Attack
============================================

WHEN TO USE:
  - you have a modular polynomial f(x) ≡ 0 (mod N) with a SMALL root x
  - typical cases here:
      (1) linear: a*x + b ≡ 0 (mod N) with |x| < N^β
      (2) partial message: ciphertext c = (m)^e mod N, you know high bits of m
  - small public exponent e (e = 3 or 5 supported here) gives low-degree polynomials
  - partial information about the plaintext lets you express m = m_high + x

WHAT THIS IMPLEMENTATION DOES:
  - provides a minimal exact-arithmetic LLL for very small toy lattices (linear case)
  - for partial message recovery it currently brute forces the unknown bit window
  - demonstrates the concept; not a full general-purpose coppersmith engine

MODES:
  TYPE 1 (Linear Small Root): solve a*x + b ≡ 0 (mod n)
    - supply: a, b, n
    - finds x if |x| < N^β (β internally approximated; effectively a small bound)
    - uses simple 2x2 lattice then short vector extraction

  TYPE 2 (Partial Message Recovery): m = m_high + x
    - ciphertext: c = (m_high + x)^e mod n
    - unknown_bits = bit length of x (search space size 2^unknown_bits)
    - brute forces x; practical here when unknown_bits <= ~24

PARAMETERS (prompted interactively):
  - a, b: coefficients of linear equation (type 1)
  - n: RSA modulus
  - c: ciphertext (type 2)
  - e: public exponent (3 or 5 supported)
  - m_high: known high portion of message (already shifted into position)
  - unknown_bits: number of low bits to recover (size of brute force)

USAGE:
  > coppersmith
  enter type (1=linear, 2=partial-msg)> 1
  enter a (coeff of x)> ...
  enter b (constant)> ...
  enter n (modulus)> ...

  or for partial message:
  > coppersmith
  enter type (1=linear, 2=partial-msg)> 2
  enter c (ciphertext)> ...
  enter e (exponent, typically 3 or 5)> 3
  enter n (modulus)> ...
  enter m_high (known high bits of message)> ...
  enter unknown_bits (number of unknown low bits)> 16

NOTES / LIMITATIONS:
  - lattice step here is only for the linear toy; real coppersmith builds larger lattices
  - partial message mode is brute force (exponential in unknown_bits)
  - keep unknown_bits modest for speed
  - ensure m_high already accounts for its bit position (i.e., m_high << unknown_bits if needed)
  - (a, n) must be coprime for simple inverse-based shortcut; otherwise lattice / brute force
  - upgrading to full coppersmith would involve multi-dimensional lattice & proper parameter tuning

EXAMPLE (linear): solve 123*x + 456 ≡ 0 (mod N) for small x
  - if gcd(123, N)=1 compute x ≡ -456 * 123^{-1} mod N, then check if it's < bound

EXAMPLE (partial): m = m_high + x, e=3, recover last 16 bits of m
  - search space 65536, fast brute force
)";
        } else if (cmd == "pminus1") {
            std::cout << R"(
pminus1 - Pollard's p-1 Factorization
=====================================

WHEN TO USE:
  - when n has a prime factor p such that p-1 is B1-smooth
  - or p-1 = s * r where s is B1-smooth and r has a single large prime in (B1, B2]

HOW IT WORKS (this tool):
  - Stage 1: power a by prime powers ≤ B1 modulo n, then g = gcd(a-1, n)
  - Stage 2 (optional): scan primes q in (B1, B2]; test gcd(a^q - 1, n)

USAGE:
  > pminus1
  enter N> <composite>
  enter B1 (stage1 bound, dec default 100000)> <bound1>
  enter B2 (stage2 bound, dec; 0 to disable, default 0)> <bound2>
  enter trials (bases to try, dec default 5)> <count>

PARAMETERS:
  - N: number to factor
  - B1: stage 1 smoothness bound
  - B2: stage 2 bound (0 disables stage 2)
  - trials: number of bases to try (a values)

NOTES:
  - increasing B1 and B2 raises cost but improves success probability
  - this stage 2 is a simple variant; good for demos and mid-size factors
  - try multiple bases if stage 1 fails; add stage 2 for a wider net
)";
        } else if (cmd == "show") {
            std::cout << R"(
show - Display Session State
=============================

Shows the current session state and stored values.
Currently shows the count of session items.
)";
        } else if (cmd == "quit" || cmd == "exit") {
            std::cout << R"(
quit / exit - Exit the Program
===============================

Exits the rsaShit REPL and returns to the shell.
)";
        } else if (cmd == "hi") {
            std::cout << R"(
hi - Friendly Greeting
======================

Says hello back.
)";
        } else if (cmd == "help") {
            std::cout << R"(
help - Display Help Information
================================

USAGE:
  help            - show general help with list of all commands
  help <command>  - show detailed help for a specific command
  help <rsaValue> - show info on an RSA parameter (N, e, p, q, d, c, m, a, b)

EXAMPLES:
  help rho       - detailed info about rho factorization
  help N         - detailed info about N in RSA
  help wiener    - detailed info about wiener attack
)";
        } else if (cmd == "N" || cmd == "n") {
            std::cout << R"(
N - RSA Modulus
===============

WHAT IS IT:
  - the public modulus in RSA
  - N = p * q where p and q are large primes
  - part of the public key (N, e)
  - size typically 1024, 2048, or 4096 bits in real systems

SECURITY:
  - factoring N to find p and q is the core RSA problem
  - if you can factor N, you can break RSA
  - security relies on difficulty of factoring large N

ATTACKS TARGETING N:
  - fermat: works when p and q are close
  - rho: general factorization method
  - wiener: exploits relationship between N and small d
)";
        } else if (cmd == "e") {
            std::cout << R"(
e - Public Exponent
===================

WHAT IS IT:
  - the public encryption exponent
  - part of the public key (N, e)
  - commonly 3 or 65537 (0x10001)
  - used for encryption: c = m^e mod N

WHY SMALL VALUES:
  - faster encryption
  - but can introduce vulnerabilities

ATTACKS TARGETING e:
  - lowe: exploits small e with broadcast attack
  - cmod: exploits reusing N with different e values
)";
        } else if (cmd == "p" || cmd == "q") {
            std::cout << R"(
p, q - Prime Factors
====================

WHAT ARE THEY:
  - the two large prime numbers where N = p * q
  - kept secret - part of the private key generation
  - typically similar in size (both around sqrt(N))

SECURITY:
  - must be kept secret at all costs
  - if revealed, RSA is completely broken
  - should be randomly generated and far apart

COMMON MISTAKES:
  - choosing p and q too close together → fermat attack
  - reusing p or q across different keys
  - using weak random number generation

ATTACKS TARGETING p, q:
  - fermat: finds p and q when they're close
  - rho: general method to factor N into p and q
)";
        } else if (cmd == "d") {
            std::cout << R"(
d - Private Exponent
====================

WHAT IS IT:
  - the private decryption exponent
  - satisfies: e * d ≡ 1 (mod φ(N))
  - where φ(N) = (p-1)(q-1)
  - used for decryption: m = c^d mod N

SECURITY:
  - must be kept absolutely secret
  - typically as large as N
  - small d values are vulnerable

COMPUTING d:
  - d = e^(-1) mod φ(N)
  - requires knowing p and q to compute φ(N)
  - why factoring N breaks RSA

ATTACKS TARGETING d:
  - wiener: exploits small d values (d < N^0.25)
)";
        } else if (cmd == "c") {
            std::cout << R"(
c - Ciphertext
==============

WHAT IS IT:
  - the encrypted message
  - computed as: c = m^e mod N
  - publicly transmitted
  - can be intercepted by attackers

ATTACKS USING c:
  - lowe: multiple ciphertexts of same message with different N
  - cmod: two ciphertexts of same message with different e
  - all attacks ultimately try to recover m from c
)";
        } else if (cmd == "m") {
            std::cout << R"(
m - Message/Plaintext
=====================

WHAT IS IT:
  - the original unencrypted message
  - must satisfy: 0 < m < N
  - encrypted to c via: c = m^e mod N
  - recovered from c via: m = c^d mod N

SECURITY NOTES:
  - if m is too small and e is small, m^e might be < N
  - in that case, just take e-th root of c (no modular arithmetic needed)
  - proper padding (like OAEP) prevents this

ATTACKS RECOVERING m:
  - lowe: recovers m from multiple broadcasts
  - cmod: recovers m from common modulus usage
  - all factorization attacks ultimately aim to find d and decrypt m
)";
        } else if (cmd == "a") {
            std::cout << R"(
a - Linear Coefficient (Coppersmith Linear Mode)
===============================================

WHAT IS IT:
  - the coefficient of x in the congruence a*x + b ≡ 0 (mod n)
  - must be considered modulo n (effectively a mod n)

ROLE IN ATTACK:
  - if gcd(a, n) = 1 you can directly compute x ≡ -b * a^{-1} (mod n)
  - coppersmith / lattice step helpful only when that root is small < bound
  - if gcd(a, n) ≠ 1 that gcd may leak a factor of n (check!)

TIPS:
  - always compute g = gcd(a, n). If g > 1 you immediately have a factor.
  - reduce a modulo n before building a lattice.
)";
        } else if (cmd == "b") {
            std::cout << R"(
b - Constant Term (Coppersmith Linear Mode)
=========================================

WHAT IS IT:
  - the constant term in the congruence a*x + b ≡ 0 (mod n)
  - interpreted modulo n

ROLE IN ATTACK:
  - combined with a to form f(x) = a*x + b
  - goal: find small x such that f(x) ≡ 0 (mod n)

TIPS:
  - reduce b modulo n to keep numbers small
  - if |b| << n and a is small, the true root may also be small
)";
        } else {
            std::cout << "No help available for '" << cmd << "'\n";
            std::cout << "Try 'help' for a list of all commands.\n";
        }
    }
} // namespace help

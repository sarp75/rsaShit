#include "pminus1.hpp"
#include <sstream>
#include <vector>
#include <algorithm>

/*
 * Pollard's p-1 factorization (Stage 1 + optional Stage 2)
 * Stage 1 finds a factor when p-1 is B1-smooth.
 * Stage 2 extends when p-1 has a large prime factor between B1 and B2 with remaining part B1-smooth.
 */

// sieve primes up to limit
static std::vector<unsigned> primes_up_to(unsigned long long limit) {
    std::vector<unsigned> primes;
    if (limit < 2ULL) return primes;
    std::vector<bool> sieve(limit + 1ULL, true);
    sieve[0] = false; sieve[1] = false;
    for (unsigned long long i = 2ULL; i * i <= limit; ++i) {
        if (sieve[i]) {
            for (unsigned long long j = i * i; j <= limit; j += i) sieve[j] = false;
        }
    }
    for (unsigned long long i = 2ULL; i <= limit; ++i) if (sieve[i]) primes.push_back(static_cast<unsigned>(i));
    return primes;
}

static unsigned long long max_prime_power_leq(unsigned p, unsigned long long B) {
    unsigned long long pk = p;
    while (pk * p <= B) pk *= p;
    return pk;
}

PMinus1Result pollards_pminus1(const BigInt &n,
                               unsigned long long B1,
                               unsigned long long max_a_trials,
                               unsigned long long B2) {
    PMinus1Result r; std::ostringstream log;
    BigInt one(static_cast<uint64_t>(1));
    BigInt two(static_cast<uint64_t>(2));

    if (n.is_zero()) { r.log = "n=0"; return r; }
    if ((n % two).is_zero()) { r.success = true; r.factor = two; r.log = "even n"; return r; }

    unsigned long long prime_limit = std::max(B1, B2);
    auto primes = primes_up_to(prime_limit);

    unsigned bases[] = {2,3,5,7,11,13,17,19,23};
    unsigned tried = 0;
    for (unsigned bi = 0; bi < sizeof(bases)/sizeof(bases[0]) && tried < max_a_trials; ++bi, ++tried) {
        BigInt a(static_cast<uint64_t>(bases[bi]));
        a %= n; if (a.is_zero()) continue;

        // stage 1 powering
        for (unsigned p : primes) {
            if ((unsigned long long)p > B1) break;
            unsigned long long pk = max_prime_power_leq(p, B1);
            BigInt e_pk(static_cast<uint64_t>(pk));
            a = BigInt::powm(a, e_pk, n);
        }
        BigInt g = BigInt::gcd(a - one, n);
        if (g != one && g != n) { r.success = true; r.factor = g; log << "stage1 base=" << bases[bi] << " B1=" << B1; r.log = log.str(); return r; }

        // stage 2 optional
        if (B2 > B1) {
            // simple stage 2: for each prime q in (B1, B2] test gcd(a^q - 1, n)
            for (unsigned p : primes) {
                if ((unsigned long long)p <= B1) continue;
                if ((unsigned long long)p > B2) break;
                BigInt exp_p(static_cast<uint64_t>(p));
                BigInt a_q = BigInt::powm(a, exp_p, n);
                BigInt g2 = BigInt::gcd(a_q - one, n);
                if (g2 != one && g2 != n) { r.success = true; r.factor = g2; log << "stage2 base=" << bases[bi] << " B1=" << B1 << " B2=" << B2 << " prime=" << p; r.log = log.str(); return r; }
            }
        }
    }

    log << "no factor found (p-1) B1=" << B1;
    if (B2 > B1) log << " B2=" << B2; log << " trials=" << max_a_trials;
    r.log = log.str();
    return r;
}

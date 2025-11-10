#include "rho.hpp"
#include <sstream>

/*
 * Symbol of rho is "ρ", a bit disappointing.
 * Pollard's rho algorithm for integer factorization.
 * Bob uses a semi-prime N = p*q where p,q are not too large.
 * Pollard's rho tries to find a non-trivial factor of N using a pseudo-random
 * sequence and the birthday paradox.
 *
 * The algorithm uses a function f(x) = (x^2 + c) mod N for some constant c.
 * We iterate x_i = f(x_{i-1}) and detect cycles using Floyd's tortoise-hare.
 * When we find a cycle, gcd(|x_slow - x_fast|, N) might give us a factor.
 */

RhoResult rho_attack(const BigInt &n, unsigned long long max_iters) {
    RhoResult rr;
    std::ostringstream log;

    // trivial checks shiii
    BigInt two(static_cast<uint64_t>(2));
    BigInt one(static_cast<uint64_t>(1));

    if (n.is_zero() || n == one) {
        log << "n must be > 1";
        rr.log = log.str();
        return rr;
    }

    // check if n is even
    if (n.is_even()) {
        rr.success = true;
        rr.factor = two;
        log << "n is even, factor=2";
        rr.log = log.str();
        return rr;
    }

    // try multiple c values with different starting points
    unsigned long long iters_per_attempt = max_iters / 60; // 20 c values * 3 starts
    if (iters_per_attempt < 50000) iters_per_attempt = 50000;

    for (unsigned c_val = 1; c_val <= 20; c_val++) {
        BigInt c(static_cast<uint64_t>(c_val));

        // try different starting points for each c
        for (unsigned start_val = 2; start_val <= 4; start_val++) {
            BigInt x(static_cast<uint64_t>(start_val));
            BigInt y = x;

            for (unsigned long long iter = 1; iter <= iters_per_attempt; iter++) {
                // tortoise: x = f(x) = x^2 + c mod n
                x = (x * x + c) % n;

                // hare: y = f(f(y))
                y = (y * y + c) % n;
                y = (y * y + c) % n;

                // compute gcd(|x - y|, n)
                BigInt diff = (x > y) ? (x - y) : (y - x);

                // skip zero differences
                if (diff.is_zero()) continue;

                BigInt d = BigInt::gcd(diff, n);

                if (d != one && d != n) {
                    // found a non-trivial factor shiiiii
                    rr.success = true;
                    rr.factor = d;
                    log << "found factor after " << iter << " iterations (c=" << c_val << ", start=" << start_val <<
                            ")";
                    rr.log = log.str();
                    return rr;
                }

                if (d == n) {
                    // hit a cycle that gives us n, try next combination
                    break;
                }
            }
        }
    }

    log << "no factor found after trying multiple c values";
    rr.log = log.str();
    return rr;
}

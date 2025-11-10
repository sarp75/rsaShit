#include "fermat.hpp"
#include <sstream>

/*
 * Who is Fermat? Doesn't matter:
 * Fermat factorization: for N = p*q with p and q close
 * Bob accidentally chose p and q close, so we can use Fermat's method to factor N.
 * Idea: N = a^2 - b^2 = (a-b)(a+b)
 * So if we can find a and b such that a^2 - b^2 = N, we have factors.
 * Start with a = ceil(sqrt(N)), then check if a^2 - N is a perfect square b^2.
 * If not, increment a and repeat, up to max_iters.
 */

static bool is_perfect_square(const BigInt &x, BigInt &root) {
    if(x.is_zero()) { root = BigInt(static_cast<uint64_t>(0)); return true; }
    BigInt r = BigInt::nth_root_floor(x, 2);
    if(r * r == x) { root = r; return true; }
    return false;
}

FermatResult fermat_factor(const BigInt &n, unsigned long long max_iters) {
    FermatResult fr; std::ostringstream log;
    // trivial checks
    BigInt two(static_cast<uint64_t>(2));
    if(n.is_zero()) { log << "n=0"; fr.log = log.str(); return fr; }
    if((n % two).is_zero()) { fr.success=true; fr.p=two; fr.q=n/two; log<<"even n"; fr.log=log.str(); return fr; }

    // a = ceil(sqrt(n))
    BigInt a = BigInt::nth_root_floor(n, 2);
    if(a*a < n) a += BigInt(static_cast<uint64_t>(1));
    BigInt one(static_cast<uint64_t>(1));
    for(unsigned long long i=0;i<max_iters;i++) {
        BigInt x = a*a - n; // candidate square
        BigInt b; // root holder
        if(!x.is_zero()) {
            if(is_perfect_square(x, b)) {
                BigInt p = a - b;
                BigInt q = a + b;
                if(p*q == n) {
                    fr.success=true; fr.p=p; fr.q=q; log<<"found after "<<i<<" iterations"; fr.log=log.str(); return fr; }
            }
        } else { // a*a == n rare perfect square case
            fr.success=true; fr.p=a; fr.q=a; log<<"n is perfect square"; fr.log=log.str(); return fr;
        }
        a += one; // next a
    }
    log<<"not found within iters="<<max_iters; fr.log=log.str(); return fr;
}


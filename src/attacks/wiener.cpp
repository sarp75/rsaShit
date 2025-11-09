#include "wiener.hpp"
#include <vector>
#include <sstream>

/*
 * Wiener does not have a small dih :(
 * Wiener attack for small d private exponent.
 * Bob tries to use RSA with small d, i.e. d < 1/3 * N^(1/4)
 * Given public (e,n), we build the continued fraction expansion of e/n
 * and the convergents p_i/q_i. For some i, q_i = d.
 * We test each convergent candidate until we find valid p,q.
 */

// internal helpers using 128-bit only (demo scale)
static std::vector<BigInt> continued_fraction(const BigInt &e, const BigInt &n) {
    std::vector<BigInt> cf;
    BigInt num = e; BigInt den = n;
    while(!den.is_zero()) {
        BigInt q = num / den;
        cf.push_back(q);
        BigInt r = num % den;
        num = den; den = r;
        if(cf.size() > 128) break; // safety bound
    }
    return cf;
}

// helper to build continued fraction terms for ratio a/b
static std::vector<BigInt> cf_expand(const BigInt &a, const BigInt &b) {
    std::vector<BigInt> cf;
    BigInt num = a; BigInt den = b;
    while(!den.is_zero()) {
        BigInt q = num / den;
        cf.push_back(q);
        BigInt r = num % den;
        num = den; den = r;
        if(cf.size() > 256) break; // safety bound
    }
    return cf;
}

struct ConvPair { BigInt num; BigInt den; }; // num/den approximates ratio

static std::vector<ConvPair> build_convergents(const std::vector<BigInt>& cf) {
    std::vector<ConvPair> out; out.reserve(cf.size());
    BigInt p_minus2(uint64_t(0)); BigInt p_minus1(uint64_t(1));
    BigInt q_minus2(uint64_t(1)); BigInt q_minus1(uint64_t(0));
    for(size_t i=0;i<cf.size(); ++i) {
        BigInt a = cf[i];
        BigInt p_i = a * p_minus1 + p_minus2;
        BigInt q_i = a * q_minus1 + q_minus2;
        out.push_back(ConvPair{p_i, q_i});
        p_minus2 = p_minus1; p_minus1 = p_i;
        q_minus2 = q_minus1; q_minus1 = q_i;
    }
    return out;
}

static bool is_perfect_square(const BigInt &x, BigInt &root_out) {
    if(x.is_zero()) { root_out = BigInt(uint64_t(0)); return true; }
    BigInt r = BigInt::nth_root_floor(x, 2);
    if(r * r == x) { root_out = r; return true; }
    return false;
}

static bool try_convergents(const std::vector<ConvPair>& convs, const BigInt &n, const BigInt &e, WienerResult &res, std::ostringstream &log, const char *tag) {
    BigInt one(uint64_t(1)); BigInt two(uint64_t(2)); BigInt four(uint64_t(4));
    for(size_t i=0;i<convs.size(); ++i) {
        const auto &c = convs[i];
        BigInt k = c.num; // numerator
        BigInt d = c.den; // denominator candidate
        if(d.is_zero() || k.is_zero()) continue;
        // (e*d - 1) divisible by k ?
        BigInt ed_minus_1 = e * d - one;
        if(!(ed_minus_1 % k).is_zero()) continue;
        BigInt phi = ed_minus_1 / k;
        if(!(phi < n)) continue;
        BigInt b = n - phi + one; // p+q
        BigInt bb = b * b;
        BigInt four_n = four * n;
        if(!(bb >= four_n)) continue; // avoid unsigned underflow
        BigInt disc = bb - four_n;
        BigInt zero(uint64_t(0));
        if(disc == zero) continue; // unlikely
        BigInt sqrt_disc(uint64_t(0));
        if(!is_perfect_square(disc, sqrt_disc)) continue;
        // parity check: (b + sqrt_disc) must be even
        BigInt sum = b + sqrt_disc;
        if(!((sum % two).is_zero())) continue;
        BigInt p = (b + sqrt_disc) / two;
        BigInt q = (b - sqrt_disc) / two;
        if(p * q == n) {
            res.success = true; res.p = p; res.q = q; res.d = d;
            log << "hit(" << tag << ") i=" << i << " k=" << k.to_dec() << " d=" << d.to_dec();
            return true;
        }
        if(i < 40) { // limited logging to avoid spam
            log << "cand(" << tag << ":i=" << i << ") k=" << k.to_dec() << " d=" << d.to_dec() << ";";
        }
    }
    return false;
}

WienerResult wiener_attack(const BigInt &n, const BigInt &e) {
    WienerResult res; std::ostringstream log;
    // we test cf of e/n and n/e (numerical stability) and stop on first success
    auto cf_en = cf_expand(e, n);
    auto convs_en = build_convergents(cf_en);
    log << "cf(e/n)=" << cf_en.size() << " convs=" << convs_en.size() << ";";
    if(try_convergents(convs_en, n, e, res, log, "e/n")) {
        res.log = log.str(); return res;
    }
    auto cf_ne = cf_expand(n, e);
    auto convs_ne = build_convergents(cf_ne);
    log << " cf(n/e)=" << cf_ne.size() << " convs=" << convs_ne.size() << ";";
    try_convergents(convs_ne, n, e, res, log, "n/e");
    if(!res.success) log << " no wiener small-d found";
    res.log = log.str();
    return res;
}

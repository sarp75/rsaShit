#include "common_modulus.hpp"
#include <sstream>

/*
 * Do not share your modulus with your family.
 * Common modulus attack: given RSA under same modulus n and two public exponents e1,e2 with their ciphertexts
 * Bob sends the same message m under the same modulus n but with two different public exponents e1 and e2,
 * resulting in ciphertexts c1 and c2. If e1 and e2 are coprime,
 * an attacker who intercepts both ciphertexts can exploit the common modulus to recover the original message m.
 * This is done using the Extended Euclidean Algorithm to find integers a and b such that a*e1 + b*e2 = 1.
 * The attacker can then compute m as m ≡ c1^a * c2^b mod n.
 */

CommonModulusResult common_modulus_attack(const BigInt &n,
                                          const BigInt &e1,
                                          const BigInt &e2,
                                          const BigInt &c1,
                                          const BigInt &c2) {
    CommonModulusResult r;
    std::ostringstream log;
    BigInt one(static_cast<uint64_t>(1));
    if (BigInt::gcd(e1, e2) != one) {
        log << "gcd(e1,e2) != 1";
        r.log = log.str();
        return r;
    }

    // allocate real mpz_ts (no nullptr)
    mpz_t g, a, b, ge1, ge2;
    mpz_inits(g, a, b, ge1, ge2, NULL); // NULL terminator for C API
    mpz_set(ge1, e1.raw());
    mpz_set(ge2, e2.raw());

    // gcdext fills g, a, b so that a*e1 + b*e2 = g (should be 1)
    mpz_gcdext(g, a, b, ge1, ge2);

    // check gcd==1
    if (mpz_cmp_ui(g, 1) != 0) {
        log << "gcd(e1,e2) != 1 (g=";
        char *gs = mpz_get_str(nullptr, 10, g);
        log << gs << ")";
        free(gs);
        mpz_clears(g, a, b, ge1, ge2, NULL);
        r.log = log.str();
        return r;
    }

    // part1 = c1^a (or inverse if a<0)
    BigInt part1(one);
    BigInt part2(one);

    if (mpz_sgn(a) >= 0) {
        BigInt a_pos(one);
        mpz_set(a_pos.raw(), a); // copy mpz -> BigInt
        part1 = BigInt::powm(c1, a_pos, n);
    } else {
        mpz_t a_abs;
        mpz_init(a_abs);
        mpz_neg(a_abs, a);
        BigInt a_pos(one);
        mpz_set(a_pos.raw(), a_abs);
        auto inv_c1 = BigInt::mod_inverse(c1, n);
        if (!inv_c1) {
            log << "inverse(c1) failed";
            mpz_clears(g, a, b, ge1, ge2, a_abs, NULL);
            r.log = log.str();
            return r;
        }
        part1 = BigInt::powm(*inv_c1, a_pos, n);
        mpz_clear(a_abs);
    }

    // part2 = c2^b (or inverse if b<0)
    if (mpz_sgn(b) >= 0) {
        BigInt b_pos(one);
        mpz_set(b_pos.raw(), b);
        part2 = BigInt::powm(c2, b_pos, n);
    } else {
        mpz_t b_abs;
        mpz_init(b_abs);
        mpz_neg(b_abs, b);
        BigInt b_pos(one);
        mpz_set(b_pos.raw(), b_abs);
        auto inv_c2 = BigInt::mod_inverse(c2, n);
        if (!inv_c2) {
            log << "inverse(c2) failed";
            mpz_clears(g, a, b, ge1, ge2, b_abs, NULL);
            r.log = log.str();
            return r;
        }
        part2 = BigInt::powm(*inv_c2, b_pos, n);
        mpz_clear(b_abs);
    }

    r.m = (part1 * part2) % n;
    r.success = true;
    log << "recovered m";

    mpz_clears(g, a, b, ge1, ge2, NULL);
    r.log = log.str();
    return r;
}

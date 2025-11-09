#pragma once

#include "bigint.hpp"

struct RSAKey {
    BigInt n;
    BigInt e;
    BigInt d;
    BigInt p;
    BigInt q;

    bool has_private() const { return !p.is_zero() && !q.is_zero(); }
    void attempt_compute_d();

    static RSAKey from_pq(const BigInt &p_, const BigInt &q_, const BigInt &e_);
};

namespace RSAOps {
    inline BigInt encrypt(const BigInt &m, const RSAKey &k) { return BigInt::powm(m, k.e, k.n); }
    inline BigInt decrypt(const BigInt &c, const RSAKey &k) { return BigInt::powm(c, k.d, k.n); }
}

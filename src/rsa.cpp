#include "rsa.hpp"

void RSAKey::attempt_compute_d() {
    if(p.is_zero() || q.is_zero() || e.is_zero()) return;
    BigInt one(uint64_t(1));
    BigInt phi = (p - one) * (q - one);
    auto inv = BigInt::mod_inverse(e, phi);
    if(inv) d = *inv;
}

RSAKey RSAKey::from_pq(const BigInt &p_, const BigInt &q_, const BigInt &e_) {
    RSAKey k; k.p = p_; k.q = q_; k.e = e_; k.n = p_ * q_; k.attempt_compute_d(); return k;
}

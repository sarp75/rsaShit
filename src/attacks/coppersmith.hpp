#pragma once

#include "../bigint.hpp"
#include <string>

struct CoppersmithResult {
    bool success{false};
    BigInt root{static_cast<uint64_t>(0)};
    std::string log;
};

/*
 * Coppersmith's method for finding small roots of univariate modular polynomial equations.
 * Given f(x) = a*x + b (mod N), finds small root x0 such that f(x0) ≡ 0 (mod N).
 *
 * @param a - coefficient of x
 * @param b - constant term
 * @param n - modulus
 * @param beta - parameter controlling root bound (0 < beta <= 1)
 * @param epsilon - LLL parameter (typically 0.01)
 * @return CoppersmithResult with success flag and root if found
 */
CoppersmithResult coppersmith_univariate_linear(
    const BigInt &a,
    const BigInt &b,
    const BigInt &n,
    double beta = 0.5,
    double epsilon = 0.01
);

/*
 * Simplified Coppersmith attack for small public exponent e and partial message recovery.
 * Given C = (M + x)^e mod N where M is partially known and x is small unknown part.
 *
 * @param c - ciphertext
 * @param e - public exponent (small, typically 3)
 * @param n - modulus
 * @param m_high - known high bits of message
 * @param unknown_bits - number of unknown low bits
 * @return CoppersmithResult with recovered full message if successful
 */
CoppersmithResult coppersmith_small_e_partial_msg(
    const BigInt &c,
    unsigned e,
    const BigInt &n,
    const BigInt &m_high,
    size_t unknown_bits
);


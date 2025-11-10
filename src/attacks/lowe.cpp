#include "lowe.hpp"
#include "../rsa.hpp"
#include <optional>
#include <sstream>

/*
 * Chinese Remainder Theorem sounds so fancy
 * Håstad broadcast attack for a low e.
 * Bob sends the same message m to e different recipients,
 * each with their own modulus n_i, but all using the same small exponent e.
 * An attacker who intercepts all e ciphertexts c_i = m^e mod n_i
 * can use the Chinese Remainder Theorem to reconstruct m^e mod N,
 * where N = n_1 * n_2 * ... * n_e.
 * Since m^e < N (if m < min(n_i)), the attacker can then compute the integer e-th root of m^e to recover m.
 */

static BigInt crt(const std::vector<BigInt>& residues, const std::vector<BigInt>& moduli) {
    BigInt N(static_cast<uint64_t>(1));
    for(const auto& m : moduli) N *= m;
    BigInt x(static_cast<uint64_t>(0));
    for(size_t i=0;i<moduli.size();++i) {
        BigInt Mi = N / moduli[i];
        auto inv = BigInt::mod_inverse(Mi % moduli[i], moduli[i]);
        if(!inv) throw std::runtime_error("crt inverse fail");
        x += Mi * (*inv) * residues[i];
        x %= N;
    }
    return x;
}

LoweResult low_e_broadcast(const std::vector<LoweTarget>& targets, unsigned e) {
    LoweResult result{false, BigInt(static_cast<uint64_t>(0)), ""};
    if(targets.size() < e) { result.log = "need at least e targets"; return result; }
    std::vector<BigInt> residues; residues.reserve(targets.size());
    std::vector<BigInt> moduli; moduli.reserve(targets.size());
    for(const auto &t : targets) {
        residues.push_back(t.c);
        moduli.push_back(t.n);
    }
    try {
        BigInt combined = crt(residues, moduli);
        BigInt m = BigInt::nth_root_floor(combined, e);
        BigInt exact(static_cast<uint64_t>(1));
        for(unsigned i=0;i<e;i++) exact *= m;
        if(exact == combined) {
            result.success = true;
            result.m = m;
            std::ostringstream oss; oss << "recovered m with bitlen=" << m.bit_length();
            result.log = oss.str();
        } else {
            result.log = "nth root mismatch";
        }
    } catch(const std::exception &ex) {
        result.log = std::string("error: ") + ex.what();
    }
    return result;
}

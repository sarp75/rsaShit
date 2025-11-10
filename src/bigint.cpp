#include "bigint.hpp"
#include <stdexcept>

// parse bigint from string (hex with 0x prefix or decimal)
void BigInt::parse_from_string(const std::string &s) {
    if (s.empty()) {
        mpz_set_ui(v_, 0);
        return;
    }

    // check for hex prefix
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        if (mpz_set_str(v_, s.c_str() + 2, 16) != 0) {
            throw std::invalid_argument("invalid hex string");
        }
    } else {
        // assume decimal
        if (mpz_set_str(v_, s.c_str(), 10) != 0) {
            throw std::invalid_argument("invalid decimal string");
        }
    }
}

// convert to decimal string
std::string BigInt::to_dec() const {
    char* str = mpz_get_str(nullptr, 10, v_);
    std::string result(str);
    free(str);
    return result;
}

// convert to hex string
std::string BigInt::to_hex(bool prefix) const {
    char* str = mpz_get_str(nullptr, 16, v_);
    std::string result = prefix ? "0x" : "";
    result += str;
    free(str);
    return result;
}

// compute floor(x^(1/n)) - the n-th root of x
BigInt BigInt::nth_root_floor(const BigInt &x, unsigned int n) {
    if (n == 0) throw std::invalid_argument("nth_root_floor: n must be > 0");
    if (x.is_zero()) return BigInt(static_cast<uint64_t>(0));

    BigInt result;
    mpz_root(result.v_, x.v_, n);
    return result;
}


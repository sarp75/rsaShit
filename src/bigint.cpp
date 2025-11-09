#include "bigint.hpp"

#include <algorithm>
#include <cctype>

void BigInt::parse_from_string(const std::string &s_in) {
    std::string s = s_in; if(s.empty()) { mpz_set_ui(raw(),0); return; }
    // trim
    size_t a=0,b=s.size(); while(a<b && std::isspace((unsigned char)s[a])) a++; while(b>a && std::isspace((unsigned char)s[b-1])) b--; s = s.substr(a,b-a);
    if(s.rfind("0x",0)==0 || s.rfind("0X",0)==0) {
        std::string hex = s.substr(2);
        mpz_set_ui(raw(),0);
        mpz_set_str(raw(), hex.c_str(), 16);
        return;
    }
    // decimal
    mpz_set_str(raw(), s.c_str(), 10);
}

std::string BigInt::to_dec() const {
    char *c = mpz_get_str(nullptr,10, raw());
    std::string out(c); free(c); return out;
}

std::string BigInt::to_hex(bool prefix) const {
    char *c = mpz_get_str(nullptr,16, raw());
    std::string out(c); free(c);
    if(out.empty()) out="0";
    if(prefix) return std::string("0x") + out;
    return out;
}

BigInt BigInt::nth_root_floor(const BigInt &x, unsigned int n) {
    if(n==0) throw std::invalid_argument("n must be >0");
    if(x.is_zero()) return BigInt(uint64_t(0));
    if(n==1) return x;
    // binary search using mpz
    BigInt low(uint64_t(1));
    BigInt high = x; // overestimate
    BigInt one(uint64_t(1));
    auto pow_limit = [&](const BigInt &a){ BigInt p(uint64_t(1)); for(unsigned i=0;i<n;i++){ p*=a; if(p > x) return p; } return p; };
    while(low < high) {
        BigInt mid = (low + high + one) / BigInt(uint64_t(2));
        BigInt pm = pow_limit(mid);
        if(pm <= x) low = mid; else high = mid - one;
    }
    return low;
}

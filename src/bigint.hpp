#pragma once

#include <gmp.h>
#include <string>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <vector>

class BigInt {
public:
    BigInt() { mpz_init(v_); }
    BigInt(const BigInt &o) { mpz_init_set(v_, o.v_); }
    BigInt(BigInt &&o) noexcept { mpz_init(v_); mpz_swap(v_, o.v_); }
    explicit BigInt(uint64_t x) { mpz_init_set_ui(v_, static_cast<unsigned long>(x)); }
    explicit BigInt(int x) { mpz_init_set_si(v_, x); }
    explicit BigInt(const std::string &s) { mpz_init(v_); parse_from_string(s); }
    ~BigInt() { mpz_clear(v_); }

    BigInt& operator=(const BigInt &o) { if(this!=&o) mpz_set(v_, o.v_); return *this; }
    BigInt& operator=(BigInt &&o) noexcept { if(this!=&o) mpz_swap(v_, o.v_); return *this; }

    // parsing helpers
    void parse_from_string(const std::string &s);

    // string conversions
    std::string to_dec() const;
    std::string to_hex(bool prefix=true) const;

    // basics
    bool is_zero() const { return mpz_cmp_ui(v_,0)==0; }
    bool is_even() const { return mpz_tstbit(v_,0)==0; }
    size_t bit_length() const { return is_zero()?0: mpz_sizeinbase(v_,2); }

    // arithmetic operators
    friend BigInt operator+(const BigInt &a, const BigInt &b) { BigInt r; mpz_add(r.v_, a.v_, b.v_); return r; }
    friend BigInt operator-(const BigInt &a, const BigInt &b) { BigInt r; mpz_sub(r.v_, a.v_, b.v_); return r; }
    friend BigInt operator*(const BigInt &a, const BigInt &b) { BigInt r; mpz_mul(r.v_, a.v_, b.v_); return r; }
    friend BigInt operator/(const BigInt &a, const BigInt &b) { BigInt r; mpz_tdiv_q(r.v_, a.v_, b.v_); return r; }
    friend BigInt operator%(const BigInt &a, const BigInt &b) { BigInt r; mpz_mod(r.v_, a.v_, b.v_); return r; }

    BigInt& operator+=(const BigInt &o) { mpz_add(v_, v_, o.v_); return *this; }
    BigInt& operator-=(const BigInt &o) { mpz_sub(v_, v_, o.v_); return *this; }
    BigInt& operator*=(const BigInt &o) { mpz_mul(v_, v_, o.v_); return *this; }
    BigInt& operator/=(const BigInt &o) { mpz_tdiv_q(v_, v_, o.v_); return *this; }
    BigInt& operator%=(const BigInt &o) { mpz_mod(v_, v_, o.v_); return *this; }

    friend bool operator==(const BigInt &a, const BigInt &b) { return mpz_cmp(a.v_, b.v_)==0; }
    friend bool operator!=(const BigInt &a, const BigInt &b) { return mpz_cmp(a.v_, b.v_)!=0; }
    friend bool operator<(const BigInt &a, const BigInt &b) { return mpz_cmp(a.v_, b.v_)<0; }
    friend bool operator>(const BigInt &a, const BigInt &b) { return mpz_cmp(a.v_, b.v_)>0; }
    friend bool operator<=(const BigInt &a, const BigInt &b) { return mpz_cmp(a.v_, b.v_)<=0; }
    friend bool operator>=(const BigInt &a, const BigInt &b) { return mpz_cmp(a.v_, b.v_)>=0; }

    // algorithms
    static BigInt gcd(const BigInt &a, const BigInt &b) { BigInt r; mpz_gcd(r.v_, a.v_, b.v_); return r; }
    static std::optional<BigInt> mod_inverse(const BigInt &a, const BigInt &m) { BigInt inv; if(mpz_invert(inv.v_, a.v_, m.v_)==0) return std::nullopt; return inv; }
    static BigInt powm(const BigInt &base, const BigInt &exp, const BigInt &mod) { BigInt r; mpz_powm(r.v_, base.v_, exp.v_, mod.v_); return r; }
    static BigInt nth_root_floor(const BigInt &x, unsigned int n);

    mpz_t& raw() { return v_; }
    const mpz_t& raw() const { return v_; }

private:
    mpz_t v_;
};

inline std::ostream& operator<<(std::ostream &os, const BigInt &x) { os << x.to_dec(); return os; }

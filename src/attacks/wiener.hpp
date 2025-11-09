#pragma once

#include "../bigint.hpp"
#include <string>

struct WienerResult {
    bool success{false};
    BigInt p{uint64_t(0)};
    BigInt q{uint64_t(0)};
    BigInt d{uint64_t(0)};
    std::string log;
};

WienerResult wiener_attack(const BigInt &n, const BigInt &e);


#pragma once

#include "../bigint.hpp"
#include <string>

struct WienerResult {
    bool success{false};
    BigInt p{static_cast<uint64_t>(0)};
    BigInt q{static_cast<uint64_t>(0)};
    BigInt d{static_cast<uint64_t>(0)};
    std::string log;
};

WienerResult wiener_attack(const BigInt &n, const BigInt &e);


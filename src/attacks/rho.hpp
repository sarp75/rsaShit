#pragma once

#include "../bigint.hpp"
#include <string>

struct RhoResult {
    bool success{false};
    BigInt factor{static_cast<uint64_t>(0)};
    std::string log;
};

RhoResult rho_attack(const BigInt &n, unsigned long long max_iters = 1000000ULL);


#pragma once

#include "../bigint.hpp"
#include <string>

struct PMinus1Result {
    bool success{false};
    BigInt factor{static_cast<uint64_t>(0)};
    std::string log;
};

PMinus1Result pollards_pminus1(const BigInt &n,
                               unsigned long long B1 = 100000ULL,
                               unsigned long long max_a_trials = 5ULL,
                               unsigned long long B2 = 0ULL);

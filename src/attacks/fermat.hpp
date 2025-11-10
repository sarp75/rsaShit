#pragma once

#include "../bigint.hpp"
#include <string>

struct FermatResult {
    bool success{false};
    BigInt p{static_cast<uint64_t>(0)};
    BigInt q{static_cast<uint64_t>(0)};
    std::string log;
};

FermatResult fermat_factor(const BigInt &n, unsigned long long max_iters = 1000000ULL);


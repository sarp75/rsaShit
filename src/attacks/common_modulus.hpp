#pragma once

#include "../bigint.hpp"
#include <string>

struct CommonModulusResult {
    bool success{false};
    BigInt m{uint64_t(0)};
    std::string log;
};

CommonModulusResult common_modulus_attack(const BigInt &n,
                                         const BigInt &e1,
                                         const BigInt &e2,
                                         const BigInt &c1,
                                         const BigInt &c2);


#pragma once

#include "../bigint.hpp"
#include <vector>
#include <string>

struct LoweTarget { BigInt n; BigInt c; };
struct LoweResult { bool success; BigInt m; std::string log; };

LoweResult low_e_broadcast(const std::vector<LoweTarget>& targets, unsigned e);


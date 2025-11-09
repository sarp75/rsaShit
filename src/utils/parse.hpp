#pragma once

#include <optional>
#include <string>
#include <vector>

struct ParsedNumber {
    bool known{false};
    std::string raw; // original trimmed input
    bool is_hex{false};
    bool is_dec{false};
};

namespace utils {
    std::optional<std::string> parse_number(const std::string &s);
    ParsedNumber parse_number_adv(const std::string &s);
}

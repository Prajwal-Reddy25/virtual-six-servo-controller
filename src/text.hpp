#pragma once

#include <charconv>
#include <cmath>
#include <string_view>
#include <vector>

namespace virtual_servo::detail {

inline bool digit(char value) { return value >= '0' && value <= '9'; }

inline std::vector<std::string_view> tokens(std::string_view line) {
    std::vector<std::string_view> result;
    std::size_t begin = line.find_first_not_of(" \t");
    while (begin != std::string_view::npos) {
        const auto end = line.find_first_of(" \t", begin);
        result.push_back(line.substr(begin, end == std::string_view::npos ? end : end - begin));
        begin = end == std::string_view::npos ? end : line.find_first_not_of(" \t", end);
    }
    return result;
}

// Enforce the protocol's decimal grammar before locale-independent conversion.
// from_chars alone would also accept hexadecimal-looking prefixes and does not
// accept a leading '+'. Full consumption and explicit grammar remove ambiguity.
inline bool decimal(std::string_view token, double &value) {
    std::size_t cursor = 0;
    if (cursor < token.size() && (token[cursor] == '+' || token[cursor] == '-')) {
        ++cursor;
    }
    std::size_t digits = 0;
    while (cursor < token.size() && digit(token[cursor])) {
        ++cursor;
        ++digits;
    }
    if (cursor < token.size() && token[cursor] == '.') {
        ++cursor;
        while (cursor < token.size() && digit(token[cursor])) {
            ++cursor;
            ++digits;
        }
    }
    if (digits == 0) {
        return false;
    }
    if (cursor < token.size() && (token[cursor] == 'e' || token[cursor] == 'E')) {
        ++cursor;
        if (cursor < token.size() && (token[cursor] == '+' || token[cursor] == '-')) {
            ++cursor;
        }
        const auto exponent_begin = cursor;
        while (cursor < token.size() && digit(token[cursor])) {
            ++cursor;
        }
        if (cursor == exponent_begin) {
            return false;
        }
    }
    if (cursor != token.size()) {
        return false;
    }
    if (token.front() == '+') {
        token.remove_prefix(1);
    }
    const auto result = std::from_chars(token.data(), token.data() + token.size(), value,
                                        std::chars_format::general);
    return result.ec == std::errc{} && result.ptr == token.data() + token.size() &&
           std::isfinite(value);
}

} // namespace virtual_servo::detail

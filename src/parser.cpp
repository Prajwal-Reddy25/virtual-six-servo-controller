#include "virtual_servo/parser.hpp"

#include "text.hpp"

#include <charconv>

namespace virtual_servo {

ParseResult parse_command(std::string_view line, const JointConfigs &configs) {
    if (line.size() > kMaxLineBytes) {
        return ParseError{ErrorCode::LineTooLong};
    }
    for (unsigned char byte : line) {
        if (byte > 127) {
            return ParseError{ErrorCode::BadEncoding};
        }
    }
    const auto tokens = detail::tokens(line);
    if (tokens.empty()) {
        return ParseError{ErrorCode::Empty};
    }

    Command command{CommandType::Get};
    std::size_t expected = 1;
    if (tokens[0] == "SET") {
        command.type = CommandType::Set;
        expected = kJointCount + 1;
    } else if (tokens[0] == "GET") {
        command.type = CommandType::Get;
    } else if (tokens[0] == "STEP") {
        command.type = CommandType::Step;
        expected = 2;
    } else if (tokens[0] == "STOP") {
        command.type = CommandType::Stop;
    } else if (tokens[0] == "QUIT") {
        command.type = CommandType::Quit;
    } else {
        return ParseError{ErrorCode::UnknownCommand};
    }
    if (tokens.size() != expected) {
        return ParseError{ErrorCode::Arity};
    }

    if (command.type == CommandType::Set) {
        // Convert everything before checking limits; never commit partial input.
        for (std::size_t i = 0; i < kJointCount; ++i) {
            if (!detail::decimal(tokens[i + 1], command.targets_deg[i])) {
                return ParseError{ErrorCode::BadNumber};
            }
        }
        for (std::size_t i = 0; i < kJointCount; ++i) {
            if (command.targets_deg[i] < configs[i].lower_deg ||
                command.targets_deg[i] > configs[i].upper_deg) {
                return ParseError{ErrorCode::JointLimit, i + 1};
            }
        }
    } else if (command.type == CommandType::Step) {
        const auto token = tokens[1];
        for (char byte : token) {
            if (!detail::digit(byte)) {
                return ParseError{ErrorCode::BadNumber};
            }
        }
        const auto result =
            std::from_chars(token.data(), token.data() + token.size(), command.step_ms);
        if (result.ec != std::errc{} || result.ptr != token.data() + token.size()) {
            return ParseError{ErrorCode::BadNumber};
        }
        if (command.step_ms < 1 || command.step_ms > 1000) {
            return ParseError{ErrorCode::StepRange};
        }
    }
    return command;
}

} // namespace virtual_servo

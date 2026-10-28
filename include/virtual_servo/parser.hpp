#pragma once

#include "virtual_servo/joint.hpp"

#include <array>
#include <cstddef>
#include <string_view>
#include <variant>

namespace virtual_servo {

inline constexpr std::size_t kJointCount = 6;
inline constexpr std::size_t kMaxLineBytes = 256;
using JointConfigs = std::array<JointConfig, kJointCount>;

enum class CommandType { Set, Get, Step, Stop, Quit };

struct Command {
    CommandType type;
    std::array<double, kJointCount> targets_deg{}; // Used only by Set.
    int step_ms = 0;                               // Used only by Step.
};

enum class ErrorCode {
    LineTooLong,
    BadEncoding,
    Empty,
    UnknownCommand,
    Arity,
    BadNumber,
    JointLimit,
    StepRange
};

struct ParseError {
    ErrorCode code;
    std::size_t joint_number = 0; // 1..6 for JointLimit; otherwise 0.
};

using ParseResult = std::variant<Command, ParseError>;

// Input is one complete line with LF or CRLF removed. Configs are already valid.
// Return a fully validated command or an error; never execute or print anything.
ParseResult parse_command(std::string_view line, const JointConfigs &configs);

} // namespace virtual_servo

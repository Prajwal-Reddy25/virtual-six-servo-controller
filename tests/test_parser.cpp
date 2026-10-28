#include "virtual_servo/parser.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
using namespace virtual_servo;

void require(bool condition, const char *message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void error(const std::string &line, ErrorCode code, std::size_t joint = 0,
           const JointConfigs &configs = {}) {
    const auto result = parse_command(line, configs);
    const auto *failure = std::get_if<ParseError>(&result);
    if (!failure || failure->code != code || failure->joint_number != joint) {
        throw std::runtime_error("unexpected result for: " + line);
    }
}

Command success(const std::string &line, CommandType type, const JointConfigs &configs = {}) {
    const auto result = parse_command(line, configs);
    const auto *command = std::get_if<Command>(&result);
    require(command && command->type == type, "expected successful command");
    return *command;
}
} // namespace

int main() {
    try {
        success("GET", CommandType::Get);
        success(" \tSTOP\t ", CommandType::Stop);
        success("QUIT", CommandType::Quit);
        const auto set = success("SET +90 .5 1. 9e1 0 180", CommandType::Set);
        require(set.targets_deg == std::array<double, 6>{90, .5, 1, 90, 0, 180},
                "converted angles differ");
        require(success("STEP 0001", CommandType::Step).step_ms == 1, "minimum step");
        require(success("STEP 1000", CommandType::Step).step_ms == 1000, "maximum step");
        success("SET -0 1e-300 90.0 1E+2 180 0", CommandType::Set);

        error("", ErrorCode::Empty);
        error(" \t ", ErrorCode::Empty);
        error("get", ErrorCode::UnknownCommand);
        error("HELLO extra", ErrorCode::UnknownCommand);
        for (const auto *line : {"GET x", "STOP 1", "QUIT x", "STEP", "STEP 1 2", "SET", "SET 1 2",
                                 "SET 1 2 3 4 5 6 7"}) {
            error(line, ErrorCode::Arity);
        }
        for (const auto *number :
             {"nan", "NaN", "inf", "-inf", "infinity", "1e9999", "1e-9999", "90oops", "0x10",
              "0x1p2", "+", "-", ".", "1e", "1e+", "1,5", "1_0", "--1", "1..0"}) {
            error(std::string("SET ") + number + " 90 90 90 90 90", ErrorCode::BadNumber);
        }
        for (const auto *number : {"-1", "+1", "1.0", "1e2", "nan", "1x", "2147483648",
                                   "9999999999999999999999999999999"}) {
            error(std::string("STEP ") + number, ErrorCode::BadNumber);
        }
        error("STEP 0", ErrorCode::StepRange);
        error("STEP 1001", ErrorCode::StepRange);
        error("STEP 2147483647", ErrorCode::StepRange);
        error("SET -0.001 90 90 90 90 90", ErrorCode::JointLimit, 1);
        error("SET 90 90 90 90 90 180.000001", ErrorCode::JointLimit, 6);
        error("SET -1 90oops 90 90 90 181", ErrorCode::BadNumber);
        error("SET -1 90 90 90 90 181", ErrorCode::JointLimit, 1);
        error("SET nan", ErrorCode::Arity);
        error(std::string(257, 'x') + "\xff", ErrorCode::LineTooLong);
        error("GET\xff", ErrorCode::BadEncoding);
        error("GET\v", ErrorCode::UnknownCommand);
        error("SET 90\r 90 90 90 90 90", ErrorCode::BadNumber);
        error(std::string("GET\0", 4), ErrorCode::UnknownCommand);
        error(std::string("SET 90\0 90 90 90 90 90", 21), ErrorCode::BadNumber);
        success(std::string(253, ' ') + "GET", CommandType::Get);

        JointConfigs configs{};
        configs[2] = {-30, 45, 0, 20};
        success("SET 90 90 -30 90 90 90", CommandType::Set, configs);
        success("SET 90 90 45 90 90 90", CommandType::Set, configs);
        error("SET 90 90 45.000001 90 90 90", ErrorCode::JointLimit, 3, configs);
        std::cout
            << "PASS: valid commands, numeric grammar, boundaries, precedence, custom limits\n";
    } catch (const std::exception &exception) {
        std::cerr << "FAIL: " << exception.what() << '\n';
        return 1;
    }
}

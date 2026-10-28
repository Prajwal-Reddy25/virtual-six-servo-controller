#include "virtual_servo/controller.hpp"

#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace virtual_servo {

std::string error_response(ParseError error) {
    switch (error.code) {
    case ErrorCode::LineTooLong:
        return "ERR LINE_TOO_LONG";
    case ErrorCode::BadEncoding:
        return "ERR BAD_ENCODING";
    case ErrorCode::Empty:
        return "ERR EMPTY";
    case ErrorCode::UnknownCommand:
        return "ERR UNKNOWN_COMMAND";
    case ErrorCode::Arity:
        return "ERR ARITY";
    case ErrorCode::BadNumber:
        return "ERR BAD_NUMBER";
    case ErrorCode::JointLimit:
        return "ERR JOINT_LIMIT " + std::to_string(error.joint_number);
    case ErrorCode::StepRange:
        return "ERR STEP_RANGE";
    }
    throw std::logic_error("unrecognized error code");
}

Controller::Controller(JointConfigs configs) : configs_(configs) {
    validate_configs(configs_);
    for (std::size_t i = 0; i < kJointCount; ++i) {
        joints_[i] = Joint(configs_[i]);
    }
}

std::string Controller::state() const {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << "STATE" << std::fixed << std::setprecision(6);
    for (const auto &joint : joints_) {
        output << ' ' << joint.position_deg();
    }
    return output.str();
}

Reply Controller::handle_line(std::string_view line) {
    const auto result = parse_command(line, configs_);
    if (const auto *error = std::get_if<ParseError>(&result)) {
        return {error_response(*error)};
    }
    const auto &command = std::get<Command>(result);
    switch (command.type) {
    case CommandType::Set:
        // Parsing has validated all targets. These assignments cannot fail.
        for (std::size_t i = 0; i < kJointCount; ++i) {
            joints_[i].set_target(command.targets_deg[i]);
        }
        return {"ACK SET"};
    case CommandType::Get:
        return {state()};
    case CommandType::Step:
        for (auto &joint : joints_) {
            joint.step(command.step_ms);
        }
        return {state()};
    case CommandType::Stop:
        for (auto &joint : joints_) {
            joint.stop();
        }
        return {"ACK STOP"};
    case CommandType::Quit:
        return {"ACK QUIT", true};
    }
    throw std::logic_error("unrecognized command type");
}

} // namespace virtual_servo

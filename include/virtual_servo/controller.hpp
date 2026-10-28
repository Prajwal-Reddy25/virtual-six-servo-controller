#pragma once

#include "virtual_servo/parser.hpp"

#include <istream>
#include <string>

namespace virtual_servo {

// Configuration errors throw std::invalid_argument before commands are accepted.
void validate_configs(const JointConfigs &configs);
JointConfigs load_configs(std::istream &input);

struct Reply {
    std::string text;
    bool quit = false;
};

std::string error_response(ParseError error);

class Controller {
  public:
    explicit Controller(JointConfigs configs = {});
    Reply handle_line(std::string_view line);

  private:
    std::string state() const;
    JointConfigs configs_;
    std::array<Joint, kJointCount> joints_{};
};

} // namespace virtual_servo

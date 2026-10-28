#include "virtual_servo/controller.hpp"

#include "text.hpp"

#include <cmath>
#include <stdexcept>

namespace virtual_servo {

void validate_configs(const JointConfigs &configs) {
    for (std::size_t i = 0; i < configs.size(); ++i) {
        const auto &config = configs[i];
        if (!std::isfinite(config.lower_deg) || !std::isfinite(config.upper_deg) ||
            !std::isfinite(config.initial_deg) || !std::isfinite(config.max_speed_deg_per_second) ||
            config.lower_deg < -360 || config.upper_deg > 360 ||
            config.lower_deg >= config.upper_deg || config.initial_deg < config.lower_deg ||
            config.initial_deg > config.upper_deg || config.max_speed_deg_per_second <= 0 ||
            config.max_speed_deg_per_second > 360) {
            throw std::invalid_argument("invalid configuration for joint " + std::to_string(i + 1));
        }
    }
}

JointConfigs load_configs(std::istream &input) {
    JointConfigs configs{};
    std::string line;
    for (auto &config : configs) {
        if (!std::getline(input, line)) {
            throw std::invalid_argument("configuration requires exactly six rows");
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const auto fields = detail::tokens(line);
        if (line.size() > kMaxLineBytes || fields.size() != 4 ||
            !detail::decimal(fields[0], config.lower_deg) ||
            !detail::decimal(fields[1], config.upper_deg) ||
            !detail::decimal(fields[2], config.initial_deg) ||
            !detail::decimal(fields[3], config.max_speed_deg_per_second)) {
            throw std::invalid_argument("configuration row requires four finite decimal numbers");
        }
    }
    if (std::getline(input, line) || input.bad() || !input.eof()) {
        throw std::invalid_argument("configuration requires exactly six rows and readable input");
    }
    validate_configs(configs);
    return configs;
}

} // namespace virtual_servo

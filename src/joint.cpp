#include "virtual_servo/joint.hpp"

#include <algorithm>

namespace virtual_servo {

Joint::Joint() : Joint(JointConfig{}) {}

Joint::Joint(JointConfig config)
    : config_(config), position_deg_(config.initial_deg), target_deg_(config.initial_deg) {}

double Joint::position_deg() const { return position_deg_; }

double Joint::target_deg() const { return target_deg_; }

void Joint::set_target(double target_deg) { target_deg_ = target_deg; }

void Joint::step(int milliseconds) {
    const double seconds = static_cast<double>(milliseconds) / 1000.0;
    const double maximum_change = config_.max_speed_deg_per_second * seconds;
    const double change = std::clamp(target_deg_ - position_deg_, -maximum_change, maximum_change);
    position_deg_ = std::clamp(position_deg_ + change, config_.lower_deg, config_.upper_deg);
}

void Joint::stop() { target_deg_ = position_deg_; }

} // namespace virtual_servo

#pragma once

namespace virtual_servo {

struct JointConfig {
    double lower_deg = 0.0;
    double upper_deg = 180.0;
    double initial_deg = 90.0;
    double max_speed_deg_per_second = 60.0;
};

// Internal motion model, not an input-validation boundary.
// Callers must supply a valid finite configuration (see PROTOCOL.md), a finite
// target within its limits, and step durations in [1, 1000] milliseconds.
// The command/configuration validation layers enforce these preconditions before
// external commands reach this model. This class does not parse or reject input.
class Joint {
  public:
    Joint();
    explicit Joint(JointConfig config);

    double position_deg() const;
    double target_deg() const;

    void set_target(double target_deg);
    void step(int milliseconds);
    void stop();

  private:
    JointConfig config_;
    double position_deg_;
    double target_deg_;
};

} // namespace virtual_servo

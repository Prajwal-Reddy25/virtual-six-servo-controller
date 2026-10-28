#include "virtual_servo/joint.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

// Explicit checks stay active in Release builds, unlike assert().
void expect_near(double actual, double expected, const char *description) {
    if (!std::isfinite(actual) || std::abs(actual - expected) > 0.000001) {
        throw std::runtime_error(std::string(description) + ": expected " +
                                 std::to_string(expected) + ", got " + std::to_string(actual));
    }
}

void initial_state() {
    virtual_servo::Joint joint;
    expect_near(joint.position_deg(), 90.0, "initial position");
    expect_near(joint.target_deg(), 90.0, "initial target");
    joint.step(1000);
    expect_near(joint.position_deg(), 90.0, "idle step");
}

void target_does_not_move_position() {
    virtual_servo::Joint joint;
    joint.set_target(150.0);
    expect_near(joint.target_deg(), 150.0, "new target");
    expect_near(joint.position_deg(), 90.0, "position before stepping");
}

void bounded_motion_both_directions() {
    virtual_servo::Joint joint;
    joint.set_target(180.0);
    joint.step(250);
    expect_near(joint.position_deg(), 105.0, "forward motion at 60 deg/s");
    joint.set_target(0.0);
    joint.step(100);
    expect_near(joint.position_deg(), 99.0, "reverse motion at 60 deg/s");
}

void reaches_target_without_overshoot() {
    virtual_servo::Joint joint;
    joint.set_target(91.0);
    joint.step(1000);
    expect_near(joint.position_deg(), 91.0, "short forward move");
    joint.set_target(90.5);
    joint.step(1000);
    expect_near(joint.position_deg(), 90.5, "short reverse move");
    joint.step(1);
    expect_near(joint.position_deg(), 90.5, "position at target");
}

void inclusive_boundaries() {
    virtual_servo::Joint joint;
    joint.set_target(180.0);
    joint.step(1000);
    joint.step(1000);
    expect_near(joint.position_deg(), 180.0, "upper boundary");
    joint.set_target(0.0);
    for (int i = 0; i < 4; ++i) {
        joint.step(1000);
    }
    expect_near(joint.position_deg(), 0.0, "lower boundary");
}

void stop_and_resume() {
    virtual_servo::Joint joint;
    joint.set_target(180.0);
    joint.step(250);
    joint.stop();
    expect_near(joint.position_deg(), 105.0, "stop preserves position");
    expect_near(joint.target_deg(), 105.0, "stop cancels target");
    joint.step(1000);
    expect_near(joint.position_deg(), 105.0, "stopped step");
    joint.set_target(60.0);
    joint.step(1000);
    expect_near(joint.position_deg(), 60.0, "motion after new target");
}

void custom_configuration_and_small_steps() {
    virtual_servo::Joint joint({10.0, 170.0, 30.0, 20.0});
    expect_near(joint.position_deg(), 30.0, "custom initial position");
    joint.set_target(100.0);
    joint.step(1);
    expect_near(joint.position_deg(), 30.02, "one millisecond");
    joint.step(999);
    expect_near(joint.position_deg(), 50.0, "one simulated second total");
}

} // namespace

int main() {
    struct TestCase {
        const char *name;
        void (*run)();
    };
    const TestCase cases[] = {
        {"initial state", initial_state},
        {"target and position separation", target_does_not_move_position},
        {"bounded bidirectional motion", bounded_motion_both_directions},
        {"no overshoot", reaches_target_without_overshoot},
        {"inclusive boundaries", inclusive_boundaries},
        {"stop and resume", stop_and_resume},
        {"custom configuration and millisecond units", custom_configuration_and_small_steps},
    };

    int failures = 0;
    for (const auto &test : cases) {
        try {
            test.run();
            std::cout << "PASS: " << test.name << '\n';
        } catch (const std::exception &error) {
            ++failures;
            std::cerr << "FAIL: " << test.name << ": " << error.what() << '\n';
        }
    }
    return failures == 0 ? 0 : 1;
}

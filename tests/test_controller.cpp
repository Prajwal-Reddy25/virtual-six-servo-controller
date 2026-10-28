#include "virtual_servo/controller.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
using namespace virtual_servo;

void expect(Controller &controller, const std::string &command, const std::string &expected) {
    const auto reply = controller.handle_line(command);
    if (reply.text != expected || reply.quit != (command == "QUIT")) {
        throw std::runtime_error(command + ": expected " + expected + ", got " + reply.text);
    }
}

void invalid_config(const std::string &text) {
    std::istringstream input(text);
    try {
        load_configs(input);
    } catch (const std::invalid_argument &) {
        return;
    }
    throw std::runtime_error("accepted invalid configuration");
}
} // namespace

int main() {
    try {
        Controller controller;
        const std::string initial =
            "STATE 90.000000 90.000000 90.000000 90.000000 90.000000 90.000000";
        const std::string moved =
            "STATE 105.000000 75.000000 105.000000 75.000000 105.000000 75.000000";
        expect(controller, "SET 180 0 180 0 180 0", "ACK SET");
        expect(controller, "GET", initial);
        expect(controller, "SET 0 180 0 180 0 181", "ERR JOINT_LIMIT 6");
        expect(controller, "SET 0 180 0 180 0 nan", "ERR BAD_NUMBER");
        expect(controller, "STEP 0", "ERR STEP_RANGE");
        expect(controller, "GET", initial);
        expect(controller, "STEP 250", moved); // Old targets survived both rejected SETs.
        expect(controller, "STOP", "ACK STOP");
        expect(controller, "STEP 1000", moved);
        expect(controller, "QUIT", "ACK QUIT");

        JointConfigs configs{};
        for (std::size_t i = 0; i < configs.size(); ++i) {
            configs[i] = {-90, 90, 0, 10.0 * static_cast<double>(i + 1)};
        }
        Controller custom(configs);
        expect(custom, "SET 90 -90 90 -90 90 -90", "ACK SET");
        expect(custom, "STEP 500",
               "STATE 5.000000 -10.000000 15.000000 -20.000000 25.000000 -30.000000");

        const std::string valid = "0 180 90 60\n";
        std::string rows;
        for (int i = 0; i < 6; ++i) {
            rows += valid;
        }
        std::istringstream input(rows);
        const auto loaded = load_configs(input);
        if (loaded[5].initial_deg != 90 || loaded[0].max_speed_deg_per_second != 60) {
            throw std::runtime_error("configuration values differ");
        }
        invalid_config("");
        invalid_config(rows + valid);
        invalid_config(rows + "\n");
        for (const auto *row : {"0 180 nan 60\n", "0 180 90 0\n", "180 0 90 60\n", "0 180 181 60\n",
                                "-361 180 90 60\n", "0 361 90 60\n", "0 180 90 361\n",
                                "0 180 90 60 extra\n", "0 180 90\n"}) {
            invalid_config(std::string(row) + rows.substr(valid.size()));
        }
        configs[0].max_speed_deg_per_second = -1;
        try {
            Controller invalid(configs);
            throw std::runtime_error("accepted invalid programmatic configuration");
        } catch (const std::invalid_argument &) {
        }
        std::cout
            << "PASS: six joints, atomic rejection, stop, custom speeds, configuration errors\n";
    } catch (const std::exception &error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}

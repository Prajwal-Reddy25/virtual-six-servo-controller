#include "virtual_servo/controller.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
enum class ReadStatus { Complete, TooLong, End };

// Retain at most 257 bytes: 256 payload bytes and a possible trailing CR.
// Discard excess through LF to restore framing. Never execute an EOF fragment.
ReadStatus read_line(std::istream &input, std::string &line) {
    line.clear();
    bool too_long = false;
    char byte = 0;
    while (input.get(byte)) {
        if (byte == '\n') {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            return too_long || line.size() > virtual_servo::kMaxLineBytes ? ReadStatus::TooLong
                                                                          : ReadStatus::Complete;
        }
        if (line.size() < virtual_servo::kMaxLineBytes + 1) {
            line.push_back(byte);
        } else {
            too_long = true;
        }
    }
    if (input.bad() || !input.eof()) {
        throw std::runtime_error("failed reading controller input");
    }
    return ReadStatus::End;
}
} // namespace

int main(int argc, char *argv[]) {
    try {
        virtual_servo::JointConfigs configs{};
        if (argc == 3 && std::string_view(argv[1]) == "--config") {
            std::ifstream file(argv[2]);
            if (!file) {
                throw std::runtime_error("cannot open configuration file");
            }
            configs = virtual_servo::load_configs(file);
        } else if (argc != 1) {
            throw std::invalid_argument("usage: servo_controller [--config PATH]");
        }
        virtual_servo::Controller controller(configs);
        std::string line;
        while (true) {
            const auto status = read_line(std::cin, line);
            if (status == ReadStatus::End) {
                return 0;
            }
            const auto reply = status == ReadStatus::TooLong
                                   ? virtual_servo::Reply{"ERR LINE_TOO_LONG"}
                                   : controller.handle_line(line);
            std::cout << reply.text << '\n' << std::flush;
            if (!std::cout) {
                throw std::runtime_error("failed writing controller response");
            }
            if (reply.quit) {
                return 0;
            }
        }
    } catch (const std::exception &error) {
        std::cerr << "controller: " << error.what() << '\n';
        return 1;
    }
}

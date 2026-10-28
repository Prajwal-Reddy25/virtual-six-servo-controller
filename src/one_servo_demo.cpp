#include "virtual_servo/joint.hpp"

#include <iomanip>
#include <iostream>
#include <locale>

int main() {
    // Fixed, valid inputs only. This executable does not accept wire commands.
    virtual_servo::Joint joint;
    std::cout.imbue(std::locale::classic());
    std::cout << std::fixed << std::setprecision(6);

    const auto show = [&joint](const char *action) {
        std::cout << action << ": position=" << joint.position_deg()
                  << " deg, target=" << joint.target_deg() << " deg\n";
    };

    show("Initial");
    joint.set_target(120.0);
    show("Target set to 120 deg");
    joint.step(250);
    show("Advanced 250 ms");
    joint.stop();
    joint.step(1000);
    show("Stopped, then advanced 1000 ms");
    joint.set_target(60.0);
    joint.step(1000);
    show("New target 60 deg, advanced 1000 ms");
}

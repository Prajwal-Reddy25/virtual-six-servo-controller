# Phase 2 verification

Executed by Codex on 2026-10-28 using CMake 3.28.3 and GNU C++ 13.3.0.
This verifies the one-servo motion milestone, not the final controller.

## Commands and results

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure --verbose
./build/one_servo_demo
c++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude -c src/parser.cpp -o build/parser.o
```

All commands exited successfully. Configuration selected C++17 with language
extensions disabled. The build completed without compiler warnings. CTest
reported one passing test entry containing seven passing motion cases:

```text
PASS: initial state
PASS: target and position separation
PASS: bounded bidirectional motion
PASS: no overshoot
PASS: inclusive boundaries
PASS: stop and resume
PASS: custom configuration and millisecond units
```

The standalone compilation of `src/parser.cpp` checks only that the provided
header/types compile. It does not link or test a parser: that function has no
definition yet.

## Captured demo output

```text
Initial: position=90.000000 deg, target=90.000000 deg
Target set to 120 deg: position=90.000000 deg, target=120.000000 deg
Advanced 250 ms: position=105.000000 deg, target=120.000000 deg
Stopped, then advanced 1000 ms: position=105.000000 deg, target=105.000000 deg
New target 60 deg, advanced 1000 ms: position=60.000000 deg, target=60.000000 deg
```

The observed 15-degree movement over 250 simulated milliseconds is consistent
with the configured 60 degrees/second. The stopped joint remained at 105 degrees
through a subsequent step and accepted a later target. These are virtual model
checks, not observations of a physical actuator.

No parser, command-validation, Python-host, or communication integration tests
have been run. No release, tag, push, or publication has been performed.

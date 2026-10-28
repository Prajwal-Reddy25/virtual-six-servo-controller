# Virtual Six-Servo Controller

A software-only actuator controller built with C++17 and a Python command-line
host. It demonstrates parsing, validation, execution, and testing before
connecting software to embedded hardware.

Six virtual joints have independent position limits and maximum speeds. The
host launches a local child process and exchanges ASCII lines over stdin/stdout.
Motion advances only through explicit `STEP` commands, making the simulation
deterministic without threads or timers.

**Status:** v1.0.0 release candidate implemented and verified; publication awaits
approval. The project author defined the scope, approved the protocol, and
directed development. Codex generated the implementation, tests, and documents.
Independent coding work is not claimed; see [contributions](docs/CONTRIBUTIONS.md).

## Architecture

```text
Python CLI / ControllerClient
          | ASCII requests / responses over subprocess pipes
C++ bounded line reader
          | complete line
parse_command -> Command or ParseError
          | validated command only
Controller -> std::array<Joint, 6> -> formatted response
```

The parser has no I/O or motion side effects. `SET` validates all six targets
before committing any. `GET` reports current simulated positions; setting
targets alone does not move a joint. `STOP` replaces each target with its current
position. Each step advances by at most `maximum_speed * milliseconds / 1000`,
without overshooting.

Validation rejects malformed numbers, missing/extra arguments, NaN, infinity,
conversion overflow/underflow, and limit violations. This prevents ambiguous
input or a partially accepted pose from reaching the motion model. The internal
`Joint` API assumes prevalidated inputs.

## Build and test

Requires CMake 3.16+, a C++17 compiler with floating-point `std::from_chars`,
and Python 3.10+. No third-party runtime or test dependencies are required.
The Python pipe-selector host and tests require POSIX (Linux/macOS). Verification
used Linux, GNU C++ 13.3.0, CMake 3.28.3, and Python 3.12.3; macOS is unverified.

From the repository root:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
ctest --test-dir build-release --output-on-failure

# Also run the Python tests directly:
PYTHONPATH=python SERVO_CONTROLLER=build-release/servo_controller \
  python3 -m unittest discover -s tests -v
```

Verified result: **4/4 CTest entries and 17/17 Python tests passed**. CTest runs
three C++ test executables and the Python suite. Coverage includes motion bounds,
strict parsing, precedence, configurations, all six joints, atomic rejection,
stopping, framing, EOF, malformed responses, timeouts, stderr drainage, and
child cleanup. See [testing evidence](docs/TESTING.md).

## Run

Run the complete Python/C++ demonstration:

```sh
PYTHONPATH=python python3 -m virtual_servo_host \
  --controller build-release/servo_controller --demo
```

For interactive commands, omit `--demo`. EOF also closes the child. For example:

```text
SET 120 60 150 30 180 0
GET
STEP 250
STOP
QUIT
```

After `STEP 250`, positions are `105 75 105 75 105 75` degrees. See the
[captured session](docs/evidence/demo.txt) and [examples](docs/EXAMPLES.md).
The host sends malformed ASCII commands so controller rejection can be observed.
Protocol `ERR` replies are displayed normally. Transport failures produce a
nonzero host exit and stderr diagnostics; failed connections are not reused.
`--timeout SECONDS` sets the response/shutdown wait (default 2).

Use the controller directly with `./build-release/servo_controller`. Each
complete line receives one flushed response; diagnostics use stderr. `QUIT` or
EOF exits, and an unterminated final line is discarded. The original
`./build-release/one_servo_demo` remains a teaching example; its descriptive
output is not wire protocol.

## Joint configuration

Defaults: initial position 90 degrees, limits 0–180 degrees, maximum speed 60
degrees per simulated second. An optional file has exactly six rows in joint
order: `lower upper initial speed`. No comments or blank rows; a final newline
is optional. Angles use degrees and speeds degrees/s.

[config/example.conf](config/example.conf) starts at zero with limits -90–90
and speeds 10, 20, 30, 40, 50, and 60 degrees/s:

```sh
PYTHONPATH=python python3 -m virtual_servo_host \
  --controller build-release/servo_controller --config config/example.conf
```

Here, `SET 90 -90 90 -90 90 -90` then `STEP 500` yields `5 -10 15 -20 25 -30`.
These illustrative bounds are not hardware calibrations.

## Limitations and hardware direction

This is a software-only actuator simulation, not a physically validated robotic
arm. It demonstrates no physical accuracy, torque capability, safe movement,
collision avoidance, coordinated trajectory quality, or real-time guarantees.
It models independent speed-limited positions without acceleration or dynamics.
`STOP` is software cancellation, not an emergency stop. There is no networking,
ROS, inverse kinematics, or graphics.

An ESP32 needs serial framing/buffering, calibrated actuator mappings,
device-side timing, link-loss behavior, and electrical/mechanical protections.
See [hardware integration](docs/HARDWARE.md).

## Documents

- [Protocol](docs/PROTOCOL.md) and [tests/evidence](docs/TESTING.md)
- [Contributions](docs/CONTRIBUTIONS.md) and [development strategy](docs/PLAN.md)
- [Learning guide and exercises](docs/LEARNING.md)
- [CV wording and interview questions](docs/PORTFOLIO.md)
- [Publication steps](docs/PUBLICATION.md) and [release draft](docs/RELEASE_v1.0.0.md)
- [Historical Phase 2 verification](docs/PHASE2_VERIFICATION.md)

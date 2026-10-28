# Portfolio wording and interview preparation

Project title: **Virtual Six-Servo Controller — Python/C++ Actuator Command Testbed**

Technologies: C++17, Python standard library, CMake/CTest, subprocess pipes,
ASCII protocols, deterministic simulation, unit/integration testing, Git.

CV bullets based on verified behavior:

- Developed an AI-assisted Python/C++17 application that exchanges ASCII commands
  over subprocess pipes and simulates six configurable servo joints.
- Implemented and verified strict validation, atomic target updates, speed-limited
  motion, and stop behavior through C++ unit tests and Python communication/fault
  tests, using Codex-generated implementation.

These describe the delivered AI-assisted project. They do not claim independent
parser authorship, physical testing, or independently demonstrated programming
fluency. Revise attribution only when later contributions establish it. Be ready
to explain all code before interviewing.

GitHub description:

> Software-only six-servo controller in C++17 with a Python CLI, ASCII subprocess
> protocol, strict validation, configurable joint limits, deterministic motion,
> and unit/integration tests. AI-assisted implementation; no physical arm claims.

Topic tags: `cpp17`, `python`, `cmake`, `ctest`, `mechatronics`, `robotics`,
`servo-control`, `simulation`, `command-line`, `integration-testing`.

Five technical interview questions:

1. How are `90oops`, NaN, and numeric overflow rejected, and why must every SET
   number be parsed before checking joint limits?
2. How do pipes and line framing connect Python to C++, and how does the host
   handle a response that arrives in several reads?
3. How do joint limits and speed bounds apply, and how do targets differ from
   simulated current positions?
4. What happens after invalid commands, oversized lines, EOF, timeouts, and child
   crashes? Which faults allow the connection to continue?
5. What changes for an ESP32 over serial, and which physical/timing/safety
   properties cannot be inferred from this simulation?

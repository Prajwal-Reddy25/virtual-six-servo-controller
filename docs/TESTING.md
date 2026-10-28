# Testing and actual execution evidence

Verified on Linux with GNU C++ 13.3.0, CMake 3.28.3, and Python 3.12.3.
Tests exercise software behavior, not physical accuracy, torque, safe movement,
collision handling, or real-time performance.

## Reproduce acceptance

From the repository root:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
ctest --test-dir build-release --output-on-failure
PYTHONPATH=python SERVO_CONTROLLER=build-release/servo_controller \
  python3 -m unittest discover -s tests -v
PYTHONPATH=python python3 -m virtual_servo_host \
  --controller build-release/servo_controller --demo
```

## Coverage

| Suite | Checks |
| --- | --- |
| `joint_motion` | Seven original motion cases: initialization, target/position separation, bidirectional speed bounds, no overshoot, inclusive boundaries, stop/resume, custom speed and millisecond conversion |
| `command_parser` | All commands, decimal grammar, signed/exponent values, malformed/non-finite/overflow/underflow numbers, full conversion, arity, step bounds and integer overflow, byte/line checks, precedence, custom limits |
| `six_joint_controller` | Six-joint movement, SET changes targets only, rejected commands preserve old targets, STOP, independent speeds, valid/invalid startup configurations |
| `python_host_and_integration` | 17 Python methods: response parsing, faulty processes, timeout/forced cleanup, stderr flooding, nonzero exits, local framing, real C++ communication, atomic rejection, step partition, custom configuration, CRLF/oversized-line recovery, EOF, CLI and piped input |

The C++ suites contain grouped/table-driven checks, not inflated counts of
separate tests. CTest has four entries. The Python suite has 17 methods,
including eight real-controller integration methods. C++ checks remain active
in Release builds and do not depend on `assert()`.

The framing test sends a 100,000-byte line followed by valid commands and checks
recovery. Source inspection confirms the reader retains at most 257 bytes before
discarding (including space for a possible CR). This is not a memory measurement.

## Results and logs

Acceptance succeeded: **4/4 CTest entries passed**; the directly invoked Python
suite reported **17 tests, OK**. The end-to-end demo ended with `ACK QUIT` and
zero exit status.

- [Fresh Release configure/build](evidence/build.txt)
- [CTest](evidence/ctest.txt)
- [Direct Python tests](evidence/python-tests.txt)
- [Actual demonstration](evidence/demo.txt)

Logs come from actual execution. Absolute repository paths are replaced with
`<repo>` to avoid publishing local filesystem details, and trailing whitespace
is removed; results are otherwise retained. `PHASE2_VERIFICATION.md` is historical and does not imply final
integration was complete during that milestone.

## Issues found and fixed

The initial six-joint build warned that list-initializing the joint array used
an explicit constructor with a default argument. A separate delegating default
constructor now expresses default initialization cleanly. The next build had
no warnings.

The initial lint pass found import order and formatting/style issues. Imports,
response checks, and explicit subprocess `check=False` calls were cleaned up;
formatting and lint were rerun. No functional test failure was observed during
implementation. Fault-injection tests intentionally trigger errors and pass only
when the host handles them. These debugging steps were performed by Codex.

## Formatting

C++ uses `.clang-format`; Python uses Ruff formatting and correctness/import
checks. Development-only tools were clang-format 23.1.3 and Ruff 0.17.3, installed
in a temporary directory, not as runtime dependencies. With tools available:

```sh
clang-format --dry-run --Werror include/virtual_servo/*.hpp src/*.cpp src/*.hpp tests/*.cpp
ruff format --isolated --no-cache --check python tests
ruff check --isolated --no-cache --select E4,E7,E9,F,I python tests
```

Windows host support, other compiler/library combinations, serial hardware,
and device timing have not been verified.

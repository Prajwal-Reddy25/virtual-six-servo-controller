# Independent parser assignment

**Historical assignment, superseded by the revised delivery strategy.** The
author later authorized Codex to implement the parser. The final parser is
Codex-generated; the original assignment below remains study material, not
an authorship claim or a pending release requirement.

The author implements `src/parser.cpp`. Codex supplied only the types and
function declaration in `include/virtual_servo/parser.hpp`, not parsing or
command-validation logic. The motion model and demo are Codex-generated.

## Your task and boundary

Implement this C++17 function:

```cpp
ParseResult parse_command(std::string_view line, const JointConfigs& configs);
```

The input is one complete ASCII command line, with LF or CRLF already removed.
`configs` contains six already-valid configurations, in joint order. The
function returns either a `Command` or a `ParseError`. It must not change any
joint, read stdin, write stdout/stderr, terminate the process, or store state
between calls. Ordinary malformed input must return an error, not an uncaught
conversion exception.

Although the Phase 2 demo models only one servo, implement the approved
six-angle `SET` format from the outset. Extending the motion model to six joints
will then use the same parser interface.

Implement all five commands in `PROTOCOL.md`. Populate `targets_deg` only for
`Set`, `step_ms` only for `Step`; other fields can keep their default values.
Use the corresponding `ErrorCode` for failures. `joint_number` is 1-based for
`JointLimit` and zero for all other errors.

Check line length and ASCII bytes on the provided complete line, then follow
the error precedence in the protocol. For an oversized stream line, the later
transport reader must itself bound buffering, discard through the newline,
and produce one error; the parser's length check alone does not provide that
stream-level behavior. EOF and line-ending removal also belong to that later
reader, not to your parser.

## Required behavior

Using default configurations:

| Input line | Expected result |
| --- | --- |
| `SET 120 60 90 90 90 90` | `CommandType::Set`, with all six target values |
| `GET` | `CommandType::Get` |
| `STEP 250` | `CommandType::Step`, `step_ms == 250` |
| `STOP` / `QUIT` | Corresponding command type |
| `  GET\t` (where `\t` is an actual tab) | `CommandType::Get` |
| Empty string or only spaces/tabs | `ErrorCode::Empty` |
| `get` or `HELLO` | `ErrorCode::UnknownCommand` |
| `SET 90 90` or seven angle arguments | `ErrorCode::Arity` |
| `GET extra` or `STEP` | `ErrorCode::Arity` |
| `SET 90 90oops 90 90 90 90` | `ErrorCode::BadNumber` |
| A `SET` angle of `nan`, `inf`, `-inf`, or `1e9999` | `ErrorCode::BadNumber` |
| `SET 90 90 90 90 90 181` | `ErrorCode::JointLimit`, `joint_number == 6` |
| `SET -1 90oops 90 90 90 90` | `ErrorCode::BadNumber` (parse all values before checking limits) |
| `STEP 0` or `STEP 1001` | `ErrorCode::StepRange` |
| `STEP -1`, `STEP 1.5`, `STEP +10`, or `STEP 1e2` | `ErrorCode::BadNumber` |
| More than 256 bytes | `ErrorCode::LineTooLong` |
| A non-ASCII byte in an otherwise short line | `ErrorCode::BadEncoding` |

Also cover inclusive angle limits, decimal/exponent syntax, leading zeros in
`STEP`, integer overflow, custom joint limits, missing/extra arguments for
every command, and multiple simultaneous errors. ASCII control characters
other than token-separating spaces/tabs are not whitespace separators under
this protocol; do not silently accept them as such. Embedded NUL must not
truncate a token or line into a successful command.

## C++ concepts to understand

- **`std::string_view`:** a borrowed, read-only view. Do not retain it after the
  call or assume its bytes end with a NUL character.
- **Tokenization and full consumption:** recognizing a numeric prefix is not
  enough; `90oops` must fail. Only spaces and tabs separate tokens here.
- **Numeric conversions:** distinguish conversion failure, overflow, and a
  successfully converted value outside its allowed range. Standard-library
  conversion APIs differ in locale handling and accepted syntax; choose one
  whose behavior you can explain. Handle NaN and infinity explicitly.
- **`std::array` and indexing:** six fixed targets/configurations, with zero-based
  storage indices translated to 1-based joint numbers in errors.
- **`enum class` and `std::variant`:** named command/error categories and exactly
  one result alternative. A caller can use `std::get_if` to inspect the result.
- **`const` references and local state:** configurations are inputs; build the
  command locally and return it only when all checks succeed.

Atomic execution will be the later controller's responsibility: it must apply
all returned targets only after parsing succeeds. Your side-effect-free parser
is the first part of that design; parser success alone does not prove the
controller commits targets atomically.

## How to submit your work

Edit `src/parser.cpp`; add small private helper functions there if useful.
Keep it standard-library-only. Add `tests/test_parser.cpp` with cases you wrote,
and briefly record your approach and any bugs you found. Do not copy the
motion test harness unless it helps you; no external test framework is needed.

You can compile your parser translation unit before linking it to a caller:

```sh
c++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude -c src/parser.cpp -o build/parser.o
```

This checks compilation only. We will add a CMake parser/test target and review
and run your tests together when you return. A compiling source file with no
function definition does not complete the assignment.

Bring back the implementation and explain how you reject trailing garbage,
how you distinguish `BadNumber` from range errors, and why you do not modify
motion state while parsing. Codex will review before offering fixes so the
record can accurately identify your independent work.

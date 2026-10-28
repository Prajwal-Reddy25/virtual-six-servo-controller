# ASCII line protocol v1

Status: **approved by the project author on 2026-10-28**, including explicit
`STEP` commands. The initial proposal was drafted by Codex. The author approved
it and requested explicit clarification of units, validation, atomic updates,
position/target semantics, stopping, responses, step durations, and EOF.

## Transport and framing

- The Python host launches the controller as a child process, writes commands
  to its stdin, and reads responses from its stdout.
- One command per line, one response per command, in arrival order. The host
  waits for a response before sending another command. No request IDs are needed.
- Use ASCII text. Send LF line endings; accept LF and CRLF. Flush each response.
- Commands are case-sensitive. Spaces and tabs separate tokens; leading and
  trailing spaces and tabs are allowed. No comments or extra tokens.
- A line may contain at most 256 bytes, excluding LF or CRLF. Discard an
  oversized line through its newline and return one `ERR LINE_TOO_LONG`.
- A blank line returns `ERR EMPTY`. Non-ASCII input returns `ERR BAD_ENCODING`.
- stdout contains protocol responses only. Human diagnostics go to stderr.
- EOF terminates the controller normally and discards any unterminated final
  line without executing it or emitting a response for it. Every complete
  command line, including an invalid or empty one, receives exactly one
  response. EOF itself receives no response. A read failure other than normal
  EOF produces a stderr diagnostic and nonzero exit status. The host treats
  unexpected controller exit or EOF as a communication
  failure, rather than a successful command acknowledgment.

## State and configuration

The final controller has six joints, numbered 1 through 6 in message order.
Each joint has a current position, target position, lower limit, upper limit,
and positive maximum speed. Angles and positions use degrees; maximum speed
uses degrees per simulated second (`degrees/second`); `STEP` durations use
integer milliseconds (`ms`, with `1000 ms = 1 simulated second`).

Defaults for every joint are limits `[0, 180]`, initial position `90`, and maximum
speed `60`. These are illustrative simulator settings, not hardware limits.
Initial targets equal initial positions.

Joint configuration is loaded at startup, before accepting commands. The
controller accepts an optional `--config PATH` file with exactly six rows,
each containing `lower upper initial speed`. No comments or blank rows are
allowed; LF/CRLF and an optional final newline are accepted. Rows have at most
256 bytes excluding their line ending. All values must be
finite, `lower < upper`, `lower <= initial <= upper`, and `speed > 0`.
Restrict configured position limits to `[-360, 360]` and speed to at most
`360` degrees per simulated second to keep this educational model bounded.
Invalid configuration produces a stderr diagnostic and nonzero exit status.
These configuration bounds are modeling choices, not physical safety criteria.

## Commands and responses

| Command | Meaning | Successful response |
| --- | --- | --- |
| `SET a1 a2 a3 a4 a5 a6` | Validate all targets, then replace them together. Current positions do not change. | `ACK SET` |
| `GET` | Read current positions. | `STATE p1 p2 p3 p4 p5 p6` |
| `STEP ms` | Advance simulated motion by an integer 1–1000 milliseconds. | `STATE p1 p2 p3 p4 p5 p6` |
| `STOP` | Set all targets to current positions, cancelling pending motion. | `ACK STOP` |
| `QUIT` | Acknowledge, flush, and exit successfully. | `ACK QUIT` |

There is no autonomous movement: only `STEP` changes current positions. This
avoids threads, scheduling, and dependence on host execution speed. `STOP`
freezes each joint at its current simulated position by replacing its target
with that position. Further steps leave it there until a subsequent valid
`SET`. `STOP` does not latch a fault.
It is not an emergency-stop mechanism for hardware.

`GET` reports actual simulated positions, not targets. A successful `SET`
changes only targets; it never directly changes positions.

For each joint and each step:

```text
maximum_change = maximum_speed * ms / 1000
change = clamp(target - current, -maximum_change, maximum_change)
current = clamp(current + change, lower_limit, upper_limit)
```

This is an independent, constant-speed position update, with no acceleration,
load, torque, dynamics, collision model, or coordinated trajectory guarantee.

Angles accept decimal numbers with an optional sign and exponent, for example
`90`, `-10.5`, `+2.0`, `.5`, and `9e1`. Require full-token parsing and a finite
representable value. Reject `nan`, `inf`, overflow, and suffixes such as `90deg`.
Use a period as the decimal separator, independently of the system locale.
Conversion underflow reported by the C++ conversion routine also produces
`ERR BAD_NUMBER`, rather than silently accepting a nonzero value as zero.
The accepted angle syntax is defined by the following grammar (`digits` means
one or more ASCII digits; `?` means optional):

```text
sign     = "+" | "-"
mantissa = digits ("." digits?)? | "." digits
exponent = ("e" | "E") sign? digits
angle    = sign? mantissa exponent?
```

Thus `1.` is accepted; hexadecimal literals, underscores, commas, and lone
signs are rejected. The grammar does not override finite-value or range checks.
`STEP` accepts only ASCII digits, without a sign, exponent, or decimal point.
Leading zeros are allowed. For example, `STEP 0` and `STEP 1001` produce
`ERR STEP_RANGE`; `STEP -1`, `STEP 1.5`, and `STEP nan` produce `ERR BAD_NUMBER`;
`STEP` and `STEP 10 20` produce `ERR ARITY`. All leave state unchanged.

`STATE` prints positions in fixed decimal notation with six fractional digits.
Keep internal values as doubles; round only for output. Tests compare numeric
positions with a tolerance of `0.000001` degrees, not raw response strings.

## Validation and errors

Every invalid command leaves current positions and targets unchanged. In
particular, never apply the first few angles of a rejected `SET` command.
Validate in this order so tests and diagnostics are predictable:

1. Framing and encoding: `ERR LINE_TOO_LONG`, then `ERR BAD_ENCODING`.
2. Empty input: `ERR EMPTY`.
3. Unknown command: `ERR UNKNOWN_COMMAND`.
4. Wrong token count for a known command: `ERR ARITY`.
5. Numeric syntax, overflow, or non-finite value: `ERR BAD_NUMBER`.
6. Angle outside its inclusive configured limits: `ERR JOINT_LIMIT j`.
7. Numeric `STEP` value outside 1–1000: `ERR STEP_RANGE`.

For `SET`, parse all six numbers before checking joint limits; report the first
out-of-limit joint in ascending order. Extremely large all-digit `STEP` values
that cannot be represented by the parser's `int` produce `ERR BAD_NUMBER`
(the verified platform uses a 32-bit signed `int`).

Errors do not terminate the controller. The next correctly framed command can
succeed. No free-form prose is included in protocol error responses.

## Example session with default configuration

`>` indicates host input and `<` controller output; these prefixes are not sent.

```text
> GET
< STATE 90.000000 90.000000 90.000000 90.000000 90.000000 90.000000
> SET 120 60 90 90 90 90
< ACK SET
> STEP 250
< STATE 105.000000 75.000000 90.000000 90.000000 90.000000 90.000000
> SET 120 60 90 90 90 181
< ERR JOINT_LIMIT 6
> SET 120 60oops 90 90 90 90
< ERR BAD_NUMBER
> STOP
< ACK STOP
> STEP 250
< STATE 105.000000 75.000000 90.000000 90.000000 90.000000 90.000000
> SET 90 90
< ERR ARITY
> STEP 0
< ERR STEP_RANGE
> QUIT
< ACK QUIT
```

These are specified expectations, not captured execution results.
See `EXAMPLES.md` and `evidence/demo.txt` for an actual host/controller session.

## Implementation clarifications

The approved commands, defaults, responses, and precedence are unchanged.
The implementation makes configuration syntax and conversion underflow explicit.
Only ASCII space and tab separate tokens; other ASCII control bytes are ordinary
token bytes and cannot be silently skipped.

## Why validation precedes motion

Malformed messages must not become actuator commands. Full-token parsing
detects corrupted or ambiguous values; finite-value and limit checks ensure
the motion calculation receives bounded inputs. Validating the entire command
before committing targets prevents a rejected message from partially changing
the arm's requested pose. Hardware will additionally require independent
electrical and mechanical protections.

# Learning guide

Study in this order. The implementation is Codex-generated; explaining or
independently extending it is a separate learning outcome.

1. **Parser/data structures:** read `parser.hpp`, `text.hpp`, and `parser.cpp`.
   Trace borrowed string views, decimal grammar, `from_chars`, arrays, and
   command/error variants. Explain why conversions precede limit checks.
2. **Six-servo state:** read `joint.cpp` and `controller.cpp`. Distinguish targets
   from positions, calculate a 250 ms step at 60 degrees/s, and trace atomic SET
   rejection and STOP. STOP cancels a target without latching a fault.
3. **Python communication:** follow a request in `client.py`, then the CLI.
   Understand nonblocking writes, partial reads, selectors, monotonic deadlines,
   response validation, and process cleanup. Timeout bounds waiting, not motion.
4. **Error handling:** compare a normal ERR reply with a broken pipe or malformed
   response. Protocol rejection leaves a usable connection; transport faults
   invalidate it. Trace QUIT, terminate, and kill cleanup paths.
5. **Testing/debugging:** run the suites and follow the atomic-rejection test.
   Faulty subprocess fixtures exercise communication errors without changing
   the production controller.

Five practical independent modifications after release:

1. Add a TARGETS query, documentation, and tests distinguishing targets/positions.
2. Introduce a trailing-garbage parser bug on a local branch, add a failing
   regression test, fix the bug, and explain the failure.
3. Add a per-joint maximum target-change rule. Specify whether it compares with
   old targets or current positions, preserving atomic SET behavior.
4. Add request IDs across command format, responses, host, and tests; document
   the protocol version change.
5. Add a standard-library Python command-file option that stops on the first
   controller error and still closes the child cleanly.

Record an authored diff/commit, rationale, test output, and reviewed explanation
for each exercise in `CONTRIBUTIONS.md`. Do not retrospectively attribute
generated code to yourself.

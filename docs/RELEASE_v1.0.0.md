# Virtual Six-Servo Controller v1.0.0

Local release draft; external publication requires approval.

A software-only Python/C++17 application validates ASCII commands and simulates
six independently configurable actuator positions over a deterministic
subprocess protocol.

- SET, GET, STEP, STOP, QUIT with strict parsing and explicit errors.
- Atomic validation, per-joint limits/speeds, bounded motion, and stop/resume.
- Python CLI with response checks, bounded waits, stderr handling, and child
  cleanup; no threads or networking.
- Four passing CTest entries, including 17 Python tests, plus a verified demo.
- Protocol, actual evidence, examples, ESP32 notes, learning exercises, and
  explicit AI-assistance attribution.

The author defined scope, approved the protocol, and authorized delivery; Codex
generated implementation and tests. Independent parser authorship and physical
servo performance are not established. Linux was tested; the host requires POSIX
and Python 3.10+. No physical testing or real-time guarantees are claimed.

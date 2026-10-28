# Development strategy

The original plan reserved parsing and validation for the author. The author
later authorized Codex to finish implementation and continue learning in
parallel. This supersedes the original parser restriction; attribution remains
explicit in `CONTRIBUTIONS.md`.

Delivery milestones:

1. Approved ASCII protocol with deterministic STEP and explicit errors.
2. C++17/CMake scaffolding, one-joint model, and motion tests.
3. Side-effect-free parser and strict numeric/limit validation.
4. Six-joint controller, startup configuration, and bounded line reader.
5. Python CLI with response checks, bounded waits, and process cleanup.
6. C++ tests, Python fault tests, real process integration, and demonstration.
7. Documentation, formatting, diff inspection, local commit, v1.0.0 preparation.
8. External publication only after explicit author approval.

Milestones 1–6 are implemented and verified; see `TESTING.md`. Publication and
independently assessed learning remain separate from software acceptance.
No threads, ROS, networking, dynamics, graphics, or third-party runtime libraries
were introduced.

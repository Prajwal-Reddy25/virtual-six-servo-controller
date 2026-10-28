# Engineering contribution record

Authorship, decisions, execution, and independent programming evidence are
recorded separately. Generated code is not evidence of independent coding.

| Work | Contributor | Evidence / status |
| --- | --- | --- |
| Purpose, scope, constraints, acceptance criteria | Project author | Initial brief and revised delivery instructions |
| Initial ASCII protocol proposal | Codex | `docs/PROTOCOL.md` |
| Protocol approval and seven clarifications | Project author | Approval on 2026-10-28, including explicit `STEP` |
| One-servo model, scaffolding, motion tests | Codex | Source, CMake, `test_joint.cpp`, historical Phase 2 evidence |
| Parser interface and initial assignment | Codex | `parser.hpp`, historical `PARSER_ASSIGNMENT.md` |
| Authorization to complete remaining components | Project author | Revised strategy superseding the parser restriction |
| Parser, numeric conversion, command validation | Codex | `parser.cpp`, `text.hpp`, parser tests |
| Six-joint controller, bounded framing, configuration | Codex | `controller.cpp`, `main.cpp`, `config.cpp`, controller tests |
| Python subprocess host and CLI | Codex | `python/virtual_servo_host/` |
| Unit/integration/fault tests, debugging, formatting | Codex | `tests/`, `TESTING.md`, acceptance logs |
| Documentation, examples, learning guide, release preparation | Codex | Repository documents |
| Independent parser implementation, link diagnosis, or extension | Not established | No authored attempt was present when the strategy changed; optional future learning work |
| Independent explanation of the implementation | Not assessed | Questions and exercises supplied; answers not reviewed |
| GitHub publication approval | Project author | Pending; no external publication authorized |

## Development history

Phase 2 ended with a tested one-servo model and an empty parser implementation
file. The author initially reserved parsing/validation for independent work;
Codex supplied concepts and an assignment without implementing that function.

Before any independent parser attempt was available in the repository, the
author changed strategy and authorized Codex to finish the remaining components.
The final implementation is AI-assisted project work with Codex-generated code.
Earlier assignment documents are historical learning material, not final code
ownership claims.

Codex corrected an explicit-default-constructor initialization warning and
formatting/lint findings. Deliberately faulty subprocess tests were written and
diagnosed by Codex. These are not independent author debugging evidence.

Future independent work should include an authored diff/commit, rationale,
test results, and reviewed explanation. No physical servo testing, real-time
behavior, or independent programming fluency is established by this release.

# quantum_rng_audit — Prestige Prompt (Jul 11 2026 v1)

## Purpose
Fork of `tsotchke/quantum_rng` for devil's-advocate auditing. Inviting, rigorous.

## One-liner Mission
Prove or refute "quantum-inspired / non-deterministic / 63.999872 bits/sample"
claims with source reading + empirical compile/run. Show how a Lean+C contract
pipeline makes entropy claims falsifiable.

## Stack
- Upstream: C11 PRNG (`src/quantum_rng/quantum_rng.c`, ~560 lines) + examples + tests.
- Audit tooling: `audit/determinism_test.c` (patches the broken build, runs the gen).
- Reference truth: NIST SP 800-90B / Dieharder methodology (not yet in repo).

## Priority Queue
- P0 — upstream PR: fix build (`<sys/types.h>`, `M_PI`/`M_E`); make seed reproducible.
- P0 — replace hardcoded "63.999872 bits/sample" with a real statistic or remove it.
- P1 — rename functions to reflect classical mixes (or doc that physics = analogy).
- P1 — add real entropy suite (NIST/Dieharder) instead of chi-square uniformity only.
- P2 — document: NOT for crypto/security (entropy = time + PID, predictable).

## Key Math (verified)
- Core = `splitmix64` + xorshift-style multiplies, seeded by
  `gettimeofday`+`getpid`+`clock`+`rdtsc`. No quantum mechanics.
- "Qubits" = array indices; "Hadamard/Pauli gates" = bit-mixing; magic constants
  named after physical quantities are arbitrary 64-bit ints.

## Verified (TRUST: HIGH)
- `qrng_init` ignores the supplied seed (mixes wall-clock time regardless).
- Same-seed runs differ → not a reproducible PRNG; stream keyed by init timestamp.
- Build broken (missing includes) — `audit/determinism_test.c` reproduces this.
- "63.999872 bits/sample" is hardcoded text; no code computes it.

## Debatable
- Whether marketing "quantum" is deliberate deception or enthusiastic analogy.
  We assume good-faith analogy; the fix is honest labeling, not accusation.

## Data Not To Re-derive
- Upstream `make all` fails; audit test patches via wrapper (see AGENTS.md).
- Only statistical test present is chi-square *uniformity* (validates PRNG is
  uniform, says nothing about entropy/quantum).

## Full Context
Read goal-mantra.md; plans/devils_advocate_v1.md for audit; AUDIT.md for findings.

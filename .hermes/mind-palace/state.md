# quantum_rng_audit — State (Jul 11 2026 v1)

## Binary / Artifact Truth Table
| Artifact | Claimed | Actual | Evidence |
|---|---|---|---|
| quantum_rng lib | builds | ❌ broken (missing includes) | gcc: pid_t/M_PI undeclared |
| determinism_test | runs | ✅ builds+passes | gcc; ./determinism_test |
| "non-deterministic output" | quantum | ❌ time-seeded classical PRNG | determinism_test |
| "63.999872 bits/sample" | proven entropy | ❌ hardcoded text | grep README/docs |
| chi-square test | validates RNG | ✅ (uniformity only) | tests/quantum_stats.c |

## Fixed / Found
- **BUG:** `qrng_init` folds in gettimeofday even with a seed → seed decorative.
- **BUG:** missing `#include <sys/types.h>`, `M_PI`/`M_E` → build fails.
- **CLAIM:** "quantum-inspired entropy" unsubstantiated (no quantum; no NIST suite).

## Hidden State
- Magic constants named QRNG_FINE_STRUCTURE etc. are arbitrary ints (no physics).
- "key exchange" examples use predictable entropy → NOT cryptographically safe.

## Verified Components
- splitmix64/hadamard_mix are competent PRNG mixes (avalanche well).
- chi-square uniformity test is a real (if limited) validation.

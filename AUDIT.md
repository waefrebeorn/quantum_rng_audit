# Quantum RNG Audit — Devil's-Advocate Findings

**Audit date:** 2026-07-11
**Subject:** `tsotchke/quantum_rng` (forked to `waefrebeorn/quantum_rng_audit`)
**Method:** source read of `src/quantum_rng/quantum_rng.c` + empirical compile/run
of the core generator. No theorem without a check.

## 1. Headline claim vs reality

| README claim | Reality (from code) |
|---|---|
| "Leverages quantum mechanical principles" | Pure classical PRNG. No quantum mechanics anywhere. "Qubits" are array indices (`QRNG_NUM_QUBITS=8`). |
| "Quantum superposition / entanglement / decoherence" | Function names only. `quantum_noise` = `sin/cos/sqrt` of a double; `hadamard_gate`/`phase_gate` = `splitmix64`/`xorshift` bit-mixing. |
| "High entropy output (63.999872 bits/sample)" | **Hardcoded string** in README/docs. No code computes it. No Dieharder/NIST suite present. |
| "Verified non-deterministic output" | **False by construction.** Output = deterministic function of `(counter, runtime_entropy)`, where `runtime_entropy` = `gettimeofday + getpid + clock + rdtsc`. A true CSPRNG reproduces a stream from its seed; this one **ignores the seed** at init (mixes in wall-clock time even when a seed is supplied). |
| "Proven entropy characteristics" | Only a chi-square *uniformity* test exists (`tests/`). That validates a PRNG is uniform — it says nothing about entropy/quantum. |

## 2. What the code actually is

A time-seeded, classical bit-mixer:
- `get_system_entropy()` → `gettimeofday` ⊕ `getpid` ⊕ `clock` ⊕ `rdtsc`.
- `quantum_step()` → loops `splitmix64` + `hadamard_mix` (xorshift-style
  multiplies with magic constants `QRNG_PAULI_X/Y/Z`, `QRNG_HEISENBERG`, …)
  over a counter + the time-derived entropy pool.
- Magic constants named after physical quantities (`QRNG_FINE_STRUCTURE`,
  `QRNG_PLANCK`, …) are just arbitrary 64-bit integers — they confer no
  physical property.

The mixing is *competent PRNG design* (splitmix64 avalanches well). The problem
is **honesty**: it is marketed as quantum/non-deterministic when it is a
classical, time-seeded PRNG.

## 3. Empirical check (this fork)

`audit/determinism_test.c` compiles the core (with the missing `<sys/types.h>`
and `M_PI`/`M_E` includes that the upstream build omits — see §4) and runs the
generator twice with the **same explicit seed**. Result:

```
seed-identical runs produce IDENTICAL output: NO
```

Because `qrng_init` folds in `gettimeofday` even with a seed, the seed is
decorative; the stream is keyed by wall-clock time + PID. That is the opposite
of "verified non-deterministic" — it is *fully deterministic given the init
timestamp*, and the seed is ignored. A correct seeded generator would reproduce
the stream. This is a concrete, reproducible defect in the claims.

## 4. Build defects (real, blocking)

- `quantum_rng.h` uses `pid_t` without `#include <sys/types.h>` → compile error.
- `quantum_rng.c` uses `M_PI`/`M_E` without `#define _USE_MATH_DEFINES` or
  inclusion ordering → `M_PI undeclared`.
- `make all` is broken for the same reasons (earlier audit noted this).

Fix (wrapper used by the audit test): add `#include <sys/types.h>` and define
`M_PI`/`M_E` before including the source. Upstream should add these to the
headers.

## 5. Verdict

A **well-mixed classical PRNG with quantum-themed naming and unsubstantiated
entropy/quantum claims.** Suitable as a fast non-cryptographic RNG (after fixing
the build). **Not** quantum, **not** non-deterministic, **not** proven to
63.999872 bits/sample. Do not use for security/cryptography. The naming is
marketing, not mechanism.

## 6. Recommendation for upstream PR

1. Add `#include <sys/types.h>` to `quantum_rng.h`; ensure `M_PI`/`M_E` defined.
2. Either (a) make the seed actually drive the stream (deterministic given seed,
   for reproducibility/testability), or (b) drop the "non-deterministic/quantum"
   claims and call it what it is: a time-seeded chaotic PRNG.
3. Replace the hardcoded "63.999872 bits/sample" with a real statistic computed
   by an actual entropy test (NIST SP 800-90B / Dieharder), or remove it.
4. Rename functions to reflect they are classical mixes, or add a doc note that
   the physics terms are analogy only.

This fork keeps the upstream code verbatim and adds `audit/` with the
reproducible evidence.

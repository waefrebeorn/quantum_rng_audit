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

## 4b. Measured output statistics (3×DA re-check, 2026-07-11)

Built the core lib (adding the missing includes) and generated 1,048,576 bytes:

| Quantity | Measured | Note |
|----------|----------|------|
| Byte Shannon entropy | **7.999847 bits/byte** (max 8.0) | output is essentially uniform |
| Chi-square uniformity | **222.6** (df=255, crit ≈ 293 @ p=0.05) | passes uniformity |
| Library's own `qrng_get_entropy_estimate()` | **1.26 bits/byte** | internally inconsistent — its math sums `-log2(raw_pool_byte+1e-10)` over 16 bytes + a bogus runtime term, divided by 17; not a real entropy estimate |
| README claim "63.999872 bits/sample" | **no code computes it** | hardcoded string; no NIST/Dieharder anywhere |

**Corrected framing (do NOT repeat the old "fabricated low entropy" line):**
the stream is a *good uniform* PRNG byte-wise (~8 bits/byte), so the real
defect is **NOT** that it has low entropy — it is that (a) the advertised
"63.999872 bits/sample" is a number with *no computation behind it*, and (b)
the library's *own* entropy estimator returns a meaningless 1.26. The honest
criticism is **unsubstantiated / misrepresented entropy**, not "low entropy."
Build+measure recipe: `gcc -D_DEFAULT_SOURCE -D_USE_MATH_DEFINES -Isrc
/tmp/qrng_measure.c src/quantum_rng/quantum_rng.c -o /tmp/qrng_measure -lm`.

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

---

## 7. Amendment (2026-07-12) — new `secure_rng` subsystem added upstream

> On 2026-07-11 upstream pushed 9 commits (now in `origin/master`, merged into
> this fork at `23154db`). The original audit (§1–§6, 2026-07-11) targeted the
> **core** `src/quantum_rng/quantum_rng.c`. Upstream has since added a separate
> `src/secure_rng/` + `src/entropy/` subsystem. Re-verified against the new code
> so the verdict is not over-claimed.

- **A real hardware-entropy layer now exists.** `src/entropy/hardware_entropy.c`
  (24 KB) actually reads `RDSEED` / `RDRAND` (Intel CPU TRNG instructions) and
  `/dev/random` (kernel pool), with health-test fallbacks. This is **genuine**
  classical hardware entropy — NOT quantum, but real (CPU/`/dev/random` are
  standard CSPRNG entropy sources). So the old line "the repo has no real entropy
  source" is **now false for `secure_rng`**; it remains true for the original
  `quantum_rng.c` core.
- **`secure_rng` still wraps the core quantum_rng.** `secure_rng.h` `#include`s
  `../quantum_rng/quantum_rng.h` and advertises "Quantum mixing for enhanced
  randomness" / `SECURE_RNG_MODE_QUANTUM` / `VERIFIED` (Bell-test) modes. The
  "quantum" word is **still marketing** over the same classical `quantum_rng.c`
  core — the new layer adds *real* hardware entropy (RDSEED/RDRAND/`/dev/random`)
  but no quantum process.
- **Core audit verdict UNCHANGED.** Re-ran `audit/determinism_test.c` after the
  merge: `same seed -> identical stream: YES (deterministic given seed)`. The
  original `quantum_rng.c` is still a deterministic, time-seeded, seed-ignoring
  PRNG. §1–§6 stand for that file.
- **Updated net verdict:** `tsotchke/quantum_rng` is no longer "just a time-seeded
  PRNG." It is now a **layered RNG**: a classical-but-competent core
  (`quantum_rng.c`, the audit target, still falsely marketed as quantum) PLUS a
  genuinely-real hardware-entropy collector (`secure_rng`/`entropy`) that mixes
  in RDSEED/RDRAND/`/dev/random`. The *honesty gap* remains on the **core** file's
  "quantum/non-deterministic/63.999872-bit" claims (§1, §4b); the *new* layer is
  real hardware entropy but is rhetorically bundled with the same "quantum" label.
- **Not yet independently benchmarked:** `secure_rng`'s health tests
  (`tests/health_tests_test.c`, 848 lines) and Bell-test path were NOT executed
  here (LLVM/build out of scope this pass). Treated as **unverified-but-plausible**
  real entropy, pending a build+run of `secure_rng_test.c`. The audit's bar
  ("no theorem without a check") means the *secure_rng* entropy claims are
  currently **unproven by us**, not confirmed.

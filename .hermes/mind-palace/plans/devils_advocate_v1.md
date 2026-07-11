# quantum_rng_audit — Triple Devil's-Advocate Audit (v1, Jul 11 2026)

Subject: `tsotchke/quantum_rng` (`src/quantum_rng/quantum_rng.c`, 398ad0c)
Method: 4-phase Claim→Verify→Risk→Mitigate, run 3×. Empirical: compile + run.

---

## PHASE 1 — Affirm & Steelman (strongest positive case)

The mixing design is *competent*: `splitmix64` avalanches well, the
`hadamard_mix` xorshift-style multiplies with distinct magic constants give good
diffusion, and there's a chi-square uniformity test in `tests/`. As a fast
**non-cryptographic** PRNG for games/simulations it's serviceable. The "quantum"
framing is at worst enthusiastic analogy.

## PHASE 2 — Attack & Devil's-Advocate (strongest counter)

**Claim A:** "Leverages quantum mechanical principles."
- Verify: read `quantum_noise`, `hadamard_gate`, `phase_gate`, `quantum_step`.
- Attack: zero quantum mechanics. `quantum_noise` = sin/cos/sqrt of a double;
  "Hadamard/Pauli gates" = splitmix64/xorshift; "qubits" = array indices 0..7.
  Magic constants (`QRNG_FINE_STRUCTURE`, etc.) are arbitrary 64-bit ints.
- Risk: readers/downstream "key exchange" examples believe real entropy exists.
- Mitigate: AUDIT.md §1 claim-vs-reality table; rename/doc recommendation.

**Claim B:** "Verified non-deterministic output."
- Verify: `audit/determinism_test.c` — two runs with same explicit seed.
- Attack: `qrng_init` folds in `gettimeofday`+`getpid`+`clock`+`rdtsc` EVEN when a
  seed is given → seed is decorative; stream keyed by init timestamp. Output is a
  deterministic function of (time, pid). "Non-deterministic" is false by design.
- Risk: any code relying on reproducible streams (tests, saved states) breaks.
- Mitigate: PR to make seed drive the stream; doc the time-seeding behavior.

**Claim C:** "High entropy output (63.999872 bits/sample)."
- Verify: grep README/docs for the number; search for code computing it.
- Attack: the number is hardcoded marketing text. No code computes it. No
  NIST SP 800-90B / Dieharder suite exists. Only chi-square *uniformity* (a PRNG
  can be uniform and still have ~0 true entropy if seeded from a clock).
- Risk: a "proven entropy" claim with no measurement is actively misleading.
- Mitigate: replace with a real statistic or remove; add a real entropy suite.

**Claim D:** "Builds with `make`."
- Attack: `make all` fails — `pid_t` without `<sys/types.h>`, `M_PI`/`M_E`
  undeclared. (Earlier audit noted this; confirmed by recompile.)
- Mitigate: one-line include fix; done in the audit wrapper.

## PHASE 3 — Triple Synthesis

quantum_rng is a **well-mixed classical PRNG wearing quantum clothes**, with
unsubstantiated entropy claims and a broken build. The fixes are small and
localized. This is an *invitation*: a PR that (1) fixes the build, (2) makes the
seed real, (3) replaces the hardcoded entropy number with a measurement, and
(4) honestly labels the physics terms — turns a finding into collaboration.

## PHASE 4 — Risk of OUR audit

- Confirmation bias: we *wanted* to debunk "quantum." **Mitigated:** we also note
  the mixing is competent and the chi-square test is real; we assume good-faith
  analogy, not malice.
- Measurement error: determinism_test uses wall-clock; two runs could in theory
  coincide. **Mitigated:** the seed-ignored behavior is structural (init always
  mixes time), provable from source, not a timing fluke.

## WuBuMath Proof Extension (the bridge)

Our pipeline shows the *rigorous* alternative tsotchke could adopt:
- Every numeric claim gets a C contract test (`make test` in WuBuMath).
- Every theorem gets a Lean proof (`lean/WubuProofs/`, 0 sorry).
- A claim like "63.999872 bits/sample" would be *computed* by a test, not typed.

Proposal: add to quantum_rng a `tests/entropy_suite.c` (NIST SP 800-90B
per-bit entropy estimate) and a Lean lemma bounding min-entropy of a
time-seeded generator — the same formalism WuBuMath uses for its RNG audit.

## Invitation to tsotchke (rigorous but welcoming)
"We audited quantum_rng. The mixing is solid; the 'quantum' is analogy and the
entropy number is hardcoded. We reproduced the seed-ignored behavior and the
broken build. Here's a PR: fix build + make seed real + measure entropy properly.
Happy to help formalize the entropy bound in Lean as a shared artifact."

# Triple Devil's-Advocate Suggestions — using WuBuMath as the independent oracle
### Targets: `tsotchke-audit` (15 repos) + `quantum_rng_audit`
### Method note: the oracle must be *real*. As of this session (2026-07-28):

- **WuBuMath C kernels are the only currently-valid oracle.** They compile and run:
  - `src/math/wubu_poincare_geom.c` (exp/log maps, geodesic distance, Christoffel, scalar curvature) — exercised by `src/tests/test_wubu_poincare_geom.c` and `test_wubu_manifold_ad.c`.
  - `src/math/wubu_hyperbolic.c`, rep-theory, manifold AD all present and tested.
  - The eshkol `cross-validation/` C port (a *standalone* copy of WuBuMath's Poincaré geometry) **runs** and numerically reproduces the documented F1 exp-map bug (`gap=0.0166`, convention-doc only).
- **WuBuMath's Lean proof library does NOT currently compile.** Verified by running `lake build WubuProofs` in `/home/wubu/WuBuMath/lean`: `PowerTower.lean` fails with ~26 real type errors (git `292cda2` "Still 26 errors (does not compile)"). So **Lean "proven / 0 sorry" claims are NOT a usable oracle** — and the audit trail is itself contested: `VALIDATION.md` (07-10) claims "0 sorry, all proven," the 07-12 amendment says "0 of them compile," and the live build confirms the 07-12 reading. This is the first 3×DA lever below.

> Therefore every suggestion below uses WuBuMath's **C/numeric** math as the cross-check, never its Lean prose. Where a tsotchke/quantum_rng claim is *about* formal proof, the valid DA move is to point at WuBuMath's *own* failure to compile as the counterexample to "rigor is achievable here."

--------------------------------------------------------------------------------
# A. SUGGESTIONS FOR `tsotchke-audit` (the 15-repo corpus)
--------------------------------------------------------------------------------

### A1 — "Provable correctness" is unprovable by construction (DA against tsotchke)
tsotchke advertises HoTT / provable foundations (eshkol) and "provable error
suppression O(ε²)" (quantum_geometric_tensor). WuBuMath — the *local* yardstick the
audit uses — has 15 Lean files yet **0 compile today**. If a focused, single-author
math library cannot keep its proofs green, then a 1.1M-LOC, 15-repo corpus with **0
proof files** has no plausible path to "provable correctness." The honest DA
conclusion: reclassify every "provable/rigorous" claim as *aspirational* until (a) a
proof artifact exists and (b) it builds in CI. Suggested action: add a CI gate that
greps for `sorry`/unproven in any claimed-proof path and fails, mirroring what
WuBuMath *should* but doesn't do.

### A2 — eshkol manifold geometry: verify against WuBuMath's C geodesic, not Lean
`eshkol/lib/core/manifold.esk` claims analytic Christoffel / sectional / Ricci /
scalar curvature. The audit already found F1 (exp-map vs distance norm mismatch,
`dist(p,exp_p(v))/|v|` drifts 2.83→4.96). **Cross-check with the running oracle:**
run `eshkol_audit/cross-validation/` (build+run confirmed this session) which
compares eshkol's analytic symbols against WuBuMath's independent RK4 geodesic. The
numeric result: Christoffel-vs-corrected-geodesic `maxgap=0.0166` (convention only),
but the *raw* eshkol exp-map still violates the geodesic invariant. Suggested action:
extend `cross-validation/` with a **scalar-curvature round-trip** — compute R from
WuBuMath's `wubu_poincare_geom` (K=−1 → R=−n(n−1)) and diff against eshkol's
`manifold.esk` Ricci trace on a known manifold; any nonzero diff is a real math bug,
not convention.

### A3 — libirrep is the one *real* exception; stress it harder with WuBuMath rep-theory
libirrep's SO(3)/SU(2) Clebsch-Gordan / Wigner-D passes 120k+ assertions — genuinely
correct. But the audit never cross-checked it against an *independent* implementation.
WuBuMath carries rep-theory kernels (`src/`, `wubu_riemannian_sgd`, hyperbolic/Lorentz
bridges). **Valid 3×DA step:** port libirrep's Wigner-D table to a WuBuMath-side
reference and assert bit-identical CG coefficients for j≤4. If they match, libirrep's
"verified-correct" claim is *upgraded* from self-tested to cross-validated (stronger).
If they diverge, one of the two has a silent convention bug (phase sign,
Condon-Shortley) — a real finding either way. This is a *valid* DA because it attacks
the one repo the audit gave a free pass.

### A4 — moonlab PROPRIETARY-gated claims can't be audited; that's a finding, not a pass
The audit marked moonlab's "32-qubit sim + PQC KEM" as ⚠️ because 4 PROPRIETARY
markers gate `live-hardware / GPU-cluster / customer-prem / calibration`. The DA
counter: **unauditable claims are unverified by definition** — scoring them ⚠️
("benefit of the doubt") is too generous. Suggested action: reclassify
PROPRIETARY-gated headline claims as **UNVERIFIED (closed)** and require the audit to
state exactly which files were inaccessible. WuBuMath's contrast here is useful:
WuBuMath's C kernels are *open and runnable* (this session ran them), so "open + runs"
is the bar; moonlab fails it by construction.

### A5 — quantum_rng entropy number: pin it with a reproducible Shannon estimator
tsotchke `quantum_rng` advertises "63.999872 bits/sample" (unsubstantiated) and its
*own* `qrng_get_entropy_estimate()` returns a meaningless ~1.3 bits/byte. The audit
measured ~8 bits/byte independently. **Use WuBuMath's numeric discipline as the
cross-check:** WuBuMath doesn't ship an entropy estimator, but the *method* does —
compute Shannon entropy the same way the audit did and pin the number in CI. The valid
DA move is to demand `quantum_rng` expose `qrng_get_entropy_estimate()` computed via
the *same* Shannon formula the audit used, and fail CI if it deviates >0.01 from a
fresh measurement. This converts a prose defect into a testable invariant.

--------------------------------------------------------------------------------
# B. SUGGESTIONS FOR `quantum_rng_audit` (the hardened RNG fork)
--------------------------------------------------------------------------------

### B1 — "VERIFIED" mode's CHSH certificate is self-referential
`secure_rng` VERIFIED mode runs a CHSH Bell test and refuses output if S≤2. But the
Bell test's *measurement randomness* is drawn from the **same** hardware-entropy pool
that seeds the generator. A DA asks: if the entropy source is subtly biased (the exact
failure RCT/APT are meant to catch), does the CHSH S-statistic still violate the bound
*because the bias is shared* between the test and the output? The honest move:
cross-check the CHSH computation against an **independent** simulator — port the CHSH
correlator to a *separate* PRNG (e.g. a fixed LCG) and confirm S still lands in
[2, 2√2] for a known entangled state. If the fork's CHSH only "passes" because of
shared entropy, an independent-PRNG run will show it. This is a *valid* structural
challenge, not a strawman.

### B2 — throughput claim ~10× overstated; pin it with a machine-stamped benchmark
README claims ~4.82M ops/sec; measured ~0.45M. The 07-12 audit fixed the narrative
but the *number* is still loose. **Valid DA action:** adopt WuBuMath's discipline —
WuBuMath pins numeric results to a specific build/CPU (`VALIDATION.md` records exact
claims). Require `quantum_rng` to print a machine-stamped throughput (`uname -m`,
compiler, `-O` flag, sample count) and fail CI if the published number differs by >2×
from a fresh measurement. Replaces "trust the README" with "trust the gate."

### B3 — seeded-mode reproducibility is real; now prove the *unseeded* path varies under containment
The fork correctly documents that seeded init reproduces the stream byte-for-byte and
unseeded is non-deterministic. A sharp DA: **show the unseeded path is
non-deterministic even under containerization** (same image, `--privileged`, pinned
CPU) — i.e. prove `gettimeofday`+`getpid` actually vary. The audit verified "differs
across runs" but not "differs when everything else is identical." Run 50 containers
from the same image; assert ≥1 distinct byte in the first 64 bytes across runs. If they
are ever identical, the "unseeded = random" claim is false (a real security finding).
The *method* (reproducible-container stress test) is the same rigor the audit applied
to entropy.

### B4 — ARM `RNDR` disabled-by-default is a silent entropy-quality downgrade
The fork disables direct `RNDR`/`RNDRRS` on Apple Silicon and falls back to
`/dev/random` (hardware-backed, fine). But the same code path on **Linux/ARM servers**
(Graviton, Ampere) also hits the disabled branch and may fall through to CPU jitter
(rated 5 bits/byte vs 8). The DA: on a Graviton box, does `entropy_init` actually
reach `RNDR`, or does it silently drop to jitter? If the quality estimator still
reports "8 bits/byte" while the source is jitter, that's a **silent mislabel** — the
same class of defect as the "63.99 bits" headline. The audit method (probe
CPUID/`FEAT_RNG` and log the *actually-selected* source) closes this.

### B5 — `qrng_get_entropy_estimate()` is a live internal inconsistency; replace it
The audit noted the internal estimator returns ~1.3 bits/byte (broken math at
`quantum_rng.c:491`) while measured stream entropy is ~8 bits/byte. This is not just a
headline-number issue — it is an *internally contradictory* piece of code that other
subsystems (reseeding thresholds, health gating) might read. **Valid DA action:**
delete or rewrite `qrng_get_entropy_estimate()` to compute the actual Shannon/Min-
entropy of a sampled window and assert it agrees with an independent measurement to
<0.1 bit. A self-inconsistent estimator is a latent correctness bug even if the
output stream is good.

--------------------------------------------------------------------------------
# C. Cross-cutting 3×DA verdict (the "turn it on WuBuMath too" pass)
--------------------------------------------------------------------------------

The audit's own methodology is triple — it must also attack the auditor's yardstick.
WuBuMath, used as the oracle here, fails its own rigor test:

- **C1 — WuBuMath Lean library does not compile (verified this session).** `lake
  build WubuProofs` fails on `PowerTower.lean` (~26 type errors). Any suggestion
  above that leans on "WuBuMath's proven math" must instead say "WuBuMath's *C*
  math." The audit's headline contrast ("tsotchke 0 proofs; WuBuMath 15 proven")
  is **currently false** — both sides ship 0 compiling proof artifacts. Restate it.
- **C2 — VALIDATION.md is stale/contested.** It claims "0 sorry, all proven" (07-10)
  while the live build and git `292cda2` contradict it. The audit should record the
  *build command and output*, not the doc's claim. (Same lesson as B2's machine-
  stamped benchmark.)
- **C3 — The eshkol F1 bug is the model for how to do this right.** It was caught by
  a *running* WuBuMath C port, not by reading Lean. Every A/B suggestion above follows
  that pattern: **run the C oracle, diff a numeric invariant, fail CI on divergence.**
  That is the only 3×DA move that survives contact with the fact that nobody's proofs
  compile.

**Bottom line for the two repos:** the suggestions are *valid* (not strawmen) because
each is anchored to a numeric invariant that WuBuMath's C kernels can actually compute
and CI can actually gate — while explicitly declining to trust either side's "formal
proof" marketing, since the audit's own oracle (WuBuMath Lean) is currently red.

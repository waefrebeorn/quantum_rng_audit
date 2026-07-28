# Owner feedback received — `tsotchke/quantum_rng` (upstream)

This records the upstream owner's responses to our audit work, so the next pass
is reconciled rather than stale. Updated 2026-07-28.

## 1. Our audit PR #6 was closed (not merged) by `tsotchke`

- **PR #6** "Devil's-advocate audit of quantum_rng (summary, evidence, proposed
  fixes)" — closed 2026-07-13 by `tsotchke`, **not merged**.
- **Owner's stated reasons** (verbatim summary):
  1. The PR targeted stale `main` instead of `master` (it had merged
     `origin/master` at `23154db` but the PR head still pointed at `main`), which
     GitHub surfaced as a stale target.
  2. The audit material was "internally stale" and should be **resubmitted
     separately only after its evidence and scope are reconciled with `master`**.
  3. He preferred the **focused production refactor in PR #7** over the audit
     branch, and closed #6 in favor of it.

**Our response (this is the polite, correct move):** we accept the close. We will
NOT re-push the #6 material as-is. A reconciled audit will be resubmitted against
`master` *after* #7 lands, with (a) the branch retargeted to `master` from the
start, and (b) scope narrowed to the falsifiable, code-backed claims only (entropy
number, CHSH self-reference, ARM `RNDR` downgrade, broken estimator), dropping the
prose that had gone stale.

## 2. The range-overflow fix (PR #5, RichardHoekstra) — already in our fork

`tsotchke` reviewed PR #5 and called it "a correct and well-reasoned fix"
(signed-overflow UB at the full `int32` span → compiler proves `range != 0` at
`-O2` and deletes the guard → `r % 0` SIGFPE; plus the `qrng_range64` wrap-to-
constant sibling).

**Verification:** our fork `quantum_rng_audit` **already contains this fix**:
- `qrng_range32` (quantum_rng.c:447) computes the span in `uint64_t`, guards
  `span > 0xFFFFFFFFULL`, and returns a raw draw for the full span — no
  signed-overflow, no `r % 0`.
- `qrng_range64` (quantum_rng.c:475) checks `range == 0` and returns
  `qrng_uint64(ctx)` (a uniform draw), not the constant `max`.

So we are aligned with the owner's accepted fix; no port needed. Good.

## 3. What the owner actually runs (evidence we lacked)

`tsotchke` noted `make verify_all` on macOS completes: **26/26 health tests,
23/23 secure-RNG integration tests, 8/8 thread-safety tests, 18/18 v3 tests.**
Our audit had said the health tests were "not yet run by us." That was a gap in
*our* evidence, not a defect in his code. A reconciled audit should either run
`make verify_all` ourselves or explicitly scope our claims to what we measured.

## 4. Action items for the next (reconciled) pass

- [ ] Retarget any new PR to `master`, not `main`.
- [ ] Re-run `make verify_all` locally; cite real test counts or scope claims to
      measured-only.
- [ ] Confirm PR #7's production refactor didn't change the entropy/CHSH paths our
      remaining claims touch; if it did, re-measure.
- [ ] Keep the 3×DA WuBuMath suggestions (3xDA_WuBuMath_SUGGESTIONS.md) as a
      *separate* doc — that analysis is about the audit methodology, not a PR to
      upstream.

# quantum_rng_audit — Overnight Map (Jul 11 2026)

## Quick Trunk
- Fork: /home/wubu/quantum_rng-audit → waefrebeorn/quantum_rng_audit (main)
- Upstream: tsotchke/quantum_rng (398ad0c)
- Audit C: audit/determinism_test.c + AUDIT.md
- Reference: NIST SP 800-90B / Dieharder (methodology; not yet in repo)

## Where We Are
- VERIFIED: classical time-seeded PRNG, not quantum; seed ignored; build broken;
  "63.999872 bits/sample" hardcoded.
- NOT VERIFIED: real entropy (no NIST/Dieharder suite exists upstream).

## Workstreams
A — upstream PR: fix build + make seed reproducible (invites collaboration)
B — replace hardcoded entropy number with real statistic / remove
C — add NIST/Dieharder entropy suite; doc "not for crypto"

## Data Not To Re-derive
- determinism_test reproduces the seed-ignored behavior (command in AGENTS.md).
- Only chi-square uniformity test exists upstream (not entropy).

## Fallback
If upstream unresponsive: keep fork as evidence; document in AUDIT.md.

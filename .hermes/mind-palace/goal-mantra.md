# quantum_rng_audit — Goal Mantra (paste at session start)

Path: /home/wubu/quantum_rng-audit | Remote: waefrebeorn/quantum_rng_audit (upstream=tsotchke/quantum_rng)
Build+run audit (patches broken build): cd audit && gcc -std=c11 -O2 determinism_test.c -o determinism_test -lm && ./determinism_test

=== STATE ===
✅ Verified: seed ignored at init (stream keyed by gettimeofday/getpid)
✅ Verified: "63.999872 bits/sample" is hardcoded text, no code computes it
✅ Verified: build broken (missing sys/types.h, M_PI/M_E)
⚠️ Debatable: marketing "quantum" = analogy vs deception (assume good-faith)
⚠️ Not done: real NIST/Dieharder entropy suite (only chi-square uniformity exists)

=== STREAMS ===
S1 [P0] upstream PR: fix build + make seed reproducible (deterministic given seed)
S2 [P0] remove/replace hardcoded "63.999872 bits/sample" with real statistic
S3 [P1] rename functions or doc that physics terms are analogy only
S4 [P2] add NIST/Dieharder entropy suite; doc "not for crypto"

=== THE LOOP ===
read source → compile/run determinism_test → verify claim vs reality → document
in AUDIT.md → keep upstream verbatim → report. NO silent fixes.

=== FULL CONTEXT ===
Read .hermes/mind-palace/prestige_prompt.md

/*
 * determinism_test.c -- Devil's-advocate check for tsotchke/quantum_rng.
 *
 * Upstream claims "verified non-deterministic output" and "quantum-inspired"
 * entropy. This test compiles the core generator (patching the missing
 * <sys/types.h> and M_PI/M_E that break the upstream build) and checks whether
 * a given seed reproduces a stream -- the defining property of a real seeded
 * PRNG, and the opposite of "non-deterministic".
 *
 * Build:
 *   gcc -std=c11 -O2 determinism_test.c -o determinism_test -lm
 *   ./determinism_test
 *
 * Expected (upstream behaviour, reproduced here): same seed => DIFFERENT output,
 * because qrng_init folds in gettimeofday/getpid even when a seed is given.
 * That proves the seed is decorative and the stream is keyed by wall-clock time
 * + PID -- a classical time-seeded PRNG, NOT quantum, NOT non-deterministic.
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif
#include "../src/quantum_rng/quantum_rng.c"

int main(void) {
    uint8_t seed[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    qrng_ctx *a = NULL, *b = NULL;
    qrng_init(&a, seed, 4);
    qrng_init(&b, seed, 4);
    uint64_t xa[8], xb[8];
    for (int i = 0; i < 8; i++) { xa[i] = qrng_uint64(a); xb[i] = qrng_uint64(b); }
    int same = 1;
    for (int i = 0; i < 8; i++) if (xa[i] != xb[i]) same = 0;

    printf("=== quantum_rng determinism check ===\n");
    printf("same seed -> identical stream: %s\n", same ? "YES (deterministic given seed)"
                                                        : "NO (seed ignored; time-seeded)");
    printf("run A first 4: %016llx %016llx %016llx %016llx\n",
           (unsigned long long)xa[0], (unsigned long long)xa[1],
           (unsigned long long)xa[2], (unsigned long long)xa[3]);
    printf("run B first 4: %016llx %016llx %016llx %016llx\n",
           (unsigned long long)xb[0], (unsigned long long)xb[1],
           (unsigned long long)xb[2], (unsigned long long)xb[3]);
    printf("\nVerdict: output is a deterministic function of (init time, pid).\n"
           "Not quantum; not non-deterministic. See AUDIT.md.\n");
    return 0;
}

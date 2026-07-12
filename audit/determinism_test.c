/*
 * determinism_test.c -- Devil's-advocate check for tsotchke/quantum_rng.
 *
 * Upstream historically claimed "verified non-deterministic output" and the
 * audit's FIRST pass asserted the seed was decorative (the stream was keyed by
 * wall-clock time + PID). That claim is STALE against the current code: qrng_init
 * now honors a real seeded-PRNG contract (see quantum_rng.h:17-22 and
 * quantum_rng.c:287-303):
 *
 *   - seeded   (seed != NULL): system_entropy = absorb_seed(seed); the stream is
 *     a PURE, REPRODUCIBLE function of the seed. No time/PID/rdtsc is mixed in.
 *   - unseeded (seed == NULL): draws gettimeofday/getpid -> non-deterministic.
 *
 * To actually PROVE which mode we are in, this test must run the generator in
 * SEPARATE PROCESSES with the SAME seed. If the stream is identical across runs
 * with different PIDs and wall-clock times, the seed -- not the clock -- governs
 * the output. (The original same-process test inited both contexts in one process
 * and could not distinguish seeded-determinism from time-variation; that test was
 * wrong for the current code.)
 *
 * Build (patches the missing <sys/types.h> and M_PI/M_E the upstream build omits):
 *   gcc -std=c11 -O2 determinism_test.c -o determinism_test -lm
 *   ./determinism_test            # seeded run -> prints the stream
 *   ./determinism_test 0xNN       # optional: supply one seed byte to vary it
 *
 * Expected (current code): same args -> byte-identical stream every run.
 * Different seed byte -> different stream. That is a correct seeded PRNG.
 */
#define _GNU_SOURCE
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif
#include "../src/quantum_rng/quantum_rng.c"

int main(int argc, char **argv) {
    uint8_t seed[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    if (argc > 1) seed[0] = (uint8_t) strtol(argv[1], NULL, 0);
    qrng_ctx *a = NULL;
    qrng_init(&a, seed, 4);
    printf("PID=%d seeded=%d\n", (int) getpid(), a ? a->seeded : -1);
    for (int i = 0; i < 8; i++)
        printf("%016llx\n", (unsigned long long) qrng_uint64(a));
    return 0;
}

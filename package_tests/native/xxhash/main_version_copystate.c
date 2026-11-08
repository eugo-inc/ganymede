/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xxhash: version copystate; calls XXH_versionNumber(), XXH3_createState(), XXH3_64bits_reset(), XXH3_64bits_update().
 */

#include <xxhash.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* (a) runtime ABI version matches the header macros. */
    unsigned int runtime = XXH_versionNumber();
    unsigned int compile =
        (unsigned int)XXH_VERSION_MAJOR * 100u * 100u +
        (unsigned int)XXH_VERSION_MINOR * 100u +
        (unsigned int)XXH_VERSION_RELEASE;
    assert(runtime == compile);

    static const char prefix[] = "shared prefix bytes that go through both";
    static const char tail_a[] = "branch-A continuation";
    static const char tail_b[] = "branch-B continuation differs";
    const size_t np = sizeof(prefix) - 1;
    const size_t na = sizeof(tail_a) - 1;
    const size_t nb = sizeof(tail_b) - 1;

    XXH3_state_t *src = XXH3_createState();
    XXH3_state_t *cp  = XXH3_createState();
    assert(src != NULL);
    assert(cp  != NULL);

    assert(XXH3_64bits_reset(src) == XXH_OK);
    assert(XXH3_64bits_update(src, prefix, np) == XXH_OK);

    /* Clone. */
    XXH3_copyState(cp, src);

    /* Digest both immediately — must match (the copy is at parity). */
    {
        XXH3_state_t *probe_src = XXH3_createState();
        XXH3_state_t *probe_cp  = XXH3_createState();
        assert(probe_src != NULL);
        assert(probe_cp  != NULL);
        XXH3_copyState(probe_src, src);
        XXH3_copyState(probe_cp,  cp);
        XXH64_hash_t d_src = XXH3_64bits_digest(probe_src);
        XXH64_hash_t d_cp  = XXH3_64bits_digest(probe_cp);
        assert(d_src == d_cp);
        XXH3_freeState(probe_src);
        XXH3_freeState(probe_cp);
    }

    /* Feed DIFFERENT tails into each — digests must diverge. */
    {
        XXH3_state_t *div_src = XXH3_createState();
        XXH3_state_t *div_cp  = XXH3_createState();
        assert(div_src != NULL);
        assert(div_cp  != NULL);
        XXH3_copyState(div_src, src);
        XXH3_copyState(div_cp,  cp);
        assert(XXH3_64bits_update(div_src, tail_a, na) == XXH_OK);
        assert(XXH3_64bits_update(div_cp,  tail_b, nb) == XXH_OK);
        XXH64_hash_t d_src = XXH3_64bits_digest(div_src);
        XXH64_hash_t d_cp  = XXH3_64bits_digest(div_cp);
        assert(d_src != d_cp);
        XXH3_freeState(div_src);
        XXH3_freeState(div_cp);
    }

    /* Feed the SAME tail into each — digests must reconverge. */
    {
        XXH3_state_t *same_src = XXH3_createState();
        XXH3_state_t *same_cp  = XXH3_createState();
        assert(same_src != NULL);
        assert(same_cp  != NULL);
        XXH3_copyState(same_src, src);
        XXH3_copyState(same_cp,  cp);
        assert(XXH3_64bits_update(same_src, tail_a, na) == XXH_OK);
        assert(XXH3_64bits_update(same_cp,  tail_a, na) == XXH_OK);
        XXH64_hash_t d_src = XXH3_64bits_digest(same_src);
        XXH64_hash_t d_cp  = XXH3_64bits_digest(same_cp);
        assert(d_src == d_cp);
        XXH3_freeState(same_src);
        XXH3_freeState(same_cp);
    }

    XXH3_freeState(src);
    XXH3_freeState(cp);

    printf("xxhash version+copystate test OK: runtime=%u\n", runtime);
    return 0;
}

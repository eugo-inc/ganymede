/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cpuinfo: cache; calls cpuinfo_initialize(), cpuinfo_get_l1d_caches_count(), cpuinfo_get_l1i_caches_count(), cpuinfo_get_l2_caches_count().
 */

#include <cpuinfo.h>
#include <assert.h>
#include <stdio.h>

static int is_power_of_two(uint32_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

int main(void) {
    assert(cpuinfo_initialize());

    const uint32_t l1d_n = cpuinfo_get_l1d_caches_count();
    const uint32_t l1i_n = cpuinfo_get_l1i_caches_count();
    const uint32_t l2_n = cpuinfo_get_l2_caches_count();
    assert(l1d_n >= 1);
    assert(l1i_n >= 1);
    assert(l2_n >= 1);

    const struct cpuinfo_cache *l1d = cpuinfo_get_l1d_cache(0);
    const struct cpuinfo_cache *l1i = cpuinfo_get_l1i_cache(0);
    const struct cpuinfo_cache *l2 = cpuinfo_get_l2_cache(0);
    assert(l1d != NULL);
    assert(l1i != NULL);
    assert(l2 != NULL);

    /* Sane geometry: non-zero size + power-of-two line_size + non-zero
     * associativity + L1d <= L2 size (inclusive of unified L2). */
    assert(l1d->size > 0);
    assert(l1i->size > 0);
    assert(l2->size > 0);
    assert(is_power_of_two(l1d->line_size));
    assert(is_power_of_two(l1i->line_size));
    assert(is_power_of_two(l2->line_size));
    assert(l1d->associativity >= 1);
    assert(l1i->associativity >= 1);
    assert(l2->associativity >= 1);
    assert(l1d->size <= l2->size);

    /* max-cache-size aggregator must be at least the L2 size of CPU 0. */
    const uint32_t max_cache = cpuinfo_get_max_cache_size();
    assert(max_cache >= l2->size);

    cpuinfo_deinitialize();

    printf("cpuinfo cache test OK: l1d=%uB l1i=%uB l2=%uB max=%uB\n",
           l1d->size, l1i->size, l2->size, max_cache);
    return 0;
}

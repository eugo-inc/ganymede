/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libnuma: symver; calls dlvsym(), numa_max_possible_node(), numa_available().
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <numa.h>

#include <stdio.h>

int main(void) {
    void *old_abi = dlvsym(RTLD_DEFAULT, "numa_node_to_cpus", "libnuma_1.1");
    void *new_abi = dlvsym(RTLD_DEFAULT, "numa_node_to_cpus", "libnuma_1.2");
    if (old_abi == NULL || new_abi == NULL || old_abi == new_abi) {
        fprintf(stderr, "versioned numa_node_to_cpus: libnuma_1.1=%p libnuma_1.2=%p (need two distinct)\n", old_abi,
                new_abi);
        return 1;
    }

    const int possible = numa_max_possible_node();
    if (possible < 0) {
        fprintf(stderr, "numa_max_possible_node returned %d\n", possible);
        return 1;
    }

    printf("libnuma: both numa_node_to_cpus versions present; numa_available()=%d, max possible node %d\n",
           numa_available(), possible);
    return 0;
}

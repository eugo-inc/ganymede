/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for mimalloc: preload override; calls in_heap(), version().
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *kPreload = "/usr/local/lib64/libmimalloc.so.3";

static int thp_enabled_field(void) {
    FILE *f = fopen("/proc/self/status", "r");
    char line[256];
    int value = -1;
    if (f == NULL) {
        return -1;
    }
    while (fgets(line, sizeof line, f) != NULL) {
        if (strncmp(line, "THP_enabled:", 12) == 0) {
            value = atoi(line + 12);
        }
    }
    fclose(f);
    return value;
}

int main(int argc, char **argv) {
    const char *stage = getenv("DEMO_MIMALLOC_TEST_STAGE");
    if (stage == NULL) {
        // Stage 0: re-run twice, with the preload and default huge pages, then with MIMALLOC_ALLOW_THP=0.
        int rc_all = 0;
        for (int round = 1; round <= 2; round++) {
            char cmd[1024];
            snprintf(cmd, sizeof cmd, "env -u MIMALLOC_ALLOW_THP LD_PRELOAD=%s DEMO_MIMALLOC_TEST_STAGE=%d %s '%s'",
                     kPreload, round, round == 2 ? "MIMALLOC_ALLOW_THP=0" : "", argv[0]);
            int rc = system(cmd);
            if (rc != 0) {
                fprintf(stderr, "FAIL stage %d exited %d\n", round, rc);
                rc_all = 1;
            }
        }
        return rc_all;
    }
    bool (*in_heap)(const void *) = (bool (*)(const void *))dlsym(RTLD_DEFAULT, "mi_is_in_heap_region");
    int (*version)(void) = (int (*)(void))dlsym(RTLD_DEFAULT, "mi_version");
    if (in_heap == NULL || version == NULL) {
        fprintf(stderr, "FAIL stage %s: mimalloc is not loaded (dlsym mi_is_in_heap_region/mi_version)\n", stage);
        return 1;
    }
    void *p = malloc(4096);
    if (p == NULL || !in_heap(p)) {
        fprintf(stderr, "FAIL stage %s: malloc returned %p outside mimalloc's heap under LD_PRELOAD\n", stage, p);
        return 1;
    }
    free(p);
    int thp = thp_enabled_field();
    // THP_enabled is -1 here only on a kernel without that /proc field; then the switch is not checked.
    if (strcmp(stage, "2") == 0 && thp != 0 && thp != -1) {
        fprintf(stderr, "FAIL stage 2: MIMALLOC_ALLOW_THP=0 but THP_enabled is %d\n", thp);
        return 1;
    }
    if (strcmp(stage, "1") == 0 && thp == 0) {
        fprintf(stderr, "FAIL stage 1: huge pages are off although MIMALLOC_ALLOW_THP is unset\n");
        return 1;
    }
    printf("OK stage %s: preloaded mimalloc %d serves malloc; THP_enabled %d\n", stage, version(), thp);
    return 0;
}

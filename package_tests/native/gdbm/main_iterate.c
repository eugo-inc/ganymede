/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for gdbm: iterate; calls gdbm_open(), gdbm_store(), gdbm_firstkey(), gdbm_nextkey().
 */

#include <gdbm.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_gdbm_iter_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
    unlink(path);

    GDBM_FILE db = gdbm_open(path, 0, GDBM_NEWDB, 0600, NULL);
    assert(db != NULL);

    const char *keys[] = {"alpha", "beta", "gamma", "delta", "epsilon"};
    const int N = (int)(sizeof(keys) / sizeof(keys[0]));
    for (int i = 0; i < N; i++) {
        datum k = { (char *)keys[i], (int)strlen(keys[i]) };
        datum v = { (char *)"v", 1 };
        assert(gdbm_store(db, k, v, GDBM_INSERT) == 0);
    }

    int seen[5] = {0, 0, 0, 0, 0};
    int total = 0;
    datum cur = gdbm_firstkey(db);
    while (cur.dptr != NULL) {
        int matched = 0;
        for (int i = 0; i < N; i++) {
            if (cur.dsize == (int)strlen(keys[i]) &&
                memcmp(cur.dptr, keys[i], cur.dsize) == 0) {
                assert(seen[i] == 0); /* each key visited at most once */
                seen[i] = 1;
                matched = 1;
                break;
            }
        }
        assert(matched == 1);
        total++;
        datum next = gdbm_nextkey(db, cur);
        free(cur.dptr);
        cur = next;
    }
    assert(total == N);
    for (int i = 0; i < N; i++) {
        assert(seen[i] == 1);
    }

    gdbm_close(db);
    unlink(path);
    printf("gdbm iterate test OK: visited %d keys\n", total);
    return 0;
}

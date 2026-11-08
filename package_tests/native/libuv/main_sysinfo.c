/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libuv: sysinfo; calls uv_cpu_info(), uv_free_cpu_info(), uv_hrtime(), uv_get_total_memory().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <uv.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* CPU info: at least one CPU on any sane host. */
    uv_cpu_info_t *cpus = NULL;
    int count = 0;
    int r = uv_cpu_info(&cpus, &count);
    assert(r == 0);
    assert(count >= 1);
    assert(cpus != NULL);
    assert(cpus[0].model != NULL);
    assert(strlen(cpus[0].model) > 0);
    uv_free_cpu_info(cpus, count);

    /* hrtime: strictly monotonic across two consecutive calls. */
    uint64_t t0 = uv_hrtime();
    uint64_t t1 = uv_hrtime();
    assert(t1 >= t0);

    /* total memory: any real machine reports > 1 MiB. */
    uint64_t total = uv_get_total_memory();
    assert(total > (1024ULL * 1024ULL));

    /* resident set: this process has touched some pages. */
    size_t rss = 0;
    r = uv_resident_set_memory(&rss);
    assert(r == 0);
    assert(rss > 0);

    /* uv_os_tmpdir: non-empty buffer. */
    char tmpbuf[512];
    size_t tmplen = sizeof(tmpbuf);
    r = uv_os_tmpdir(tmpbuf, &tmplen);
    assert(r == 0);
    assert(tmplen > 0);
    assert(strlen(tmpbuf) == tmplen);

    /* uv_os_homedir: non-empty buffer. */
    char homebuf[512];
    size_t homelen = sizeof(homebuf);
    r = uv_os_homedir(homebuf, &homelen);
    assert(r == 0);
    assert(homelen > 0);
    assert(strlen(homebuf) == homelen);

    /* uv_version: positive packed semver. */
    unsigned int v = uv_version();
    assert(v > 0);

    printf("libuv sysinfo test OK: cpus=%d, tmpdir=%s\n", count, tmpbuf);
    return 0;
}

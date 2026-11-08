/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nghttp2: priority; calls nghttp2_priority_spec_init(), nghttp2_priority_spec_check_default(), nghttp2_priority_spec_default_init().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp2/nghttp2.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    nghttp2_priority_spec pri;
    nghttp2_priority_spec_init(&pri, /*stream_id=*/3, /*weight=*/42, /*exclusive=*/1);

    assert(pri.stream_id == 3);
    assert(pri.weight == 42);
    assert(pri.exclusive == 1);

    int is_default = nghttp2_priority_spec_check_default(&pri);
    assert(is_default == 0);

    nghttp2_priority_spec defpri;
    nghttp2_priority_spec_default_init(&defpri);

    assert(defpri.weight == NGHTTP2_DEFAULT_WEIGHT);
    assert(defpri.stream_id == 0);
    assert(defpri.exclusive == 0);

    is_default = nghttp2_priority_spec_check_default(&defpri);
    assert(is_default != 0);

    printf("nghttp2 priority-spec OK: default weight=%d\n", defpri.weight);
    return 0;
}

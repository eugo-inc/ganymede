/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nghttp3: pri; calls nghttp3_pri_parse_priority().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp3/nghttp3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int parse(nghttp3_pri *dest, const char *s) {
    return nghttp3_pri_parse_priority(dest, (const uint8_t *)s, strlen(s));
}

int main(void) {
    /* 1) The documented urgency range. */
    assert(NGHTTP3_URGENCY_HIGH == 0);
    assert(NGHTTP3_URGENCY_LOW == 7);
    assert(NGHTTP3_DEFAULT_URGENCY >= NGHTTP3_URGENCY_HIGH);
    assert(NGHTTP3_DEFAULT_URGENCY <= NGHTTP3_URGENCY_LOW);
    assert(NGHTTP3_URGENCY_LEVELS == NGHTTP3_URGENCY_LOW + 1);

    /* 2) "u=1" parses to urgency 1; inc retains its prior value. */
    {
        nghttp3_pri pri = {NGHTTP3_DEFAULT_URGENCY, 0};
        int rv = parse(&pri, "u=1");
        assert(rv == 0);
        assert(pri.urgency == 1);
        assert(pri.inc == 0);
    }

    /* 3) "u=5, i" -> urgency 5, inc 1. */
    {
        nghttp3_pri pri = {NGHTTP3_DEFAULT_URGENCY, 0};
        int rv = parse(&pri, "u=5, i");
        assert(rv == 0);
        assert(pri.urgency == 5);
        assert(pri.inc == 1);
    }

    /* 4) "i=?0" -> inc explicitly 0; urgency unchanged from input. */
    {
        nghttp3_pri pri = {2, 1};
        int rv = parse(&pri, "i=?0");
        assert(rv == 0);
        assert(pri.urgency == 2);
        assert(pri.inc == 0);
    }

    /* 5) Empty Priority: header value is well-formed -> dest unchanged. */
    {
        nghttp3_pri pri = {NGHTTP3_DEFAULT_URGENCY, 0};
        int rv = parse(&pri, "");
        assert(rv == 0);
        assert(pri.urgency == NGHTTP3_DEFAULT_URGENCY);
        assert(pri.inc == 0);
    }

    /* 6) u=8 is one past NGHTTP3_URGENCY_LOW -> rejected. */
    {
        nghttp3_pri pri = {NGHTTP3_DEFAULT_URGENCY, 0};
        int rv = parse(&pri, "u=8");
        assert(rv == NGHTTP3_ERR_INVALID_ARGUMENT);
    }

    /* 7) Malformed structured-fields input -> rejected. */
    {
        nghttp3_pri pri = {NGHTTP3_DEFAULT_URGENCY, 0};
        int rv = parse(&pri, "this is not a valid sf-dict");
        assert(rv == NGHTTP3_ERR_INVALID_ARGUMENT);
    }

    printf("nghttp3 pri parse test OK\n");
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for harfbuzz: core API; calls hb_version(), hb_version_string(), hb_buffer_create(), hb_buffer_allocation_successful().
 */

#include <hb.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    unsigned int major = 0, minor = 0, micro = 0;
    hb_version(&major, &minor, &micro);
    assert(major >= 1);

    const char *vs = hb_version_string();
    assert(vs != NULL);
    assert(strlen(vs) > 0);

    hb_buffer_t *buf = hb_buffer_create();
    assert(buf != NULL);
    assert(hb_buffer_allocation_successful(buf));

    hb_buffer_add_utf8(buf, "hello", -1, 0, -1);
    hb_buffer_guess_segment_properties(buf);

    /* Direction must be horizontal LTR for ASCII Latin. */
    assert(hb_buffer_get_direction(buf) == HB_DIRECTION_LTR);
    /* Script auto-guess for Latin ASCII. */
    assert(hb_buffer_get_script(buf) == HB_SCRIPT_LATIN);
    assert(hb_buffer_get_length(buf) == 5);

    hb_buffer_destroy(buf);

    printf("harfbuzz test OK: %s\n", vs);
    return 0;
}

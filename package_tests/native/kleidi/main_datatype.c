/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for kleidi: datatype; calls kai_get_datatype_size_in_bytes(), kai_roundup().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <kai/kai_common.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Single-byte types. */
    assert(kai_get_datatype_size_in_bytes(kai_dt_int8) == 1);
    assert(kai_get_datatype_size_in_bytes(kai_dt_uint8) == 1);

    /* Two-byte types. */
    assert(kai_get_datatype_size_in_bytes(kai_dt_f16) == 2);
    assert(kai_get_datatype_size_in_bytes(kai_dt_bf16) == 2);
    assert(kai_get_datatype_size_in_bytes(kai_dt_int16) == 2);
    assert(kai_get_datatype_size_in_bytes(kai_dt_uint16) == 2);

    /* Four-byte types. */
    assert(kai_get_datatype_size_in_bytes(kai_dt_f32) == 4);
    assert(kai_get_datatype_size_in_bytes(kai_dt_int32) == 4);
    assert(kai_get_datatype_size_in_bytes(kai_dt_uint32) == 4);
    assert(kai_get_datatype_size_in_bytes(kai_dt_bool) == 4);

    assert(kai_get_datatype_size_in_bytes(kai_dt_unknown) == 0);

    /* kai_roundup: pad a 13-byte tensor row up to a 16-byte boundary
     * (the canonical KleidiAI use case for the helper). */
    assert(kai_roundup(13, 16) == 16);
    assert(kai_roundup(16, 16) == 16);
    assert(kai_roundup(17, 16) == 32);
    assert(kai_roundup(0, 1) == 0);
    assert(kai_roundup(1, 1) == 1);

    printf("kleidi datatype test OK\n");
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libraqm: utf variants; calls raqm_create(), raqm_set_text(), raqm_destroy(), raqm_set_text_utf16().
 */

#include <raqm.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* UTF-32: U+0041 'A', U+0042 'B', U+05D0 Hebrew Alef, U+0627 Arabic Alif. */
    const uint32_t u32[] = { 0x0041, 0x0042, 0x05D0, 0x0627 };
    const size_t u32_len = sizeof(u32) / sizeof(u32[0]);

    raqm_t *rq32 = raqm_create();
    assert(rq32 != NULL);
    assert(raqm_set_text(rq32, u32, u32_len) == 1);
    raqm_destroy(rq32);

    /* UTF-16: same characters in 16-bit code units (all BMP, no surrogates). */
    const uint16_t u16[] = { 0x0041, 0x0042, 0x05D0, 0x0627 };
    const size_t u16_len = sizeof(u16) / sizeof(u16[0]);

    raqm_t *rq16 = raqm_create();
    assert(rq16 != NULL);
    assert(raqm_set_text_utf16(rq16, u16, u16_len) == 1);
    raqm_destroy(rq16);

    raqm_t *rqz = raqm_create();
    assert(rqz != NULL);
    assert(raqm_set_text(rqz, u32, 0) == 1);
    raqm_destroy(rqz);

    printf("libraqm UTF-32/UTF-16 test OK\n");
    return 0;
}

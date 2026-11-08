/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_mqtt: validate utf8; calls aws_default_allocator(), aws_mqtt_library_init(), aws_byte_cursor_from_c_str(), aws_mqtt_validate_utf8_text().
 */

#include <aws/mqtt/mqtt.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/common.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct aws_byte_cursor cursor_from_bytes(const unsigned char *bytes, size_t len) {
    struct aws_byte_cursor c;
    c.ptr = (uint8_t *)bytes;
    c.len = len;
    return c;
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_mqtt_library_init(alloc);

    /* (1) Plain ASCII — must validate. */
    struct aws_byte_cursor ascii = aws_byte_cursor_from_c_str("demo/healthz");
    assert(aws_mqtt_validate_utf8_text(ascii) == AWS_OP_SUCCESS);

    /* (2) Valid multibyte UTF-8 — Cyrillic "Привет" (0xD0 0x9F 0xD1 0x80 0xD0
     *     0xB8 0xD0 0xB2 0xD0 0xB5 0xD1 0x82). Must validate — exercises the
     *     multibyte decode path. */
    static const unsigned char cyrillic[] = {
        0xD0, 0x9F, 0xD1, 0x80, 0xD0, 0xB8, 0xD0, 0xB2, 0xD0, 0xB5, 0xD1, 0x82,
    };
    assert(aws_mqtt_validate_utf8_text(cursor_from_bytes(cyrillic, sizeof(cyrillic))) == AWS_OP_SUCCESS);

    /* (3) An embedded NUL must reject. aws_byte_cursor_from_c_str would stop at the
     *     NUL, so the cursor is built explicitly. */
    static const unsigned char embedded_nul[] ={'e', 'u', 'g', 'o', 0x00, 'z'};
    assert(aws_mqtt_validate_utf8_text(cursor_from_bytes(embedded_nul, sizeof(embedded_nul))) == AWS_OP_ERR);

    /* (4) U+0001 (C0 control) must reject. */
    static const unsigned char c0_ctrl[] = {'a', 0x01, 'b'};
    assert(aws_mqtt_validate_utf8_text(cursor_from_bytes(c0_ctrl, sizeof(c0_ctrl))) == AWS_OP_ERR);

    /* (5) U+0085 NEL (C1 control) — UTF-8 encoded as 0xC2 0x85 — must reject. */
    static const unsigned char c1_ctrl[] = {'a', 0xC2, 0x85, 'b'};
    assert(aws_mqtt_validate_utf8_text(cursor_from_bytes(c1_ctrl, sizeof(c1_ctrl))) == AWS_OP_ERR);

    /* (6) U+FFFE Unicode non-character — UTF-8 encoded as 0xEF 0xBF 0xBE — must reject. */
    static const unsigned char nonchar[] = {'a', 0xEF, 0xBF, 0xBE, 'b'};
    assert(aws_mqtt_validate_utf8_text(cursor_from_bytes(nonchar, sizeof(nonchar))) == AWS_OP_ERR);

    aws_mqtt_library_clean_up();
    printf("aws_c_mqtt validate-utf8 OK\n");
    return 0;
}

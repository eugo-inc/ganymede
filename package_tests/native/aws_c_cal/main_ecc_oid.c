/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_cal: ecc oid; calls aws_ecc_oid_from_curve_name(), aws_ecc_curve_name_from_oid(), aws_default_allocator(), aws_cal_library_init().
 */

#include <aws/cal/cal.h>
#include <aws/cal/ecc.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void check_round_trip(enum aws_ecc_curve_name curve) {
    struct aws_byte_cursor oid;
    AWS_ZERO_STRUCT(oid);

    int rc = aws_ecc_oid_from_curve_name(curve, &oid);
    assert(rc == AWS_OP_SUCCESS);
    assert(oid.ptr != NULL);
    assert(oid.len > 0);

    enum aws_ecc_curve_name recovered = (enum aws_ecc_curve_name)-1;
    rc = aws_ecc_curve_name_from_oid(&oid, &recovered);
    assert(rc == AWS_OP_SUCCESS);
    assert(recovered == curve);
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_cal_library_init(alloc);

    check_round_trip(AWS_CAL_ECDSA_P256);
    check_round_trip(AWS_CAL_ECDSA_P384);

    /* ---- Cross-curve distinctness: P-256 and P-384 must have DIFFERENT OIDs
       (catches a copy-paste bug where both names map to the same entry). */
    struct aws_byte_cursor oid_p256;
    struct aws_byte_cursor oid_p384;
    AWS_ZERO_STRUCT(oid_p256);
    AWS_ZERO_STRUCT(oid_p384);
    assert(aws_ecc_oid_from_curve_name(AWS_CAL_ECDSA_P256, &oid_p256) == AWS_OP_SUCCESS);
    assert(aws_ecc_oid_from_curve_name(AWS_CAL_ECDSA_P384, &oid_p384) == AWS_OP_SUCCESS);
    assert(oid_p256.len != oid_p384.len ||
           memcmp(oid_p256.ptr, oid_p384.ptr, oid_p256.len) != 0);

    /* ---- Negative: a garbage OID must NOT decode. Use a fabricated 4-byte
       blob that doesn't match any real OID encoding for a P-curve. */
    static const unsigned char bogus_oid_bytes[] = {0xDE, 0xAD, 0xBE, 0xEF};
    struct aws_byte_cursor bogus = aws_byte_cursor_from_array(bogus_oid_bytes, sizeof(bogus_oid_bytes));
    enum aws_ecc_curve_name garbage = (enum aws_ecc_curve_name)-1;
    int rc = aws_ecc_curve_name_from_oid(&bogus, &garbage);
    assert(rc != AWS_OP_SUCCESS);

    aws_cal_library_clean_up();

    printf("aws_c_cal ECC curve-OID round-trip + negative test OK\n");
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ldns: buffer; calls ldns_buffer_new(), ldns_buffer_status_ok(), ldns_dname_new_frm_str(), ldns_rdf2buffer_wire().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <ldns/ldns.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    ldns_buffer *buf = ldns_buffer_new(256);
    assert(buf != NULL);
    assert(ldns_buffer_status_ok(buf));

    ldns_rdf *dname = ldns_dname_new_frm_str("example.com.");
    assert(dname != NULL);

    /* Serialize the dname onto the buffer in DNS wire format. */
    ldns_status st = ldns_rdf2buffer_wire(buf, dname);
    assert(st == LDNS_STATUS_OK);
    assert(ldns_buffer_status_ok(buf));

    /* Wire-encoded "example.com." occupies exactly 13 bytes. */
    assert(ldns_buffer_position(buf) == 13);

    /* Verify the length-prefixed label bytes. */
    const uint8_t *wire = ldns_buffer_begin(buf);
    assert(wire[0] == 7);                          /* length of "example" */
    assert(memcmp(wire + 1, "example", 7) == 0);
    assert(wire[8] == 3);                          /* length of "com" */
    assert(memcmp(wire + 9, "com", 3) == 0);
    assert(wire[12] == 0);                         /* root label terminator */

    ldns_rdf_deep_free(dname);
    ldns_buffer_free(buf);

    printf("ldns buffer probe OK\n");
    return 0;
}

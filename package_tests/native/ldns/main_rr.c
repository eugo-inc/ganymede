/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ldns: rr; calls ldns_rr_new_frm_str(), ldns_rr_get_type(), ldns_rr_get_class(), ldns_rr_ttl().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <ldns/ldns.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    ldns_rr *rr = NULL;

    /* Parse a canonical A record from zonefile syntax. */
    ldns_status st = ldns_rr_new_frm_str(
        &rr,
        "example.com. 3600 IN A 192.0.2.1",
        3600,
        NULL,
        NULL);

    assert(st == LDNS_STATUS_OK);
    assert(rr != NULL);

    /* Type must be A (1). */
    assert(ldns_rr_get_type(rr) == LDNS_RR_TYPE_A);

    /* Class must be IN (1). */
    assert(ldns_rr_get_class(rr) == LDNS_RR_CLASS_IN);

    assert(ldns_rr_ttl(rr) == 3600);

    const ldns_rdf *owner = ldns_rr_owner(rr);
    assert(owner != NULL);
    assert(ldns_dname_label_count(owner) == 2);

    /* A record carries exactly one rdata field (the IPv4 address). */
    assert(ldns_rr_rd_count(rr) == 1);

    ldns_rr_free(rr);

    printf("ldns rr probe OK\n");
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ldns: rdf a; calls ldns_rdf_new_frm_str(), ldns_rdf_get_type(), ldns_rdf_size(), ldns_rdf_data().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <ldns/ldns.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    /* Parse "192.0.2.1" as an IPv4 (A-record) RDF. */
    ldns_rdf *a = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, "192.0.2.1");
    assert(a != NULL);

    /* The RDF must report type A. */
    assert(ldns_rdf_get_type(a) == LDNS_RDF_TYPE_A);

    /* Packed IPv4 address is exactly 4 bytes on the wire. */
    assert(ldns_rdf_size(a) == 4);

    /* Raw bytes must match the parsed dotted-quad. */
    const uint8_t *bytes = ldns_rdf_data(a);
    assert(bytes != NULL);
    assert(bytes[0] == 192);
    assert(bytes[1] == 0);
    assert(bytes[2] == 2);
    assert(bytes[3] == 1);

    char *s = ldns_rdf2str(a);
    assert(s != NULL);
    assert(strstr(s, "192.0.2.1") != NULL);
    free(s);

    ldns_rdf_deep_free(a);

    printf("ldns rdf_a probe OK\n");
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ldns: core API; calls ldns_version(), ldns_dname_new_frm_str(), ldns_dname_label_count(), ldns_rdf_deep_free().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <ldns/ldns.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Version string must be present — catches a broken .so / missing symbol. */
    const char *v = ldns_version();
    assert(v != NULL);
    assert(strlen(v) > 0);

    ldns_rdf *dname = ldns_dname_new_frm_str("example.com.");
    assert(dname != NULL);

    /* "example.com." has 2 labels (excluding the empty root). */
    assert(ldns_dname_label_count(dname) == 2);

    ldns_rdf_deep_free(dname);

    printf("ldns test OK: %s\n", v);
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for blosc: codec registry; calls blosc_compname_to_compcode(), blosc_compcode_to_compname(), blosc_get_complib_info().
 */

#include <blosc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void verify_codec(const char *name, int expected_code) {
    int code = blosc_compname_to_compcode(name);
    assert(code == expected_code);

    const char *roundtrip_name = NULL;
    int rc = blosc_compcode_to_compname(code, &roundtrip_name);
    assert(rc >= 0);
    assert(roundtrip_name != NULL);
    assert(strcmp(roundtrip_name, name) == 0);

    /* blosc_get_complib_info populates complib + version (caller frees). */
    char *complib = NULL;
    char *version = NULL;
    int info_rc = blosc_get_complib_info((char *)name, &complib, &version);
    assert(info_rc >= 0);
    assert(complib != NULL);
    assert(version != NULL);
    assert(strlen(complib) > 0);
    assert(strlen(version) > 0);
    free(complib);
    free(version);
}

int main(void) {
    verify_codec("blosclz", BLOSC_BLOSCLZ);
    verify_codec("lz4",     BLOSC_LZ4);
    /* snappy is an optional codec; a build without it returns a negative code. */
    if (blosc_compname_to_compcode("snappy") >= 0) {
        verify_codec("snappy", BLOSC_SNAPPY);
    }
    verify_codec("zlib",    BLOSC_ZLIB);
    verify_codec("zstd",    BLOSC_ZSTD);

    /* An unknown codec must return a negative code. */
    int bogus = blosc_compname_to_compcode("definitely_not_a_codec");
    assert(bogus < 0);

    printf("blosc codec-registry test OK\n");
    return 0;
}

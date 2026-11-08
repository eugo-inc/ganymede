/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for curl: core API; calls curl_global_init(), curl_easy_init(), curl_easy_cleanup(), curl_global_cleanup().
 */

#include <curl/curl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);

    CURL *h = curl_easy_init();
    assert(h != NULL);

    curl_easy_cleanup(h);
    curl_global_cleanup();

    /* Version string must contain "libcurl/" (this is part of the library's documented contract). */
    const char *v = curl_version();
    assert(v != NULL);
    assert(strstr(v, "libcurl/") != NULL);

    printf("curl test OK: %s\n", v);
    return 0;
}

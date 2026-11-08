/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for curl: escape; calls curl_global_init(), curl_easy_init(), curl_easy_escape(), curl_easy_unescape().
 */

#include <curl/curl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);

    CURL *h = curl_easy_init();
    assert(h != NULL);

    const char *raw = "hello world/foo?bar=baz&qux";

    /* Encode. The result MUST contain the percent-encoded space (%20)
     * and MUST NOT contain a literal space or ampersand. */
    char *encoded = curl_easy_escape(h, raw, 0);
    assert(encoded != NULL);
    assert(strstr(encoded, "%20") != NULL);
    assert(strchr(encoded, ' ') == NULL);
    assert(strchr(encoded, '&') == NULL);

    int decoded_len = 0;
    char *decoded = curl_easy_unescape(h, encoded, 0, &decoded_len);
    assert(decoded != NULL);
    assert(decoded_len == (int)strlen(raw));
    assert(strcmp(decoded, raw) == 0);

    curl_free(decoded);
    curl_free(encoded);

    /* Sanity: version_info must report something non-empty. */
    curl_version_info_data *vi = curl_version_info(CURLVERSION_NOW);
    assert(vi != NULL);
    assert(vi->version != NULL);
    assert(strlen(vi->version) > 0);

    curl_easy_cleanup(h);
    curl_global_cleanup();

    printf("curl escape/unescape test OK (curl=%s)\n", vi->version);
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for curl: url; calls curl_global_init(), curl_url(), curl_url_set(), curl_url_get().
 */

#include <curl/curl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);

    CURLU *u = curl_url();
    assert(u != NULL);

    /* Parse a representative URL. curl_url_set/get return CURLUcode (CURLUE_OK), not CURLcode. */
    CURLUcode rc = curl_url_set(u, CURLUPART_URL, "https://example.com:8443/a/b?x=1#frag", 0);
    assert(rc == CURLUE_OK);

    /* Pull out individual parts and verify each. */
    char *scheme = NULL;
    rc = curl_url_get(u, CURLUPART_SCHEME, &scheme, 0);
    assert(rc == CURLUE_OK);
    assert(scheme != NULL);
    assert(strcmp(scheme, "https") == 0);
    curl_free(scheme);

    char *host = NULL;
    rc = curl_url_get(u, CURLUPART_HOST, &host, 0);
    assert(rc == CURLUE_OK);
    assert(host != NULL);
    assert(strcmp(host, "example.com") == 0);
    curl_free(host);

    char *port = NULL;
    rc = curl_url_get(u, CURLUPART_PORT, &port, 0);
    assert(rc == CURLUE_OK);
    assert(port != NULL);
    assert(strcmp(port, "8443") == 0);
    curl_free(port);

    char *path = NULL;
    rc = curl_url_get(u, CURLUPART_PATH, &path, 0);
    assert(rc == CURLUE_OK);
    assert(path != NULL);
    assert(strcmp(path, "/a/b") == 0);
    curl_free(path);

    rc = curl_url_set(u, CURLUPART_SCHEME, "http", 0);
    assert(rc == CURLUE_OK);

    char *full = NULL;
    rc = curl_url_get(u, CURLUPART_URL, &full, 0);
    assert(rc == CURLUE_OK);
    assert(full != NULL);
    assert(strncmp(full, "http://", 7) == 0);
    assert(strstr(full, "example.com") != NULL);
    curl_free(full);

    curl_url_cleanup(u);
    curl_global_cleanup();

    printf("curl url-parser test OK\n");
    return 0;
}

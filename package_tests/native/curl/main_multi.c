/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for curl: multi; calls curl_global_init(), curl_multi_init(), curl_multi_setopt(), curl_easy_init().
 */

#include <curl/curl.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);

    CURLM *m = curl_multi_init();
    assert(m != NULL);

    /* CURLMOPT_MAXCONNECTS must be a documented, supported option. */
    CURLMcode mc = curl_multi_setopt(m, CURLMOPT_MAXCONNECTS, (long)8);
    assert(mc == CURLM_OK);

    CURL *easy = curl_easy_init();
    assert(easy != NULL);

    assert(curl_easy_setopt(easy, CURLOPT_URL, "http://127.0.0.1/") == CURLE_OK);

    mc = curl_multi_add_handle(m, easy);
    assert(mc == CURLM_OK);

    mc = curl_multi_remove_handle(m, easy);
    assert(mc == CURLM_OK);

    curl_easy_cleanup(easy);

    mc = curl_multi_cleanup(m);
    assert(mc == CURLM_OK);

    curl_global_cleanup();

    printf("curl multi-interface test OK\n");
    return 0;
}

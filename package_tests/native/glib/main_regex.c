/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for glib: regex; calls g_regex_new(), g_regex_match(), g_match_info_fetch(), g_free().
 */

#include <glib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    GError *err = NULL;

    /* Compile a pattern with one capture group. */
    GRegex *re = g_regex_new("hello, (\\w+)!", (GRegexCompileFlags)0, (GRegexMatchFlags)0, &err);
    assert(err == NULL);
    assert(re != NULL);

    /* Match: positive case. */
    GMatchInfo *mi = NULL;
    gboolean matched = g_regex_match(re, "hello, world!", (GRegexMatchFlags)0, &mi);
    assert(matched == TRUE);
    assert(mi != NULL);

    /* Extract capture group 1. */
    gchar *capture = g_match_info_fetch(mi, 1);
    assert(capture != NULL);
    assert(strcmp(capture, "world") == 0);
    g_free(capture);
    g_match_info_free(mi);

    /* Negative case: ensure the regex actually discriminates. */
    GMatchInfo *mi2 = NULL;
    gboolean nomatch = g_regex_match(re, "goodbye, world!", (GRegexMatchFlags)0, &mi2);
    assert(nomatch == FALSE);
    g_match_info_free(mi2);

    /* Substitution: replace the capture group's surroundings. */
    gchar *replaced = g_regex_replace(
        re, "hello, glib!", -1, 0, "greet \\1", (GRegexMatchFlags)0, &err
    );
    assert(err == NULL);
    assert(replaced != NULL);
    assert(strcmp(replaced, "greet glib") == 0);
    g_free(replaced);

    g_regex_unref(re);

    printf("glib GRegex test OK (pcre2 runtime dep verified)\n");
    return 0;
}

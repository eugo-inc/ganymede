/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for inih: core API; calls ini_parse_string(), ini_parse_string_length().
 */

#define _GNU_SOURCE
#include <ini.h>

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

enum { DEMO_LONG_VALUE = INI_MAX_LINE + 100 };

static char seen[3 * INI_MAX_LINE];

static int record(void *user, const char *section, const char *name, const char *value) {
    (void) user;
    char item[2 * INI_MAX_LINE];
    snprintf(item, sizeof item, "%s.%s=%s;", section, name ? name : "(null)", value ? value : "(null)");
    strncat(seen, item, sizeof seen - strlen(seen) - 1);
    return 1;
}

static int check(const char *what, int rc, int want_rc, const char *want_seen) {
    if (rc == want_rc && strcmp(seen, want_seen) == 0) {
        return 0;
    }
    fprintf(stderr, "inih: %s: rc=%d (want %d), callbacks '%s' (want '%s')\n", what, rc, want_rc, seen, want_seen);
    return 1;
}

int main(void) {
    int failed = 0;

    static const char doc[] = "\xEF\xBB\xBF; comment\n# comment\ntop = 1 ; inline comment\n"
                              "[net]\ntls: tcp\nlist = a\n  b\n[empty]\n";
    seen[0] = '\0';
    failed |= check("defaults", ini_parse_string(doc, record, NULL), 0, ".top=1;net.tls=tcp;net.list=a;net.list=b;");

    seen[0] = '\0';
    failed |= check("name without a value", ini_parse_string("x = 1\nnovalue\n", record, NULL), 2, ".x=1;");

    seen[0] = '\0';
    failed |= check("ini_parse_string_length", ini_parse_string_length("a = 1\nb = 2\n", 6, record, NULL), 0, ".a=1;");

    static char longdoc[DEMO_LONG_VALUE + 64];
    strcpy(longdoc, "k = ");
    memset(longdoc + 4, 'v', DEMO_LONG_VALUE);
    strcpy(longdoc + 4 + DEMO_LONG_VALUE, "\nafter = 2\n");
    static char want_long[INI_MAX_LINE + 64];
    strcpy(want_long, ".k=");
    memset(want_long + 3, 'v', INI_MAX_LINE - 1 - 4);
    strcpy(want_long + 3 + (INI_MAX_LINE - 1 - 4), ";.after=2;");
    seen[0] = '\0';
    failed |= check("line longer than INI_MAX_LINE", ini_parse_string(longdoc, record, NULL), 1, want_long);

    Dl_info info = {0};
    void *symbol = dlsym(RTLD_DEFAULT, "ini_parse_string");
    if (symbol == NULL || dladdr(symbol, &info) == 0 || info.dli_fname == NULL ||
        strstr(info.dli_fname, "libinih.so") == NULL) {
        fprintf(stderr, "inih: ini_parse_string does not come from libinih.so (dladdr: %s)\n",
                info.dli_fname ? info.dli_fname : "unresolved");
        failed = 1;
    }

    if (failed) {
        return 1;
    }
    printf("inih: %s, default parser settings, long-line rule and ini_parse_string_length OK\n", info.dli_fname);
    return 0;
}

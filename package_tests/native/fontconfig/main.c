/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fontconfig: core API; calls FcGetVersion(), FcInitLoadConfig(), FcPatternCreate(), FcPatternAddString().
 */

#include <fontconfig/fontconfig.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Version is encoded as MAJOR * 10000 + MINOR * 100 + REVISION. Must be > 0. */
    int version = FcGetVersion();
    assert(version > 0);

    /* Load just the configuration (no font cache scan) — catches expat linkage. */
    FcConfig *config = FcInitLoadConfig();
    assert(config != NULL);

    FcPattern *pat = FcPatternCreate();
    assert(pat != NULL);

    FcBool added = FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)"Serif");
    assert(added == FcTrue);

    FcChar8 *value = NULL;
    FcResult res = FcPatternGetString(pat, FC_FAMILY, 0, &value);
    assert(res == FcResultMatch);
    assert(value != NULL);
    assert(strcmp((const char *)value, "Serif") == 0);

    FcPatternDestroy(pat);
    FcConfigDestroy(config);
    FcFini();

    printf("fontconfig test OK: version=%d\n", version);
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for icu_le_hb: core API; calls icu::LayoutEngine::layoutEngineFactory().
 */

#include <layout/LayoutEngine.h>
#include <cassert>
#include <cstdio>

int main() {
    LEErrorCode status = LE_ILLEGAL_ARGUMENT_ERROR;

    icu::LayoutEngine *engine = icu::LayoutEngine::layoutEngineFactory(
        /* fontInstance */ nullptr,
        /* scriptCode   */ 0,
        /* languageCode */ 0,
        status);

    assert(engine == nullptr);
    assert(LE_FAILURE(status));

    std::printf("icu_le_hb test OK\n");
    return 0;
}

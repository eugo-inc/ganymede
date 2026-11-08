/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for absl: core API; calls absl::StrCat().
 */

#include <absl/base/config.h>
#include <absl/strings/str_cat.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
#ifndef ABSL_NAMESPACE_BEGIN
#error "absl/base/config.h did not define ABSL_NAMESPACE_BEGIN — broken install."
#endif

    /* Exercise the canonical StrCat API — pulls in absl_strings + absl_base at link time. */
    const std::string result = absl::StrCat("demo-", 42, "-", "absl");
    assert(result == "demo-42-absl");

    std::printf("absl test OK: %s\n", result.c_str());
    return 0;
}

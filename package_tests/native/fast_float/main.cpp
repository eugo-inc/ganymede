/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fast_float: core API; calls fast_float::from_chars().
 */

#include <fast_float/fast_float.h>

#include <cstdio>
#include <cstring>
#include <system_error>

int main() {
    const char *text = "1.5 trailing";
    double d = 0.0;
    auto r = fast_float::from_chars(text, text + std::strlen(text), d);
    if (r.ec != std::errc() || d != 1.5 || r.ptr != text + 3) {
        std::printf("FAIL: double \"%s\" -> ec=%d value=%.17g consumed=%td\n", text, static_cast<int>(r.ec), d, r.ptr - text);
        return 1;
    }

    const char *text_f = "0.25";
    float f = 0.0f;
    auto rf = fast_float::from_chars(text_f, text_f + std::strlen(text_f), f);
    if (rf.ec != std::errc() || f != 0.25f || rf.ptr != text_f + 4) {
        std::printf("FAIL: float \"%s\"\n", text_f);
        return 1;
    }

    const char *junk = "abc";
    double untouched = 7.0;
    auto rj = fast_float::from_chars(junk, junk + 3, untouched);
    if (rj.ec != std::errc::invalid_argument || untouched != 7.0 || rj.ptr != junk) {
        std::printf("FAIL: \"abc\" was not rejected\n");
        return 1;
    }

    std::printf("fast_float test OK: %d.%d.%d\n", FASTFLOAT_VERSION_MAJOR, FASTFLOAT_VERSION_MINOR, FASTFLOAT_VERSION_PATCH);
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for double_conversion: parsing flags; compiles against <double-conversion/double-conversion.h>.
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <double-conversion/double-conversion.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

int main() {
    using double_conversion::StringToDoubleConverter;

    /* 1. Default (NO_FLAGS) rejects leading whitespace -> NaN sentinel returned. */
    {
        StringToDoubleConverter strict(
            StringToDoubleConverter::NO_FLAGS, 0.0, /*junk_string_value*/ std::nan(""),
            "Infinity", "NaN");
        const char input[] = "  42.0";
        int processed = 0;
        double v = strict.StringToDouble(input, static_cast<int>(std::strlen(input)), &processed);
        assert(std::isnan(v));
        assert(processed == 0);
    }

    /* 2. ALLOW_LEADING_SPACES accepts the same input. */
    {
        StringToDoubleConverter lenient(
            StringToDoubleConverter::ALLOW_LEADING_SPACES, 0.0, std::nan(""),
            "Infinity", "NaN");
        const char input[] = "  42.0";
        int processed = 0;
        double v = lenient.StringToDouble(input, static_cast<int>(std::strlen(input)), &processed);
        assert(v == 42.0);
        assert(processed == static_cast<int>(std::strlen(input)));
    }

    /* 3. ALLOW_TRAILING_JUNK stops parsing at the first non-numeric char without erroring. */
    {
        StringToDoubleConverter lenient(
            StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, std::nan(""),
            "Infinity", "NaN");
        const char input[] = "12.5xyz";
        int processed = 0;
        double v = lenient.StringToDouble(input, static_cast<int>(std::strlen(input)), &processed);
        assert(v == 12.5);
        assert(processed == 4); /* consumed "12.5" */
    }

    /* 4. ALLOW_HEX parses 0xff -> 255. */
    {
        StringToDoubleConverter hex(
            StringToDoubleConverter::ALLOW_HEX, 0.0, std::nan(""),
            "Infinity", "NaN");
        const char input[] = "0xff";
        int processed = 0;
        double v = hex.StringToDouble(input, static_cast<int>(std::strlen(input)), &processed);
        assert(v == 255.0);
        assert(processed == 4);
    }

    /* 5. Infinity / NaN literals parse to IEEE inf / nan. */
    {
        StringToDoubleConverter parser(
            StringToDoubleConverter::NO_FLAGS, 0.0, std::nan(""),
            "Infinity", "NaN");
        int processed = 0;

        double inf = parser.StringToDouble("Infinity", 8, &processed);
        assert(std::isinf(inf));
        assert(inf > 0.0);
        assert(processed == 8);

        double n = parser.StringToDouble("NaN", 3, &processed);
        assert(std::isnan(n));
        assert(processed == 3);
    }

    std::printf("double_conversion parsing-flags test OK\n");
    return 0;
}

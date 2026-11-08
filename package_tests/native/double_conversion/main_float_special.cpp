/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for double_conversion: float special; calls DoubleToStringConverter::EcmaScriptConverter(), infinity().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <double-conversion/double-conversion.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>

int main() {
    using double_conversion::DoubleToStringConverter;
    using double_conversion::StringToDoubleConverter;
    using double_conversion::StringBuilder;

    /* (a) StringToFloat: parse "3.14" into a 32-bit float and confirm it rounds to the
     * IEEE-754 single-precision representation of 3.14 (not the double). */
    {
        StringToDoubleConverter parser(
            StringToDoubleConverter::NO_FLAGS, 0.0f, std::nanf(""),
            "Infinity", "NaN");
        const char input[] = "3.14";
        int processed = 0;
        float v = parser.StringToFloat(input, static_cast<int>(std::strlen(input)), &processed);
        assert(processed == 4);
        assert(v == 3.14f);
    }

    /* (a-cont.) StringToFloat preserves "Infinity" sentinel for the float path. */
    {
        StringToDoubleConverter parser(
            StringToDoubleConverter::NO_FLAGS, 0.0f, std::nanf(""),
            "Infinity", "NaN");
        int processed = 0;
        float v = parser.StringToFloat("Infinity", 8, &processed);
        assert(std::isinf(v));
        assert(v > 0.0f);
        assert(processed == 8);
    }

    /* (b) ToShortest of +Infinity prints the infinity symbol. */
    {
        const DoubleToStringConverter& conv = DoubleToStringConverter::EcmaScriptConverter();
        char buf[64];
        StringBuilder builder(buf, sizeof(buf));
        double inf = std::numeric_limits<double>::infinity();
        assert(conv.ToShortest(inf, &builder));
        const char* out = builder.Finalize();
        assert(out != nullptr);
        assert(std::strcmp(out, "Infinity") == 0);
    }

    /* (b-cont.) ToShortest of -Infinity is prefixed with '-'. */
    {
        const DoubleToStringConverter& conv = DoubleToStringConverter::EcmaScriptConverter();
        char buf[64];
        StringBuilder builder(buf, sizeof(buf));
        double ninf = -std::numeric_limits<double>::infinity();
        assert(conv.ToShortest(ninf, &builder));
        const char* out = builder.Finalize();
        assert(out != nullptr);
        assert(std::strcmp(out, "-Infinity") == 0);
    }

    /* (b-cont.) ToShortest of NaN prints the NaN symbol. */
    {
        const DoubleToStringConverter& conv = DoubleToStringConverter::EcmaScriptConverter();
        char buf[64];
        StringBuilder builder(buf, sizeof(buf));
        double n = std::nan("");
        assert(conv.ToShortest(n, &builder));
        const char* out = builder.Finalize();
        assert(out != nullptr);
        assert(std::strcmp(out, "NaN") == 0);
    }

    std::printf("double_conversion float / special-value test OK\n");
    return 0;
}

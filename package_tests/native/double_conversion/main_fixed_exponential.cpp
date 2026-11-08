/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for double_conversion: fixed exponential; compiles against <double-conversion/double-conversion.h>.
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <double-conversion/double-conversion.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    using double_conversion::DoubleToStringConverter;
    using double_conversion::StringBuilder;

    DoubleToStringConverter converter(
        DoubleToStringConverter::NO_FLAGS,
        "Infinity",
        "NaN",
        'e',
        /* decimal_in_shortest_low */ -6,
        /* decimal_in_shortest_high */ 21,
        /* max_leading_zeroes_in_precision_mode */ 6,
        /* max_trailing_zeroes_in_precision_mode */ 0);

    /* ToFixed: 3.14159 with 2 decimal digits -> "3.14" */
    {
        char buf[64];
        StringBuilder builder(buf, sizeof(buf));
        assert(converter.ToFixed(3.14159, 2, &builder));
        const char* out = builder.Finalize();
        assert(out != nullptr);
        assert(std::strcmp(out, "3.14") == 0);
    }

    /* ToFixed: 0.0001 with 6 decimal digits -> "0.000100" */
    {
        char buf[64];
        StringBuilder builder(buf, sizeof(buf));
        assert(converter.ToFixed(0.0001, 6, &builder));
        const char* out = builder.Finalize();
        assert(out != nullptr);
        assert(std::strcmp(out, "0.000100") == 0);
    }

    /* ToExponential: 12345.6789 with 4 fractional digits -> "1.2346e4" */
    {
        char buf[64];
        StringBuilder builder(buf, sizeof(buf));
        assert(converter.ToExponential(12345.6789, 4, &builder));
        const char* out = builder.Finalize();
        assert(out != nullptr);
        assert(std::strcmp(out, "1.2346e4") == 0);
    }

    /* ToPrecision: 0.000123456 with 3 significant digits -> "0.000123" */
    {
        char buf[64];
        StringBuilder builder(buf, sizeof(buf));
        assert(converter.ToPrecision(0.000123456, 3, &builder));
        const char* out = builder.Finalize();
        assert(out != nullptr);
        assert(std::strcmp(out, "0.000123") == 0);
    }

    std::printf("double_conversion fixed/exponential/precision test OK\n");
    return 0;
}

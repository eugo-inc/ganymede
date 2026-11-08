/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for double_conversion: core API; calls DoubleToStringConverter::EcmaScriptConverter().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <double-conversion/double-conversion.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    using double_conversion::DoubleToStringConverter;
    using double_conversion::StringToDoubleConverter;
    using double_conversion::StringBuilder;

    char buffer[64];
    StringBuilder builder(buffer, sizeof(buffer));
    const DoubleToStringConverter& converter = DoubleToStringConverter::EcmaScriptConverter();
    assert(converter.ToShortest(3.14, &builder));
    const char* formatted = builder.Finalize();
    assert(formatted != nullptr);
    assert(std::strcmp(formatted, "3.14") == 0);

    StringToDoubleConverter parser(
        StringToDoubleConverter::NO_FLAGS, 0.0, 0.0, "Infinity", "NaN");
    int processed = 0;
    double parsed = parser.StringToDouble(formatted, static_cast<int>(std::strlen(formatted)), &processed);
    assert(processed == 4);
    assert(parsed == 3.14);

    std::printf("double_conversion test OK: %s -> %.17g\n", formatted, parsed);
    return 0;
}

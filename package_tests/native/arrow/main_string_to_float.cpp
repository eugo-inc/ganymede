/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for arrow: string to float; calls arrow::internal::StringToFloat(), arrow::Scalar::Parse(), max(), denorm_min().
 */

#include <arrow/api.h>
#include <arrow/util/float16.h>
#include <arrow/util/value_parsing.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>

namespace {

int failures = 0;

enum class Want { kValue, kPosInf, kNegInf, kNaN, kNegZero };

template <typename T>
bool matches(T got, Want want, double value) {
    switch (want) {
        case Want::kPosInf: return std::isinf(got) && got > 0;
        case Want::kNegInf: return std::isinf(got) && got < 0;
        case Want::kNaN: return std::isnan(got);
        case Want::kNegZero: return got == 0 && std::signbit(got);
        case Want::kValue: return got == static_cast<T>(value) && !std::signbit(got) == !std::signbit(value);
    }
    return false;
}

template <typename T>
void expect(const char *text, char decimal_point, bool accepted, Want want = Want::kValue, double value = 0.0) {
    T got = static_cast<T>(-12345);
    const bool ok = arrow::internal::StringToFloat(text, std::strlen(text), decimal_point, &got);
    if (ok != accepted || (ok && !matches(got, want, value))) {
        std::printf("FAIL: StringToFloat<%s>(\"%s\", '%c'): accepted=%d value=%.17g (want accepted=%d)\n",
                    sizeof(T) == 4 ? "float" : "double", text, decimal_point, ok, static_cast<double>(got), accepted);
        ++failures;
    }
}

void expect_float16(const char *text, char decimal_point, bool accepted, float value = 0.0f) {
    arrow::util::Float16 got;
    const bool ok = arrow::internal::StringToFloat(text, std::strlen(text), decimal_point, &got);
    if (ok != accepted || (ok && got.ToFloat() != value)) {
        std::printf("FAIL: StringToFloat<Float16>(\"%s\", '%c'): accepted=%d value=%g (want accepted=%d value=%g)\n", text,
                    decimal_point, ok, static_cast<double>(got.ToFloat()), accepted, static_cast<double>(value));
        ++failures;
    }
}

void expect_scalar(const std::shared_ptr<arrow::DataType> &type, const char *text, bool accepted, double value = 0.0,
                   bool want_inf = false) {
    const auto parsed = arrow::Scalar::Parse(type, text);
    bool good = parsed.ok() == accepted;
    double got = 0.0;
    if (good && accepted) {
        const auto &scalar = *parsed;
        got = type->id() == arrow::Type::DOUBLE ? std::static_pointer_cast<arrow::DoubleScalar>(scalar)->value
                                                : std::static_pointer_cast<arrow::FloatScalar>(scalar)->value;
        good = want_inf ? (std::isinf(got) && got > 0) : got == value;
    }
    if (!good) {
        std::printf("FAIL: Scalar::Parse(%s, \"%s\"): ok=%d value=%.17g status=%s\n", type->ToString().c_str(), text,
                    parsed.ok(), got, parsed.status().ToString().c_str());
        ++failures;
    }
}

}  // namespace

int main() {
    // double
    expect<double>("1.5", '.', true, Want::kValue, 1.5);
    expect<double>("+1.5", '.', true, Want::kValue, 1.5);  // leading '+': FASTFLOAT_ALLOWS_LEADING_PLUS
    expect<double>("1,5", ',', true, Want::kValue, 1.5);   // ConvertOptions::decimal_point = ','
    expect<double>("1,5", '.', false);                      // ',' is not the decimal point here
    expect<double>("-0.25e2", '.', true, Want::kValue, -25.0);
    expect<double>("-0.0", '.', true, Want::kNegZero);
    expect<double>("0.1", '.', true, Want::kValue, 0.1);  // correctly rounded
    expect<double>("1.7976931348623157e308", '.', true, Want::kValue, std::numeric_limits<double>::max());
    expect<double>("4.9406564584124654e-324", '.', true, Want::kValue, std::numeric_limits<double>::denorm_min());
    expect<double>("1e400", '.', true, Want::kPosInf);    // out of range, accepted as +inf
    expect<double>("-1e400", '.', true, Want::kNegInf);
    expect<double>("1e-400", '.', true, Want::kValue, 0.0);  // underflow, accepted as 0
    expect<double>("nan", '.', true, Want::kNaN);
    expect<double>("-inf", '.', true, Want::kNegInf);
    expect<double>("1.5x", '.', false);   // trailing text
    expect<double>(" 1.5", '.', false);   // leading space
    expect<double>("0x1p3", '.', false);  // hex is not "general" format
    expect<double>("", '.', false);
    expect<double>("+", '.', false);
    // float
    expect<float>("+0.5", '.', true, Want::kValue, 0.5);
    expect<float>("2,25", ',', true, Want::kValue, 2.25);
    expect<float>("3.4028235e38", '.', true, Want::kValue, static_cast<double>(std::numeric_limits<float>::max()));
    expect<float>("1e60", '.', true, Want::kPosInf);  // fits a double, not a float: accepted as +inf
    // Float16, parsed as float then narrowed
    expect_float16("1.5", '.', true, 1.5f);
    expect_float16("+2,5", ',', true, 2.5f);
    expect_float16("2.5y", '.', false);
    // the public route: Scalar::Parse -> ParseValue<DoubleType/FloatType> -> StringToFloat, '.' as decimal point
    expect_scalar(arrow::float64(), "+2.5", true, 2.5);
    expect_scalar(arrow::float64(), "2.5x", false);
    expect_scalar(arrow::float32(), "1e60", true, 0.0, true);

    if (failures != 0) {
        std::printf("%d case(s) failed\n", failures);
        return 1;
    }
    std::printf("arrow string-to-float OK (%s)\n", arrow::GetBuildInfo().version_string.c_str());
    return 0;
}

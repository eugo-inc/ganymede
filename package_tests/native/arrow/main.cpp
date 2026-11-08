/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for arrow: core API; calls arrow::GetBuildInfo().
 */

#include <arrow/api.h>
#include <arrow/util/config.h>
#include <cassert>
#include <cstdio>

int main() {
    arrow::Int64Builder builder;
    assert(builder.Append(1).ok());
    assert(builder.Append(2).ok());
    assert(builder.Append(3).ok());

    std::shared_ptr<arrow::Array> array;
    assert(builder.Finish(&array).ok());
    assert(array != nullptr);
    assert(array->length() == 3);
    assert(array->type_id() == arrow::Type::INT64);

    auto int64_array = std::static_pointer_cast<arrow::Int64Array>(array);
    assert(int64_array->Value(0) == 1);
    assert(int64_array->Value(1) == 2);
    assert(int64_array->Value(2) == 3);

    const arrow::BuildInfo &info = arrow::GetBuildInfo();
    assert(!info.version_string.empty());

    std::printf("arrow test OK: %s\n", info.version_string.c_str());
    return 0;
}

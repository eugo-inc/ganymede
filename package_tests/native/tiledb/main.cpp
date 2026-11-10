/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for tiledb: core API; calls tiledb::Dimension::create(), tiledb::Filter(), tiledb::Attribute::create(), tiledb::Array::create().
 */

#include <tiledb/tiledb>

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

int main() {
    const std::string uri = "/tmp/demo_tiledb_dense";
    std::filesystem::remove_all(uri);
    tiledb::Context ctx;
    tiledb::Domain domain(ctx);
    domain.add_dimension(tiledb::Dimension::create<int32_t>(ctx, "rows", {{1, 4}}, 2))
        .add_dimension(tiledb::Dimension::create<int32_t>(ctx, "cols", {{1, 4}}, 2));
    tiledb::ArraySchema schema(ctx, TILEDB_DENSE);
    schema.set_domain(domain).set_order({{TILEDB_ROW_MAJOR, TILEDB_ROW_MAJOR}});
    tiledb::FilterList filters(ctx);
    filters.add_filter(tiledb::Filter(ctx, TILEDB_FILTER_ZSTD)).add_filter(tiledb::Filter(ctx, TILEDB_FILTER_LZ4));
    schema.add_attribute(tiledb::Attribute::create<int32_t>(ctx, "a").set_filter_list(filters));
    tiledb::Array::create(uri, schema);

    std::vector<int32_t> data(16);
    for (int i = 0; i < 16; ++i) data[i] = i * i - 7;
    {
        tiledb::Array array(ctx, uri, TILEDB_WRITE);
        tiledb::Query query(ctx, array);
        query.set_layout(TILEDB_ROW_MAJOR).set_data_buffer("a", data);
        query.submit();
        array.close();
    }
    std::vector<int32_t> slice(4);
    {
        tiledb::Array array(ctx, uri, TILEDB_READ);
        tiledb::Subarray subarray(ctx, array);
        subarray.add_range(0, 2, 3).add_range(1, 2, 3);
        tiledb::Query query(ctx, array, TILEDB_READ);
        query.set_subarray(subarray).set_layout(TILEDB_ROW_MAJOR).set_data_buffer("a", slice);
        query.submit();
        array.close();
    }
    // rows 2-3, cols 2-3 of the row-major 4x4 grid: indices 5, 6, 9, 10.
    const std::vector<int32_t> want = {5 * 5 - 7, 6 * 6 - 7, 9 * 9 - 7, 10 * 10 - 7};
    const auto [major, minor, patch] = tiledb::version();
    std::printf("TILEDB-CHECK: slice %d %d %d %d (want %d %d %d %d), version %d.%d.%d\n", slice[0], slice[1], slice[2],
                slice[3], want[0], want[1], want[2], want[3], major, minor, patch);
    std::filesystem::remove_all(uri);
    if (slice != want) {
        std::printf("TILEDB-CHECK: FAIL\n");
        return 1;
    }
    std::printf("TILEDB-CHECK: PASS\n");
    return 0;
}

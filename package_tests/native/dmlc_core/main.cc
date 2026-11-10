/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for dmlc_core: core API; calls dmlc::Stream::Create(), Create().
 */

#include <dmlc/data.h>
#include <dmlc/io.h>
#include <dmlc/memory_io.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

#define CHECK_OR_FAIL(condition)                                                           \
    do {                                                                                   \
        if (!(condition)) {                                                                \
            std::fprintf(stderr, "DMLC: FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return 1;                                                                      \
        }                                                                                  \
    } while (0)

int main() {
    std::string buffer;
    {
        dmlc::MemoryStringStream out(&buffer);
        dmlc::Stream *stream = &out;  // the typed Write/Read templates live on dmlc::Stream (the subclass hides them)
        std::vector<float> values = {1.5f, -2.0f, 3.25f};
        stream->Write(values);
        stream->Write(std::string("demo"));
    }
    {
        dmlc::MemoryStringStream in(&buffer);
        dmlc::Stream *stream = &in;
        std::vector<float> values;
        std::string text;
        CHECK_OR_FAIL(stream->Read(&values));
        CHECK_OR_FAIL(stream->Read(&text));
        CHECK_OR_FAIL(values.size() == 3 && values[2] == 3.25f);
        CHECK_OR_FAIL(text == "demo");
    }

    // 2. A libsvm file through dmlc-core's file system and parser.
    char path[] = "/tmp/demo_dmlc_probe_XXXXXX";
    int fd = mkstemp(path);
    CHECK_OR_FAIL(fd >= 0);
    close(fd);
    {
        std::unique_ptr<dmlc::Stream> file(dmlc::Stream::Create(path, "w"));
        const char *rows = "1 0:1.5 3:2\n0 1:-1\n1 2:4 4:0.5\n";
        file->Write(rows, std::string(rows).size());
    }
    std::unique_ptr<dmlc::Parser<uint32_t>> parser(dmlc::Parser<uint32_t>::Create(path, 0, 1, "libsvm"));
    CHECK_OR_FAIL(parser != nullptr);
    size_t rows_seen = 0;
    size_t entries_seen = 0;
    float label_sum = 0.0f;
    float value_sum = 0.0f;
    parser->BeforeFirst();
    while (parser->Next()) {
        const dmlc::RowBlock<uint32_t> &block = parser->Value();
        for (size_t i = 0; i < block.size; ++i) {
            dmlc::Row<uint32_t> row = block[i];
            label_sum += row.get_label();
            for (size_t j = 0; j < row.length; ++j) {
                value_sum += row.get_value(j);
                ++entries_seen;
            }
            ++rows_seen;
        }
    }
    std::remove(path);
    CHECK_OR_FAIL(rows_seen == 3);
    CHECK_OR_FAIL(entries_seen == 5);
    CHECK_OR_FAIL(label_sum == 2.0f);
    CHECK_OR_FAIL(value_sum == 7.0f);

    std::printf("DMLC: PASS stream round trip + libsvm parse (%zu rows, %zu entries)\n", rows_seen, entries_seen);
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for flatbuffers: verifier; calls flatbuffers::GetRoot().
 */

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/verifier.h>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <vector>

int main() {
    flatbuffers::FlatBufferBuilder builder(1024);
    const char *payload = "demo-flatbuffers-verifier";
    auto str_off = builder.CreateString(payload);
    builder.Finish(str_off);

    const uint8_t *buf = builder.GetBufferPointer();
    size_t size = builder.GetSize();
    assert(buf != nullptr);
    assert(size > 0);

    /* A Verifier over the valid bytes must accept the String root. */
    {
        flatbuffers::Verifier verifier(buf, size);
        const flatbuffers::String *root =
            flatbuffers::GetRoot<flatbuffers::String>(buf);
        bool ok = verifier.VerifyString(root);
        assert(ok && "Verifier rejected a well-formed string buffer");
    }

    {
        /* Truncate to a length too small to contain the root offset header. */
        size_t truncated_size = 2;
        assert(truncated_size < size);
        flatbuffers::Verifier verifier(buf, truncated_size);
        /* Verify<T>(elem) takes a byte OFFSET into the buffer (not a pointer)
         * and checks there are sizeof(T) aligned bytes readable at that offset.
         * At offset 0 with only 2 bytes available there isn't room for a 4-byte
         * uoffset_t, so this must fail. */
        bool ok = verifier.Verify<flatbuffers::uoffset_t>(size_t(0));
        assert(!ok && "Verifier accepted a truncated buffer");
    }

    /* A Verifier over a buffer with corrupted root-offset bytes must REJECT. */
    {
        std::vector<uint8_t> corrupted(buf, buf + size);
        /* Overwrite the first 4 bytes (root offset) with a value pointing past
         * the buffer end — any sane Verifier rejects this. */
        corrupted[0] = 0xff;
        corrupted[1] = 0xff;
        corrupted[2] = 0xff;
        corrupted[3] = 0x7f;
        flatbuffers::Verifier verifier(corrupted.data(), corrupted.size());
        const flatbuffers::String *root =
            flatbuffers::GetRoot<flatbuffers::String>(corrupted.data());
        bool ok = verifier.VerifyString(root);
        assert(!ok && "Verifier accepted a buffer with a corrupted root offset");
    }

    std::printf("flatbuffers verifier test OK\n");
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for sentencepiece: core API; calls sentencepiece::SentencePieceProcessor::GetPieceSize().
 */

#include <sentencepiece_processor.h>
#include <cstdio>

int main() {
    /* Construct the canonical processor — pulls in libsentencepiece. */
    sentencepiece::SentencePieceProcessor sp;

    /* With no model loaded, the vocabulary is empty: GetPieceSize() returns 0 (not a crash). */
    const int pieces = sp.GetPieceSize();
    if (pieces != 0) {
        std::fprintf(stderr, "GetPieceSize() on an unloaded processor returned %d, expected 0\n", pieces);
        return 1;
    }

    std::printf("sentencepiece test OK\n");
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for flac: cxx roundtrip; compiles against <FLAC++/decoder.h>, <FLAC++/encoder.h>.
 */

#include <FLAC++/decoder.h>
#include <FLAC++/encoder.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {

constexpr uint32_t kSampleCount = 4096;

class MemoryEncoder : public FLAC::Encoder::Stream {
  public:
    std::vector<FLAC__byte> bytes;

  protected:
    ::FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t count, uint32_t, uint32_t) override {
        bytes.insert(bytes.end(), buffer, buffer + count);
        return ::FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
    }
};

class MemoryDecoder : public FLAC::Decoder::Stream {
  public:
    explicit MemoryDecoder(const std::vector<FLAC__byte> &input) : input_(input) {}
    std::vector<FLAC__int32> samples;
    bool failed = false;

  protected:
    ::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t *count) override {
        const size_t left = input_.size() - offset_;
        if (left == 0) {
            *count = 0;
            return ::FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        *count = *count < left ? *count : left;
        std::memcpy(buffer, input_.data() + offset_, *count);
        offset_ += *count;
        return ::FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }

    ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32 *const buffer[]) override {
        samples.insert(samples.end(), buffer[0], buffer[0] + frame->header.blocksize);
        return ::FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }

    void error_callback(::FLAC__StreamDecoderErrorStatus) override { failed = true; }

  private:
    const std::vector<FLAC__byte> &input_;
    size_t offset_ = 0;
};

}  // namespace

int main() {
    std::vector<FLAC__int32> pcm(kSampleCount);
    for (uint32_t i = 0; i < kSampleCount; ++i) {
        pcm[i] = static_cast<FLAC__int32>((i * 37u) % 2000u) - 1000;  // a ramp; no libm needed
    }

    MemoryEncoder encoder;
    if (!encoder.is_valid() || !encoder.set_channels(1) || !encoder.set_bits_per_sample(16) || !encoder.set_sample_rate(16000) ||
        !encoder.set_compression_level(5) || !encoder.set_total_samples_estimate(kSampleCount)) {
        std::fprintf(stderr, "configuring the C++ encoder failed\n");
        return 1;
    }
    if (encoder.init() != ::FLAC__STREAM_ENCODER_INIT_STATUS_OK || !encoder.process_interleaved(pcm.data(), kSampleCount) ||
        !encoder.finish()) {
        std::fprintf(stderr, "the C++ encoder failed: %s\n", encoder.get_state().as_cstring());
        return 1;
    }
    if (encoder.bytes.size() < 4 || std::memcmp(encoder.bytes.data(), "fLaC", 4) != 0) {
        std::fprintf(stderr, "the encoded stream (%zu bytes) does not start with the fLaC marker\n", encoder.bytes.size());
        return 1;
    }

    MemoryDecoder decoder(encoder.bytes);
    if (decoder.init() != ::FLAC__STREAM_DECODER_INIT_STATUS_OK || !decoder.process_until_end_of_stream() || decoder.failed) {
        std::fprintf(stderr, "the C++ decoder failed: %s\n", decoder.get_state().as_cstring());
        return 1;
    }
    decoder.finish();
    if (decoder.samples != pcm) {
        std::fprintf(stderr, "decoded %zu samples that differ from the %u encoded ones\n", decoder.samples.size(), kSampleCount);
        return 1;
    }

    std::printf("libFLAC++ %s: %u samples -> %zu bytes -> the same %zu samples\n", FLAC__VERSION_STRING, kSampleCount,
                encoder.bytes.size(), decoder.samples.size());
    return 0;
}

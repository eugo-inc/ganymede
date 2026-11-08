/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for jpeg_xl: encoder settings; calls JxlEncoderCreate(), JxlThreadParallelRunnerCreate(), JxlThreadParallelRunnerDefaultNumWorkerThreads(), JxlEncoderSetParallelRunner().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jxl/encode.h>
#include <jxl/thread_parallel_runner.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    JxlEncoder *enc = JxlEncoderCreate(NULL);
    assert(enc != NULL);

    void *runner = JxlThreadParallelRunnerCreate(
        NULL, JxlThreadParallelRunnerDefaultNumWorkerThreads());
    assert(runner != NULL);
    JxlEncoderStatus rc_runner = JxlEncoderSetParallelRunner(
        enc, JxlThreadParallelRunner, runner);
    assert(rc_runner == JXL_ENC_SUCCESS);

    /* Allocate frame settings tied to the encoder. The returned pointer is
     * owned by the encoder — no separate destroy. */
    JxlEncoderFrameSettings *settings = JxlEncoderFrameSettingsCreate(enc, NULL);
    assert(settings != NULL);

    /* Effort 7 is the libjxl default for the cjxl CLI and a valid mid-range
     * value (range is 1..10). Setting it must succeed. */
    JxlEncoderStatus rc_effort = JxlEncoderFrameSettingsSetOption(
        settings, JXL_ENC_FRAME_SETTING_EFFORT, 7);
    assert(rc_effort == JXL_ENC_SUCCESS);

    /* Decoding-speed option — a distinct option key, must also succeed. */
    JxlEncoderStatus rc_speed = JxlEncoderFrameSettingsSetOption(
        settings, JXL_ENC_FRAME_SETTING_DECODING_SPEED, 0);
    assert(rc_speed == JXL_ENC_SUCCESS);

    JxlEncoderDestroy(enc);
    JxlThreadParallelRunnerDestroy(runner);

    printf("jpeg_xl encoder settings test OK\n");
    return 0;
}

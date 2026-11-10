/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cuda_nvcomp: c api; calls nvcompGetProperties(), nvcompBatchedZstdCompressGetMaxOutputChunkSize(), nvcompBatchedDeflateCompressGetMaxOutputChunkSize().
 */

#include <nvcomp.h>
#include <stdio.h>

int main(void) {
    nvcompProperties_t properties = {0};
    nvcompStatus_t status = nvcompGetProperties(&properties);
    if (status != nvcompSuccess) {
        printf("NVCOMP-C-API: FAIL nvcompGetProperties status=%d\n", (int)status);
        return 1;
    }
    if (properties.version != (unsigned)NVCOMP_VER) {
        printf("NVCOMP-C-API: FAIL library version %u, headers say %u\n", properties.version, (unsigned)NVCOMP_VER);
        return 1;
    }
    size_t zstd_max = 0;
    status = nvcompBatchedZstdCompressGetMaxOutputChunkSize(65536, nvcompBatchedZstdCompressDefaultOpts, &zstd_max);
    if (status != nvcompSuccess || zstd_max < 65536) {
        printf("NVCOMP-C-API: FAIL zstd max chunk status=%d size=%zu\n", (int)status, zstd_max);
        return 1;
    }
    size_t deflate_max = 0;
    status = nvcompBatchedDeflateCompressGetMaxOutputChunkSize(65536, nvcompBatchedDeflateCompressDefaultOpts, &deflate_max);
    if (status != nvcompSuccess || deflate_max < 65536) {
        printf("NVCOMP-C-API: FAIL deflate max chunk status=%d size=%zu\n", (int)status, deflate_max);
        return 1;
    }
    printf("NVCOMP-C-API: PASS version=%u cudart=%u zstd_max=%zu deflate_max=%zu\n", properties.version,
           properties.cudart_version, zstd_max, deflate_max);
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libde265: core API; calls de265_get_version(), de265_get_version_number(), de265_new_decoder(), de265_start_worker_threads().
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <libde265/de265.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  const char *const version = de265_get_version();
  if (version == NULL || strcmp(version, LIBDE265_VERSION) != 0) {
    fprintf(stderr, "version mismatch: library '%s', header '%s'\n", version ? version : "(null)", LIBDE265_VERSION);
    return 1;
  }
  if (de265_get_version_number() != LIBDE265_NUMERIC_VERSION) {
    fprintf(stderr, "numeric version mismatch: library 0x%08x, header 0x%08x\n", de265_get_version_number(),
            (unsigned)LIBDE265_NUMERIC_VERSION);
    return 1;
  }

  de265_decoder_context *const decoder = de265_new_decoder();
  if (decoder == NULL) {
    fprintf(stderr, "de265_new_decoder returned NULL\n");
    return 1;
  }
  de265_error error = de265_start_worker_threads(decoder, 2);
  if (!de265_isOK(error)) {
    fprintf(stderr, "de265_start_worker_threads: %s\n", de265_get_error_text(error));
    return 1;
  }

  /* An empty stream: flushing must succeed and yield no picture. */
  error = de265_flush_data(decoder);
  if (!de265_isOK(error)) {
    fprintf(stderr, "de265_flush_data: %s\n", de265_get_error_text(error));
    return 1;
  }
  int more = 1;
  while (more) {
    more = 0;
    error = de265_decode(decoder, &more);
    if (error == DE265_ERROR_WAITING_FOR_INPUT_DATA) {
      break;
    }
  }
  if (de265_get_next_picture(decoder) != NULL) {
    fprintf(stderr, "an empty stream produced a picture\n");
    return 1;
  }

  /* A NAL unit of garbage bytes is rejected, not decoded into a picture. */
  de265_reset(decoder);
  const unsigned char garbage[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  error = de265_push_NAL(decoder, garbage, sizeof garbage, 0, NULL);
  if (!de265_isOK(error)) {
    fprintf(stderr, "de265_push_NAL refused to queue bytes: %s\n", de265_get_error_text(error));
    return 1;
  }
  de265_flush_data(decoder);
  more = 1;
  int decode_calls = 0;
  while (more && decode_calls < 100) {
    more = 0;
    decode_calls++;
    error = de265_decode(decoder, &more);
  }
  if (de265_get_next_picture(decoder) != NULL) {
    fprintf(stderr, "garbage bytes produced a picture\n");
    return 1;
  }
  de265_free_decoder(decoder);

  Dl_info info;
  if (dladdr((void *)de265_new_decoder, &info) == 0 || info.dli_fname == NULL ||
      strncmp(info.dli_fname, "/usr/local/lib64/", strlen("/usr/local/lib64/")) != 0) {
    fprintf(stderr, "libde265 was not loaded from /usr/local/lib64: %s\n", info.dli_fname ? info.dli_fname : "(null)");
    return 1;
  }
  printf("libde265 %s OK (%s)\n", version, info.dli_fname);
  return 0;
}

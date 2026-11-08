/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libgpg_error: b64; calls gpgrt_fopenmem(), gpgrt_b64enc_start(), gpgrt_b64enc_write(), gpgrt_b64enc_finish().
 */

#include <gpg-error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    static const unsigned char plaintext[] =
        "Demo libgpg_error base64 round-trip probe: 0123456789!@#$%^&*()";
    const size_t plaintext_len = sizeof(plaintext) - 1; /* exclude NUL */

    /* ---- Encode into an in-memory estream. ---- */
    gpgrt_stream_t out = gpgrt_fopenmem(0, "w+b");
    assert(out != NULL);

    gpgrt_b64state_t enc = gpgrt_b64enc_start(out, "TEST");
    assert(enc != NULL);

    gpg_err_code_t ec = gpgrt_b64enc_write(enc, plaintext, plaintext_len);
    assert(ec == 0);

    ec = gpgrt_b64enc_finish(enc);
    assert(ec == 0);

    /* Snapshot the encoded bytes from the stream. */
    long encoded_len = gpgrt_ftell(out);
    assert(encoded_len > 0);
    assert((size_t)encoded_len > plaintext_len); /* base64 inflates */

    char *encoded = (char *)malloc((size_t)encoded_len + 1);
    assert(encoded != NULL);
    int rc = gpgrt_fseek(out, 0, SEEK_SET);
    assert(rc == 0);
    size_t got = gpgrt_fread(encoded, 1, (size_t)encoded_len, out);
    assert(got == (size_t)encoded_len);
    encoded[encoded_len] = '\0';
    gpgrt_fclose(out);

    /* ---- Decode back via b64dec. The "TEST" title above produced PEM-style
     *      header/footer lines; gpgrt_b64dec_start("TEST") will strip them. ---- */
    gpgrt_b64state_t dec = gpgrt_b64dec_start("TEST");
    assert(dec != NULL);

    unsigned char *decoded = (unsigned char *)malloc((size_t)encoded_len);
    assert(decoded != NULL);
    memcpy(decoded, encoded, (size_t)encoded_len);
    size_t decoded_len = 0;
    ec = gpgrt_b64dec_proc(dec, decoded, (size_t)encoded_len, &decoded_len);
    /* dec_proc returns 0 OR GPG_ERR_EOF (success-with-end-reached); both are OK. */
    assert(ec == 0 || ec == GPG_ERR_EOF);

    ec = gpgrt_b64dec_finish(dec);
    assert(ec == 0 || ec == GPG_ERR_EOF);

    /* The decoded length must equal the original plaintext length and bytes must match. */
    assert(decoded_len == plaintext_len);
    assert(memcmp(decoded, plaintext, plaintext_len) == 0);

    free(encoded);
    free(decoded);

    printf("libgpg_error base64 round-trip test OK: %zu -> %ld -> %zu bytes\n",
           plaintext_len, encoded_len, decoded_len);
    return 0;
}

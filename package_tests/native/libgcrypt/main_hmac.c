/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libgcrypt: hmac; calls gcry_check_version(), gcry_control(), gcry_mac_open(), gcry_mac_setkey().
 */

#include <gcrypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *v = gcry_check_version(NULL);
    assert(v != NULL);
    gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    static const unsigned char key[20] = {
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    };
    static const unsigned char data[] = { 'H', 'i', ' ', 'T', 'h', 'e', 'r', 'e' };
    static const unsigned char expected[32] = {
        0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
        0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
        0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
        0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
    };

    gcry_mac_hd_t hd;
    gcry_error_t err;
    err = gcry_mac_open(&hd, GCRY_MAC_HMAC_SHA256, 0, NULL);
    assert(err == 0);
    err = gcry_mac_setkey(hd, key, sizeof(key));
    assert(err == 0);

    /* Stream the data in two chunks to exercise the incremental path. */
    err = gcry_mac_write(hd, data, 4);
    assert(err == 0);
    err = gcry_mac_write(hd, data + 4, sizeof(data) - 4);
    assert(err == 0);

    unsigned char mac[32];
    size_t maclen = sizeof(mac);
    err = gcry_mac_read(hd, mac, &maclen);
    assert(err == 0);
    assert(maclen == 32);
    assert(memcmp(mac, expected, sizeof(mac)) == 0);

    /* Verify the verify-side API too. */
    err = gcry_mac_verify(hd, expected, sizeof(expected));
    assert(err == 0);

    gcry_mac_close(hd);
    printf("libgcrypt HMAC test OK\n");
    return 0;
}

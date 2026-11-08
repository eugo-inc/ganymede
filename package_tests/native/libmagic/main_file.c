/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libmagic: file; calls magic_open(), magic_load(), magic_file(), magic_descriptor().
 */

#include <magic.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Aspect: file-path-based detection via magic_file().
 * Distinct from main.c which uses magic_buffer() against an in-memory blob.
 * Writes a tiny file to /tmp and asks libmagic to classify it from disk. */
int main(void) {
    magic_t cookie = magic_open(MAGIC_NONE);
    assert(cookie != NULL);
    int rc = magic_load(cookie, NULL);
    assert(rc == 0);

    /* Write a minimal ASCII text file. */
    char path[] = "/tmp/libmagic_test_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    const char *body = "Hello, libmagic!\nThis is a plain text file.\n";
    ssize_t n = write(fd, body, strlen(body));
    assert(n == (ssize_t)strlen(body));
    close(fd);

    const char *desc = magic_file(cookie, path);
    assert(desc != NULL);
    /* libmagic should describe an ASCII text file. Both modern ("ASCII text")
     * and older ("ASCII English text") wordings contain the substring "text". */
    assert(strstr(desc, "text") != NULL);

    /* Also verify magic_descriptor() — the fd-based variant. */
    int fd2 = open(path, 0 /* O_RDONLY */);
    assert(fd2 >= 0);
    const char *desc2 = magic_descriptor(cookie, fd2);
    assert(desc2 != NULL);
    assert(strstr(desc2, "text") != NULL);
    close(fd2);

    unlink(path);
    magic_close(cookie);
    printf("libmagic file/descriptor test OK\n");
    return 0;
}

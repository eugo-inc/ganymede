/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libattr: copy; calls attr_copy_check_permissions(), attr_copy_action(), attr_copy_file().
 * Works on scratch files in the current directory. Where the file system has no user
 * extended attributes, only the name checks run.
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <attr/error_context.h>
#include <attr/libattr.h>
#include <sys/xattr.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROBE_ATTR "user.demo_libattr_copy"

static int error_calls;

/* error_context hooks: count error reports; quoting returns the name unchanged. */
static void count_error(struct error_context *ctx, const char *fmt, ...) {
    (void)ctx;
    (void)fmt;
    error_calls++;
}
static const char *plain_quote(struct error_context *ctx, const char *name) {
    (void)ctx;
    return name;
}
static void plain_quote_free(struct error_context *ctx, const char *name) {
    (void)ctx;
    (void)name;
}

/* attr_copy_file() copies an attribute when the check callback returns non-zero. */
static int copy_probe_only(const char *name, struct error_context *ctx) {
    (void)ctx;
    return strcmp(name, PROBE_ATTR) == 0;
}
static int copy_none(const char *name, struct error_context *ctx) {
    (void)name;
    (void)ctx;
    return 0;
}

static void make_scratch(char *path) {
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);
}

int main(void) {
    /* ATTR_ACTION_* macros from <attr/libattr.h>. */
    assert(ATTR_ACTION_SKIP != ATTR_ACTION_PERMISSIONS);

    struct error_context ctx;
    ctx.error = count_error;
    ctx.quote = plain_quote;
    ctx.quote_free = plain_quote_free;

    /* An ordinary user attribute matches no xattr.conf rule, so its action is 0 (copy)
     * and attr_copy_check_permissions() accepts it. */
    assert(attr_copy_action(PROBE_ATTR, &ctx) == 0);
    assert(attr_copy_check_permissions(PROBE_ATTR, &ctx) == 1);

    char src[] = "demo_libattr_copy_src_XXXXXX";
    char dst[] = "demo_libattr_copy_dst_XXXXXX";
    char dst_none[] = "demo_libattr_copy_none_XXXXXX";
    make_scratch(src);
    make_scratch(dst);
    make_scratch(dst_none);

    if (setxattr(src, PROBE_ATTR, "v", 1, 0) != 0) {
        assert(errno == ENOTSUP || errno == EOPNOTSUPP);
        unlink(src);
        unlink(dst);
        unlink(dst_none);
        printf("libattr copy probe OK: name checks only; this file system has no user extended attributes\n");
        return 0;
    }

    /* Copy with a callback that selects the probe attribute: it arrives on dst. */
    int rc = attr_copy_file(src, dst, copy_probe_only, &ctx);
    assert(rc == 0);
    assert(error_calls == 0);
    char value[8];
    ssize_t len = getxattr(dst, PROBE_ATTR, value, sizeof(value));
    assert(len == 1 && value[0] == 'v');

    /* Copy with a callback that selects nothing: dst_none stays without it. */
    rc = attr_copy_file(src, dst_none, copy_none, &ctx);
    assert(rc == 0);
    assert(error_calls == 0);
    len = getxattr(dst_none, PROBE_ATTR, value, sizeof(value));
    assert(len == -1 && errno == ENODATA);

    unlink(src);
    unlink(dst);
    unlink(dst_none);
    printf("libattr copy/error_context probe OK: attr_copy_file honours its check callback\n");
    return 0;
}

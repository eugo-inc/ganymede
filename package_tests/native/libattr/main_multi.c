/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libattr: multi; calls attr_multi().
 * Works on a scratch file in the current directory. Where the file system has no user
 * extended attributes, the batch must fail with ENOTSUP and nothing more is checked.
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <attr/attributes.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static int unsupported(int error) {
    return error == ENOTSUP || error == EOPNOTSUPP;
}

int main(void) {
    assert(ATTR_OP_GET != ATTR_OP_SET);
    assert(ATTR_OP_SET != ATTR_OP_REMOVE);
    assert(ATTR_OP_GET != ATTR_OP_REMOVE);
    assert(ATTR_MAX_MULTIOPS > 0);

    char path[] = "demo_libattr_multi_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);

    char name[] = "demo_libattr_probe_a";
    char value_in[] = "v1";
    char value_out[32];
    attr_multiop_t ops[3];
    memset(ops, 0, sizeof(ops));

    /* One batch: set the attribute, read it back, then remove it. The ops run in order. */
    ops[0].am_opcode = ATTR_OP_SET;
    ops[0].am_attrname = name;
    ops[0].am_attrvalue = value_in;
    ops[0].am_length = 2;

    ops[1].am_opcode = ATTR_OP_GET;
    ops[1].am_attrname = name;
    ops[1].am_attrvalue = value_out;
    ops[1].am_length = (int)sizeof(value_out);

    ops[2].am_opcode = ATTR_OP_REMOVE;
    ops[2].am_attrname = name;

    /* Only ATTR_DONTFOLLOW is accepted in the flags argument: anything else is EINVAL. */
    int rc = attr_multi(path, ops, 3, ATTR_ROOT);
    assert(rc == -1 && errno == EINVAL);

    rc = attr_multi(path, ops, 3, 0);
    if (rc != 0) {
        assert(rc == -1 && unsupported(errno));
        unlink(path);
        printf("libattr attr_multi probe OK: this file system has no user extended attributes\n");
        return 0;
    }
    assert(ops[1].am_length == 2);
    assert(memcmp(value_out, "v1", 2) == 0);

    /* The REMOVE op ran last, so the attribute is gone: ENOATTR (ENODATA on Linux). */
    char probe[8];
    int probe_len = (int)sizeof(probe);
    rc = attr_get(path, name, probe, &probe_len, 0);
    assert(rc == -1 && errno == ENOATTR);

    unlink(path);
    printf("libattr attr_multi probe OK: set, get and remove in one batch\n");
    return 0;
}

#pragma GCC diagnostic pop

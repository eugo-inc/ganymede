/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for highway: abort; calls hwy::SetAbortFunc(), hwy::GetAbortFunc(), hwy::SetWarnFunc(), hwy::GetWarnFunc().
 */

#include <hwy/base.h>
#include <cassert>
#include <cstdio>

static void my_abort_handler(const char *, int, const char *) {
}

static void my_warn_handler(const char *, int, const char *) {
}

int main() {
    /* 1. SetAbortFunc returns the prior handler (nullptr if first call). */
    hwy::AbortFunc prev_abort = hwy::SetAbortFunc(&my_abort_handler);
    hwy::AbortFunc &cur_abort = hwy::GetAbortFunc();
    assert(cur_abort == &my_abort_handler);

    hwy::AbortFunc returned = hwy::SetAbortFunc(prev_abort);
    assert(returned == &my_abort_handler);
    /* Getter now reports the original (which was nullptr on a fresh process). */
    hwy::AbortFunc &restored_abort = hwy::GetAbortFunc();
    assert(restored_abort == prev_abort);

    hwy::WarnFunc prev_warn = hwy::SetWarnFunc(&my_warn_handler);
    hwy::WarnFunc &cur_warn = hwy::GetWarnFunc();
    assert(cur_warn == &my_warn_handler);
    hwy::WarnFunc returned_warn = hwy::SetWarnFunc(prev_warn);
    assert(returned_warn == &my_warn_handler);

    std::printf("highway abort/warn handler test OK\n");
    return 0;
}

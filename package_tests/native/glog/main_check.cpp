/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for glog: check; calls google::InitGoogleLogging(), google::ShutdownGoogleLogging().
 */

#include <glog/logging.h>
#include <cassert>
#include <cstdio>

int main(int argc, char *argv[]) {
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = google::GLOG_ERROR;
    google::InitGoogleLogging(argv[0]);

    /* Boolean CHECK. */
    CHECK(1 + 1 == 2) << "basic CHECK must accept true predicate";

    /* Comparison CHECK_* family. */
    int a = 7;
    int b = 7;
    int c = 9;
    CHECK_EQ(a, b) << "CHECK_EQ on equal ints";
    CHECK_NE(a, c) << "CHECK_NE on different ints";
    CHECK_LT(a, c) << "CHECK_LT: 7 < 9";
    CHECK_LE(a, b) << "CHECK_LE: 7 <= 7";
    CHECK_GT(c, a) << "CHECK_GT: 9 > 7";
    CHECK_GE(b, a) << "CHECK_GE: 7 >= 7";

    /* CHECK_NOTNULL must RETURN the pointer (this is the documented behaviour
     * used by glog consumers to chain initialization). */
    int sentinel = 42;
    int *p = &sentinel;
    int *q = CHECK_NOTNULL(p);
    assert(q == p);
    assert(*q == 42);

    /* C-string CHECK. */
    const char *s1 = "demo";
    const char *s2 = "demo";
    CHECK_STREQ(s1, s2) << "CHECK_STREQ on equal C strings";

    google::ShutdownGoogleLogging();
    std::printf("glog CHECK macros test OK\n");
    return 0;
}

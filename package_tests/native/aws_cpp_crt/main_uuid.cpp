/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_cpp_crt: uuid; compiles against <aws/crt/Api.h>, <aws/crt/UUID.h>.
 */

#include <aws/crt/Api.h>
#include <aws/crt/UUID.h>
#include <cassert>
#include <iostream>

int main() {
    Aws::Crt::ApiHandle api;

    Aws::Crt::UUID u1;
    assert(static_cast<bool>(u1));

    Aws::Crt::String s1 = u1.ToString();
    assert(s1.size() == 36);
    assert(s1[8] == '-');
    assert(s1[13] == '-');
    assert(s1[18] == '-');
    assert(s1[23] == '-');

    Aws::Crt::UUID u1_reparsed(s1);
    assert(static_cast<bool>(u1_reparsed));
    assert(u1 == u1_reparsed);
    assert(!(u1 != u1_reparsed));

    /* Two independently-generated UUIDs differ (entropy sanity). */
    Aws::Crt::UUID u2;
    assert(static_cast<bool>(u2));
    assert(u1 != u2);

    std::cout << "aws_cpp_crt UUID test OK: " << s1 << std::endl;
    return 0;
}

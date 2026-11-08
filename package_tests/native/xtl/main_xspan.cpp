/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xtl: xspan; compiles against <xtl/xspan.hpp>.
 */

#include <xtl/xspan.hpp>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

int main() {
    /* Dynamic-extent span over a C array. */
    int data[] = {10, 20, 30, 40, 50};
    xtl::span<int> view(data, 5);
    assert(view.size() == 5);
    assert(view.data() == data);

    /* Indexed access reads the underlying storage. */
    assert(view[0] == 10);
    assert(view[4] == 50);

    /* Mutating through the view changes the underlying array — confirms
       it's a true reference, not a copy. */
    view[2] = 999;
    assert(data[2] == 999);

    std::vector<int> vec = {1, 2, 3, 4, 5, 6};
    xtl::span<int> vview(vec.data(), vec.size());
    assert(vview.size() == 6);
    assert(vview[5] == 6);

    /* subspan: a window into the middle of the source. */
    auto sub = vview.subspan(2, 3);
    assert(sub.size() == 3);
    assert(sub[0] == 3);
    assert(sub[2] == 5);

    std::cout << "xtl xspan test OK" << std::endl;
    return 0;
}

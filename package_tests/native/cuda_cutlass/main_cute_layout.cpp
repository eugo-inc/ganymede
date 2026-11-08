/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cuda_cutlass: cute layout; calls make_layout(), make_shape(), cosize(), layout().
 */

#include <cstdio>
#include <cute/layout.hpp>

int main() {
    using namespace cute;
    auto layout = make_layout(make_shape(Int<4>{}, Int<8>{}), LayoutLeft{});
    static_assert(decltype(size(layout))::value == 32, "cute size");
    static_assert(decltype(cosize(layout))::value == 32, "cute cosize");
    // Column-major: (row, col) -> row + col*4.
    if (layout(1, 2) != 9) { std::fprintf(stderr, "cute layout(1,2) = %d, want 9\n", (int)layout(1, 2)); return 1; }
    std::printf("cute layout algebra OK\n");
    return 0;
}

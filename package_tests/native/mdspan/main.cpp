/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for mdspan: core API; uses Kokkos::mdspan, Kokkos::dextents, Kokkos::layout_stride and Kokkos::submdspan().
 */

#define MDSPAN_USE_PAREN_OPERATOR 1  // enable the m(i, j) call operator; the library leaves it off from C++23 on unless asked
#include <mdspan/mdspan.hpp>

#include <array>
#include <cstddef>
#include <cstdio>

int main() {
    std::array<double, 6> data{0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    using extents_2d = Kokkos::dextents<std::size_t, 2>;
    int failures = 0;

    // Row-major 2x3 view (the default layout): m(i, j) is data[i * 3 + j].
    Kokkos::mdspan<double, extents_2d> matrix(data.data(), 2, 3);
    if (matrix.extent(0) != 2 || matrix.extent(1) != 3 || matrix(1, 2) != 5.0 || matrix(0, 1) != 1.0) {
        ++failures;
    }
#if MDSPAN_USE_BRACKET_OPERATOR
    if (matrix[1, 0] != 3.0) {
        ++failures;
    }
#endif

    // The same numbers read column-major through layout_stride (strides 1 and 3): 3x2, t(i, j) is data[i + 3 * j].
    Kokkos::layout_stride::mapping<extents_2d> column_major(extents_2d(3, 2), std::array<std::size_t, 2>{1, 3});
    Kokkos::mdspan<double, extents_2d, Kokkos::layout_stride> transposed(data.data(), column_major);
    if (transposed(2, 1) != 5.0 || transposed(1, 0) != 1.0 || transposed.stride(1) != 3) {
        ++failures;
    }

    // One row as its own view.
    auto row = Kokkos::submdspan(matrix, 1, Kokkos::full_extent);
    if (row.extent(0) != 3 || row(0) != 3.0 || row(2) != 5.0) {
        ++failures;
    }

    std::printf("mdspan probe: %s (%d failure(s))\n", failures == 0 ? "OK" : "FAILED", failures);
    return failures == 0 ? 0 : 1;
}

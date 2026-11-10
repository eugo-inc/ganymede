/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for spglib: spacegroup; calls spg_get_international(), spg_get_dataset(), spg_get_major_version(), spg_get_minor_version().
 */

#include <spglib.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    double lattice[3][3] = {{2.87, 0, 0}, {0, 2.87, 0}, {0, 0, 2.87}};
    double position[2][3] = {{0, 0, 0}, {0.5, 0.5, 0.5}};
    int types[2] = {26, 26};
    char symbol[11] = {0};
    int number = spg_get_international(symbol, lattice, position, types, 2, 1e-5);
    if (number != 229 || strcmp(symbol, "Im-3m") != 0) {
        fprintf(stderr, "expected Im-3m (229), got '%s' (%d)\n", symbol, number);
        return 1;
    }
    SpglibDataset *dataset = spg_get_dataset(lattice, position, types, 2, 1e-5);
    if (dataset == NULL || dataset->n_operations != 96) {
        /* 48 point operations x 2 lattice translations of the conventional bcc cell given here */
        fprintf(stderr, "unexpected dataset: %p, %d operations\n", (void *)dataset, dataset ? dataset->n_operations : -1);
        return 1;
    }
    printf("spglib %d.%d.%d: %s (%d), %d operations\n", spg_get_major_version(), spg_get_minor_version(),
           spg_get_micro_version(), symbol, number, dataset->n_operations);
    spg_free_dataset(dataset);
    return 0;
}

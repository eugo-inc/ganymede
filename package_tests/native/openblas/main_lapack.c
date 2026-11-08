/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openblas: lapack; calls dgetrf_(), dgesv_().
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Reference LAPACK LP64 prototypes (OpenBLAS exports the unsuffixed names in libopenblas.so). */
extern void dgesv_(const int *n, const int *nrhs, double *a, const int *lda,
                   int *ipiv, double *b, const int *ldb, int *info);
extern void dgetrf_(const int *m, const int *n, double *a, const int *lda, int *ipiv, int *info);

/* Column-major (i,j) — the layout LAPACK expects. */
#define AT(p, n, i, j) ((p)[(size_t)(j) * (size_t)(n) + (size_t)(i)])

static void fill_matrix(double *a, int n) {
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
            AT(a, n, i, j) = (double)(((i * 7919 + j * 104729) % 193) - 96) / 32.0;
    for (int i = 0; i < n; ++i) AT(a, n, i, i) += (double)(n * 4);
}

/* Check 1 — the FACTORISATION. Rebuild P*L*U from dgetrf's output and compare against the original. */
static int check_factorisation(int n) {
    int rc = 1;
    double *a = malloc((size_t)n * n * sizeof(double));
    double *a0 = malloc((size_t)n * n * sizeof(double));
    double *m = malloc((size_t)n * n * sizeof(double));
    int *ipiv = malloc((size_t)n * sizeof(int));
    if (!a || !a0 || !m || !ipiv) { fprintf(stderr, "n=%d: out of memory\n", n); goto done; }

    fill_matrix(a, n);
    memcpy(a0, a, (size_t)n * n * sizeof(double));

    int info = -999;
    dgetrf_(&n, &n, a, &n, ipiv, &info);
    if (info != 0) { fprintf(stderr, "n=%d: dgetrf info=%d (expected 0)\n", n, info); goto done; }

    /* Structural check before any tolerance: LAPACK ipiv is 1-based and every entry must name a row. */
    for (int i = 0; i < n; ++i)
        if (ipiv[i] < 1 || ipiv[i] > n) {
            fprintf(stderr, "n=%d: dgetrf pivot ipiv[%d]=%d out of range [1,%d]\n", n, i, ipiv[i], n);
            goto done;
        }

    /* M = L*U  (L unit-lower, U upper, both packed in `a`). */
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i) {
            double s = 0.0;
            int kmax = (i < j) ? i : j;
            for (int k = 0; k <= kmax; ++k)
                s += ((i == k) ? 1.0 : AT(a, n, i, k)) * AT(a, n, k, j);
            AT(m, n, i, j) = s;
        }
    /* Apply dgetrf's row interchanges in REVERSE to recover P*L*U. */
    for (int i = n - 1; i >= 0; --i) {
        int p = ipiv[i] - 1;
        if (p == i) continue;
        for (int j = 0; j < n; ++j) {
            double t = AT(m, n, i, j);
            AT(m, n, i, j) = AT(m, n, p, j);
            AT(m, n, p, j) = t;
        }
    }

    double err = 0.0;
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i) {
            double e = fabs(AT(m, n, i, j) - AT(a0, n, i, j));
            if (!isfinite(e) || e > err) err = e;
        }
    if (!isfinite(err) || err > 1e-9) {
        fprintf(stderr, "n=%d: dgetrf FACTORS WRONG — max|P L U - A| = %.6e\n", n, err);
        goto done;
    }
    rc = 0;
done:
    free(a); free(a0); free(m); free(ipiv);
    return rc;
}

/* Check 2 — the SOLVE. b = A*x_expected, so dgesv must return x_expected. */
static int check_solve(int n) {
    int rc = 1;
    double *a = malloc((size_t)n * n * sizeof(double));
    double *b = malloc((size_t)n * sizeof(double));
    double *x = malloc((size_t)n * sizeof(double));
    int *ipiv = malloc((size_t)n * sizeof(int));
    if (!a || !b || !x || !ipiv) { fprintf(stderr, "n=%d: out of memory\n", n); goto done; }

    fill_matrix(a, n);
    for (int i = 0; i < n; ++i) x[i] = 1.0 + (double)i / (double)n;
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int j = 0; j < n; ++j) s += AT(a, n, i, j) * x[j];
        b[i] = s;
    }

    int nrhs = 1, info = -999;
    dgesv_(&n, &nrhs, a, &n, ipiv, b, &n, &info);
    if (info != 0) { fprintf(stderr, "n=%d: dgesv info=%d (expected 0)\n", n, info); goto done; }

    double err = 0.0;
    for (int i = 0; i < n; ++i) {
        if (!isfinite(b[i])) {
            fprintf(stderr, "n=%d: dgesv non-finite solution at [%d] = %g\n", n, i, b[i]);
            goto done;
        }
        double e = fabs(b[i] - x[i]);
        if (e > err) err = e;
    }
    if (err > 1e-6) {
        fprintf(stderr, "n=%d: dgesv SOLUTION WRONG — max abs error %.6e\n", n, err);
        goto done;
    }
    printf("n=%-4d ok (factors + solve, max abs error %.3e)\n", n, err);
    rc = 0;
done:
    free(a); free(b); free(x); free(ipiv);
    return rc;
}

static int run_size(int n) {
    if (check_factorisation(n) != 0) return 1;
    return check_solve(n);
}

int main(void) {
    static const int sizes[] = {2, 3, 4, 8, 64, 200, 512};
    int failures = 0;
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i)
        if (run_size(sizes[i]) != 0) ++failures;

    if (failures != 0) {
        fprintf(stderr, "openblas LAPACK smoke: %d/%zu sizes FAILED\n",
                failures, sizeof(sizes) / sizeof(sizes[0]));
        return 1;
    }
    printf("openblas LAPACK smoke: all sizes factored and solved correctly\n");
    return 0;
}

/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for python: core API; calls Py_InitializeEx(), Py_IsInitialized(), Py_GetVersion(), PyRun_SimpleString().
 *
 * Nothing here names a CPython version: the headers come from include/python3.*, the
 * library is whichever libpython3.X the link_with pattern in testspec.json finds, and the
 * run checks that this library is the same major.minor series as those headers.
 */

#include <Python.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Skip site.py to avoid sys.path / venv discovery quirks during a smoke test. */
    Py_NoSiteFlag = 1;
    Py_InitializeEx(0);
    assert(Py_IsInitialized() != 0);

    const char *v = Py_GetVersion();
    assert(v != NULL);

    /* The runtime version must start with the "major.minor." the headers were built for. */
    char series[32];
    int n = snprintf(series, sizeof(series), "%d.%d.", PY_MAJOR_VERSION, PY_MINOR_VERSION);
    assert(n > 0 && (size_t)n < sizeof(series));
    if (strncmp(v, series, (size_t)n) != 0) {
        fprintf(stderr, "headers are for Python %s*, but the linked library reports %s\n", series, v);
        return 1;
    }

    /* Run a tiny program that exercises arithmetic + the C-level eval loop. Non-zero return = failure. */
    int rc = PyRun_SimpleString("assert (lambda x: x * 2)(21) == 42\n");
    assert(rc == 0);

    /* Clean finalize must succeed (returns 0 on success, negative on error). */
    int fin = Py_FinalizeEx();
    assert(fin == 0);

    printf("python test OK: %s\n", v);
    return 0;
}

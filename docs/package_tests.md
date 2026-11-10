---
title: Package smoke tests
description: "Smoke tests for Python packages (one Jupyter notebook per package, run with pytest) and install checks for native libraries, with a standalone runner."
---

# Package smoke tests

The package tests in ganymede are quick checks that an installed package actually works: one
Jupyter notebook per Python package, run with pytest, and declarative install checks for native
libraries, run with a standalone script against any install prefix. They are smoke tests, not a
replacement for each package's own test suite. This page explains both kinds and lists every
package; the notebooks themselves are on GitHub and are not rendered on this site.

## Python package smoke tests

Each notebook in `package_tests/python/` imports its package and exercises the package's main API
with a few small, self-contained examples. A notebook fails if any call raises an error, and many
also assert on their results.

```bash
pip install -r package_tests/python/requirements.txt <package>
pytest package_tests/python/<package>.ipynb
```

`<package>` is the notebook's file name without `.ipynb`, for example `requests` or
`scikit_learn`. Every notebook is collected as one test and runs in a fresh temporary working
directory, in a Jupyter kernel started with the same Python interpreter as pytest.

Cells that need something a typical machine may not have carry a tag such as `requires-gpu`,
`requires-network` or `requires-aws-credentials`, and are skipped unless you enable that
capability with `--with <capability>` (or `--with all`). Other tags allow a cell to fail
(`allow-fail`) or set its time limit (`timeout-<seconds>`). The full list of tags and
capabilities is in
[package_tests/python/README.md](https://github.com/eugo-inc/ganymede/blob/main/package_tests/python/README.md).

## Native library checks

Each folder in `package_tests/native/` holds a `testspec.json` that lists what a correct install
contains, plus small C/C++ programs that use the library for real. `run_native_checks.py` (Python
3.9 or newer, standard library only, Linux) runs these checks against any install prefix:
`/usr/local`, a conda environment, a container image or a Debian-style `/usr`.

| Check | Passes when |
| --- | --- |
| artifacts | every listed file exists under the prefix |
| needed | every shared-library dependency (ELF `NEEDED` entry) of each listed binary resolves |
| pic | every static archive links into a shared object (only when the spec asks for it) |
| smoke | the spec's optional command runs, exits 0 and prints the expected text |
| programs | each test program compiles and links against the prefix, then runs and exits 0 |

```bash
cd package_tests/native
python3 run_native_checks.py --prefix /usr/local -k zstd -v
python3 run_native_checks.py --validate-only    # check every spec without a prefix or compiler
```

The specs were written against one fairly complete install, so on another prefix some rows can
fail for ordinary reasons, such as a package that is not installed or a feature that was built out;
the failure detail says which file, symbol or command was missing. See
[package_tests/native/README.md](https://github.com/eugo-inc/ganymede/blob/main/package_tests/native/README.md)
and [SCHEMA.md](https://github.com/eugo-inc/ganymede/blob/main/package_tests/native/SCHEMA.md).

<!-- package-list -->

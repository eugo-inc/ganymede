# Native package checks

Declarative install checks for native libraries and command-line tools: compression,
cryptography, imaging and media, geospatial, scientific and HPC libraries, CUDA
components and more. Each package folder holds a `testspec.json` that lists what a
correct install contains, plus small C/C++ programs that use the library for real.
`run_native_checks.py` runs those checks against any install prefix: `/usr/local`, a
conda environment, a container image, or a Debian-style `/usr`.

## What gets checked

For every selected package, in order:

| Check | Passes when |
|---|---|
| artifacts | every listed file exists under the prefix; glob rows match at least one file. `lib/` and `lib64/` are interchangeable, a versioned shared library may carry a different version suffix, and a dangling symlink fails. |
| needed | every NEEDED entry in the dynamic section of each ELF artifact (shared libraries and executables) resolves to a library of the same architecture, through RPATH/RUNPATH, `LD_LIBRARY_PATH`, the loader cache and `/etc/ld.so.conf`, the default system directories, or the prefix's own lib directories. |
| pic | (only when the spec sets `static_archives_must_be_pic`) every static archive links into a shared object. When there is no compiler, or the link fails for an unrelated reason, a `readelf -r` scan looks for absolute relocations in code sections: finding one is a failure, and finding none leaves the check SKIPped, because the scan alone cannot prove an archive is position-independent. |
| smoke | the spec's optional command runs from the prefix, exits 0 and prints the expected text. |
| programs | each test program compiles and links against the prefix's headers and libraries, then runs and exits 0. |

A package PASSes when no check failed and at least one ran, is SKIPped when every check
was skipped, and FAILs otherwise. The runner exits with status 1 when any package failed,
2 on a usage error, and 0 otherwise.

## Requirements

- Linux (the checks read ELF files).
- Python 3.9 or newer. Only the standard library is used.
- A C and C++ compiler for the `pic` and `programs` checks (`cc`/`c++` by default).
  Without one the test programs are reported as SKIP, and the `pic` check can only
  fail an archive (through the relocation scan), never pass it.
- `readelf` (binutils or LLVM) is optional; it is used as a fallback.

## Running it

```bash
cd package_tests/native

# everything, against /usr/local
python3 run_native_checks.py

# a conda environment, a few packages, four at a time, with a JSON report
python3 run_native_checks.py --prefix "$CONDA_PREFIX" -k zstd -k 'lib*' -j 4 --json report.json

# the system packages of a Debian or Ubuntu machine
python3 run_native_checks.py --prefix /usr -k zstd -k libexpat

# list the packages a pattern selects
python3 run_native_checks.py --list -k 'aws_*'

# check every spec and the files it names, without a prefix or a compiler
python3 run_native_checks.py --validate-only
```

Main options:

| Option | Default | Meaning |
|---|---|---|
| `--prefix PATH` | `/usr/local` | install prefix to check |
| `--specs-dir PATH` | the folder of the script | where the `<package>/testspec.json` folders live |
| `-k`, `--package NAME` | all | only these packages; shell-style patterns; repeatable |
| `--with CAP` | none | enable checks that need `gpu`, `cuda-driver`, `nvidia-driver`, `rdma`, `toolchain`, or `all` |
| `--cc`, `--cxx` | `$CC`/`$CXX`, else `cc`/`c++` | compilers for test programs and the PIC probe |
| `--cuda-home PATH` | `$CUDA_HOME`, else `/usr/local/cuda` | value of the `{cuda_home}` placeholder |
| `--extra-cflags STR` | empty | extra flags added to every test-program compile |
| `--ld-path` | off | put the prefix lib dirs on `LD_LIBRARY_PATH` for smoke runs and test programs |
| `--timeout-scale X` | `1.0` | multiply every timeout, for slow or busy machines |
| `-j`, `--jobs N` | `1` | packages to check in parallel |
| `--json PATH` | none | write every check and its detail as JSON |
| `--list` | off | print the selected package names and exit |
| `--validate-only` | off | check that every selected spec is valid, its name matches its folder and every file it names exists; needs no prefix or compiler; exits 1 on any problem |
| `-v`, `--verbose` | off | print every check, not only the failing ones |
| `--keep` | off | keep each package's scratch build folder |

The console output is one row per package with `passed/ran` counts per check, followed by
the failing checks with their details:

```
PACKAGE  RESULT  ARTIFACTS  NEEDED  PIC  SMOKE  PROGRAMS  NOTE
zstd     PASS          5/5     2/2    -    1/1       7/7
```

## Reading the results on your own prefix

The specs were written against one fairly complete install, so on another prefix expect
some rows to fail for ordinary reasons: a package that is not installed, a tool your
distribution splits into a separate package, or an optional feature built out. A versioned
shared library that is present only in another version (for example `libfoo.so.2.x` where
the spec lists `libfoo.so.3`) still passes the artifacts check, with the note "different
version"; a real version mismatch then shows up in the smoke or program checks. The
failure detail says which file, symbol or command was missing, so you can tell those apart
from a broken install.

A few practical notes:

- CUDA components installed from NVIDIA's packages usually live under `/usr`,
  `/usr/local/cuda` or a versioned `cuda-<version>` folder. Run the `cuda_*` packages with
  the matching `--prefix` and `--cuda-home`.
- NVIDIA's rpm and deb packages of some CUDA math libraries (cuTENSOR, cuDSS, nvJPEG2000,
  nvTIFF) put the libraries and headers in a per-CUDA-version folder such as
  `lib64/libcutensor/<cuda major>/`; run those packages with `--prefix /usr`.
- `cuda_nvdebug`: NVIDIA installs it under `/opt/nvidia/nvdebug`; run it with
  `--prefix /opt/nvidia/nvdebug`.
- Checks that need hardware or a driver (`requires` in the spec) are skipped unless you
  pass the matching `--with` option.
- A NEEDED entry that resolves only through the prefix's own lib directories still passes,
  with a note: such a binary needs `LD_LIBRARY_PATH` or a loader entry to run.

## Adding or changing a package

1. Create `<package>/testspec.json`; the folder name must equal `name`.
2. Put each test program beside it and list it under `test_programs`. One program per
   aspect of the library keeps failures easy to read.
3. Run `python3 run_native_checks.py --prefix <your prefix> -k <package> -v`.

[SCHEMA.md](SCHEMA.md) describes every field, the placeholders and the capabilities.

The runner has its own self-test, which builds a tiny throwaway prefix with the local C
compiler and checks that every check passes, fails and skips where it should. It also
runs `--validate-only` over every spec in this folder:

```bash
# from the root of this repository
python3 -m unittest package_tests/native/test_run_native_checks.py

# or from package_tests/native
python3 -m unittest test_run_native_checks
```

## License

BSD-3-Clause, Copyright (c) 2026, Eugo (<https://eugo.io>). See the `LICENSE` file at
the root of this repository. Third-party material elsewhere in the repository is listed in
[THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md).

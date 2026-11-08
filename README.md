# ganymede

Example notebooks, Ray patterns, and smoke tests for widely used Python packages and native
libraries, from [Eugo](https://eugo.io).

| Folder | What is in it |
| --- | --- |
| [`ray_patterns/`](ray_patterns/) | Ray programs drawn in Eugo's visual Ray builder: each notebook pairs a picture of the graph with the Ray code the builder generated for it, run on Ray 2.55.1. |
| [`demo_notebooks/`](demo_notebooks/) | Short demos of popular Python libraries: data storage, geospatial, visualisation, data processing and more. |
| [`package_tests/python/`](package_tests/python/) | One smoke-test notebook per Python package: import it and exercise its main API. Runs under pytest. |
| [`package_tests/native/`](package_tests/native/) | Declarative checks for native libraries (files present, shared-library dependencies resolve, static archives are position-independent, small test programs compile and run), with a standalone runner. |

## Quick start

```bash
git clone https://github.com/eugo-inc/ganymede.git
cd ganymede
python -m venv .venv && . .venv/bin/activate
```

**Ray patterns** — install Ray and open any notebook:

```bash
pip install "ray[default,data,serve,train,tune]==2.55.1" "protobuf<7" jupyterlab
jupyter lab ray_patterns/
```

Some patterns need extra libraries (for example `torch`, `lightgbm`, `xgboost` or `optuna`);
the *Extra packages* column in [ray_patterns/README.md](ray_patterns/README.md) lists what each
one needs. Keep `protobuf` below 7: Ray Serve 2.55.1 does not work with protobuf 7.

**Package tests** — install the runner and the package you want to test, then run its notebook:

```bash
pip install -r package_tests/python/requirements.txt numpy
pytest package_tests/python/numpy.ipynb
```

**Native checks** — point the runner at an install prefix:

```bash
python package_tests/native/run_native_checks.py --prefix /usr/local -k zlib_ng
```

**Demo notebooks** — notebooks that connect to a database or a cloud service read their
connection settings from environment variables; the *Environment variables* cell right after
the title lists them. See [demo_notebooks/README.md](demo_notebooks/README.md).

## Checks

```bash
python tools/check_notebooks.py
```

verifies that every notebook parses, carries no stray metadata, and (outside `ray_patterns/`)
is committed without outputs.

## Contributing and security

See [CONTRIBUTING.md](CONTRIBUTING.md). Please report security issues privately as described in
[SECURITY.md](SECURITY.md).

## License

Original content is licensed under the [BSD 3-Clause License](LICENSE), Copyright (c) 2026, Eugo.
Some examples are adapted from third-party projects and remain under their own licenses; see
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

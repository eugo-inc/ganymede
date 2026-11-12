---
title: Open HPC, Ray and geospatial Python notebooks
description: "Open, BSD-licensed Jupyter notebooks from Eugo: case studies on public NOAA and Copernicus data, Ray patterns, Python library demos and package smoke tests."
---

# ganymede

ganymede is an open collection of Jupyter notebooks from [Eugo](https://eugo.io): worked case
studies on public NOAA and Copernicus data, small runnable programs for Ray Core, Ray Data, Ray
Serve, Ray Train and Ray Tune, short demos of popular Python libraries, and smoke tests for Python
packages and native libraries. The notebooks use open-source Python packages that you install
yourself, mostly with pip, and the source is on [GitHub](https://github.com/eugo-inc/ganymede) under the BSD 3-Clause License.

[Eugo](https://eugo.io) is building the performance cloud. We're making high performance computing the
default for every engineering team. None of these notebooks needs Eugo.

## Who it is for

- **Scientific and geospatial Python users** who want to read public satellite and hydrology
  archives (GOES-16, the National Water Model, Sentinel-2) straight from cloud storage with xarray,
  rasterio and STAC, and then scale the same step out with Ray.
- **Ray users** looking for one short, runnable example of a Ray feature: tasks and actors,
  datasets, model serving, distributed training or hyperparameter tuning.
- **Anyone checking an environment**: the package smoke tests show quickly whether an installed
  Python package or native library actually works.

## What is in it

| Section | What you will find |
| --- | --- |
| [Case studies](demo_notebooks/case_studies/index.md) | Longer worked examples on public data: land surface temperature from GOES-16 NetCDF files, streamflow forecasts from the NOAA National Water Model, and resampling Sentinel-2 imagery. Each one ends by running its per-file step in parallel with Ray. |
| [Ray patterns](ray_patterns/index.md) | One small Ray program per notebook, drawn in Eugo's visual Ray builder and shown with the plain Ray code it generated, run on Ray 2.55.1 with outputs (three that need a GPU, a model-serving engine or a data catalog are code only). |
| [Demo notebooks](demo_notebooks/index.md) | Short demos of libraries for data storage, geospatial work, visualisation, data processing, testing and more. |
| [Package tests](package_tests.md) | One smoke-test notebook per Python package, and declarative install checks for native libraries. |
| [FAQ](faq.md) | Short answers to common questions, each linking the notebook that shows it. |

## Gallery

Figures from the notebooks, as committed in the repository. Select one to open its notebook.

<!-- gallery -->

## Run the notebooks locally

```bash
git clone https://github.com/eugo-inc/ganymede.git
cd ganymede
python -m venv .venv && . .venv/bin/activate
```

**Ray patterns**: install Ray and open any notebook.

```bash
pip install "ray[default,data,serve,train,tune]==2.55.1" "protobuf<7" jupyterlab
jupyter lab ray_patterns/
```

Some patterns need extra libraries (for example `torch`, `lightgbm`, `xgboost` or `optuna`); the
[Ray patterns](ray_patterns/index.md) page lists what each one needs. Keep `protobuf` below 7:
Ray Serve 2.55.1 does not work with protobuf 7.

**Package tests**: install the runner and the package you want to test, then run its notebook.

```bash
pip install -r package_tests/python/requirements.txt numpy
pytest package_tests/python/numpy.ipynb
```

**Native checks**: point the runner at an install prefix.

```bash
python package_tests/native/run_native_checks.py --prefix /usr/local -k zlib_ng
```

**Demo notebooks**: notebooks that connect to a database or a cloud service read their connection
settings from environment variables; the *Environment variables* cell right after the title lists
them. The case studies need no account: every read is anonymous.

## Links

- Source code: [github.com/eugo-inc/ganymede](https://github.com/eugo-inc/ganymede)
- Cite: [doi:10.5281/zenodo.24117548](https://doi.org/10.5281/zenodo.24117548) (all versions; see [How do I cite ganymede?](faq.md#how-do-i-cite-ganymede))
- License: [BSD 3-Clause](https://github.com/eugo-inc/ganymede/blob/main/LICENSE), Copyright (c) 2026, Eugo, Inc. Adapted examples keep their own licenses; see [THIRD_PARTY_NOTICES.md](https://github.com/eugo-inc/ganymede/blob/main/THIRD_PARTY_NOTICES.md).
- Contributing: [CONTRIBUTING.md](https://github.com/eugo-inc/ganymede/blob/main/CONTRIBUTING.md) · Security: [SECURITY.md](https://github.com/eugo-inc/ganymede/blob/main/SECURITY.md)
- A plain-text index of every page for AI assistants: [llms.txt](llms.txt)
- Eugo: [eugo.io](https://eugo.io)

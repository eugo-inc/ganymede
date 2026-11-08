# Python package smoke tests

One Jupyter notebook per Python package. Each notebook imports its package and
exercises the package's main API with a few small, self-contained examples. A
notebook fails if any call raises an error, and many also assert on their
results. They are smoke tests: a quick way to check that an installed package
actually works in your environment. They do not replace the package's own test
suite.

Maintained by [Eugo](https://eugo.io).

## Running them

From the root of this repository:

```bash
pip install -r package_tests/python/requirements.txt <package>
pytest package_tests/python/<package>.ipynb
```

`<package>` is the notebook's file name without `.ipynb`, for example `requests`
or `scikit_learn`. pip treats `_` and `-` the same way, so
`pip install scikit_learn` installs `scikit-learn`.

To run several notebooks at once, select them with `-k`, for example
`pytest package_tests/python -k "requests or httpx"`. `-k` matches substrings,
so `-k requests` also selects `requests_cache`, `types_requests` and so on, and
each of those needs its own package installed.

Every notebook is collected as one test and runs in a fresh temporary working
directory, in a Jupyter kernel started with the same Python interpreter as
pytest, so the kernel sees exactly the packages installed in that environment.
`ipykernel` (it is in `requirements.txt`) must be installed there too. A
`python3` kernelspec you may have installed elsewhere (for example with
`python -m ipykernel install --user`) is not used. Some notebooks assert the behaviour of a
particular release of their package, so a much older or newer release can fail
an assertion even when it works. A few notebooks also use the package's
optional extras (the `dask` notebook, for example, needs
`pip install "dask[dataframe,distributed]"`, and the `cachecontrol` notebook
needs `pip install "cachecontrol[filecache]"`); if a cell stops with
`ModuleNotFoundError`, install the missing module and run the notebook again.
One exception: opentelemetry-instrumentation-openai-v2 2.4b0 imports
`opentelemetry.util.genai.instruments`, which opentelemetry-util-genai renamed
in 1.2b0, so that module cannot be installed. Until a newer openai-v2 release
ships, install `"opentelemetry-util-genai<=1.1b0"` alongside it (every
util-genai release is a pre-release, so pip ignores a plain `<1.2` pin).

Useful options:

| Option | What it does |
| --- | --- |
| `--with <capability>` | Run the cells that need `<capability>` (see below). Repeat it, or give a comma-separated list. `--with all` enables everything. |
| `--cell-timeout <seconds>` | Default time limit per cell. The default is 120 seconds. Unlike `timeout-<seconds>` tags, this value is not capped. |

The environment variable `PACKAGE_TESTS_WITH=cap1,cap2` does the same as
`--with cap1 --with cap2`. These options come from `conftest.py` in this
folder, so pytest only knows them when the path you give it is
`package_tests/python` or a notebook inside it.

## Cell tags

The notebooks use cell tags to say what a cell needs. The runner in
`conftest.py` reads them:

| Tag | Meaning |
| --- | --- |
| `requires-<capability>` | The cell needs something a typical machine may not have. It is skipped unless the capability is enabled. The end of the pytest run lists every notebook that skipped cells and what they needed. |
| `skip-by-default` | The whole notebook is skipped unless you pass `--with all`. |
| `allow-fail` | The cell may raise an error; the notebook carries on. |
| `timeout-<seconds>` | Time limit for that cell, instead of the default. Values above 600 are capped at 600. |

If every code cell in a notebook needs a capability you did not enable, the
whole notebook is reported as skipped.

## Capabilities

| Capability | The cell needs |
| --- | --- |
| `gpu` | An NVIDIA GPU that the package can use, with its driver. |
| `cuda-driver` | The NVIDIA driver library (`libcuda`) to be loadable. |
| `cuda-runtime` | The CUDA runtime libraries (for example `libcudart` and NVRTC) to be installed. |
| `cuda-static-libs` | The CUDA static libraries (for example `libcudadevrt.a`) to be installed, and `CUDA_HOME` or `CUDA_PATH` set to the CUDA Toolkit root. |
| `compiler` | A working C/C++ compiler on `PATH`. |
| `docker` | A reachable Docker daemon. |
| `docker-registry` | A Docker daemon that can reach Docker Hub. |
| `network` | Outbound internet access. |
| `huggingface-hub` | Access to the Hugging Face Hub to download models or datasets. |
| `aws-credentials` | AWS credentials. These cells make real, possibly billable, API calls: use a scratch account. |
| `gcp-credentials` | Google Cloud Application Default Credentials. These cells make real API calls. |

## License

BSD-3-Clause, like the rest of this repository. A few notebooks adapt code from the tests or
examples of the package they test; that code stays under its upstream license, listed in
[THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md).

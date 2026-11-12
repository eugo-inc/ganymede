---
title: Frequently asked questions
nav_title: FAQ
description: "Answers about ganymede: GOES-16, National Water Model and Sentinel-2 data in Python, Ray Data, Serve and Tune examples, package tests, license and citation."
---

# Frequently asked questions

These are short answers to common questions about ganymede, each grounded in a notebook or README
in the repository and linking to it.

## How do I read GOES-16 NetCDF files from public cloud storage in Python?

Open the public `s3://noaa-goes16` bucket anonymously with `s3fs.S3FileSystem(anon=True)`, list
the hourly files with `fs.glob`, download one into memory with `cat_file` and open it with
`xarray.open_dataset(..., engine="h5netcdf")`. The
[GOES-16 land surface temperature](demo_notebooks/case_studies/goes16_netcdf.ipynb) case study does
exactly this for the `ABI-L2-LSTC` product, applies the data quality flags and converts the
satellite's fixed-grid scan angles to latitude and longitude with pyproj. The `noaa-goes16` archive
ends in early April 2025, when GOES-19 replaced GOES-16; the `noaa-goes19` bucket carries the same
product for recent dates.

## How do I work with National Water Model output in Python?

The NOAA National Water Model publishes its forecasts as NetCDF files in the public
`s3://noaa-nwm-pds` bucket, one channel file per forecast hour, which xarray opens like any other
NetCDF file. The [National Water Model](demo_notebooks/case_studies/national_water_model.ipynb)
case study finds the newest complete 10-day (medium-range) forecast, maps the forecast streamflow
and follows it at five USGS stream gauges. Forecast files identify river reaches only by
`feature_id`, so the notebook reads reach coordinates and gauge ids from the NWM Retrospective v3.0
Zarr store.

## How do I resample Sentinel-2 bands to a common grid?

The [Sentinel-2 resampling](demo_notebooks/case_studies/sentinel2_resampling.ipynb) case study
finds scenes through the Earth Search STAC API and reads small windows of the Cloud-Optimized
GeoTIFFs over HTTPS with rasterio. It compares area averaging with nearest neighbour when going from
10 m to 60 m, and nearest neighbour with bilinear interpolation when going from 20 m to 10 m.
Categorical layers such as the scene classification layer are only ever resampled with nearest
neighbour, because averaging class codes produces meaningless classes.

## How do I run the same analysis over many files in parallel with Ray?

Wrap the per-file function with `ray.remote`, submit one task per file and collect the results with
`ray.get`; each task downloads and reads its own file, so only a few numbers per file return to the
driver. All three case studies end with such a "Scaling out with Ray" section. The Ray patterns
show the building blocks on their own:
[fan out one task per list item](ray_patterns/core/03_map-over-list.ipynb),
[map and reduce](ray_patterns/core/01_map-reduce.ipynb) and
[share one object with ray.put](ray_patterns/core/09_put-shared-object.ipynb).

## How do I write a Ray Data pipeline?

Start with the [three-step pipeline](ray_patterns/data/15_data-pipeline.ipynb) that creates a
dataset, transforms it with `map_batches` and counts the rows. Other patterns cover
[writing Parquet](ray_patterns/data/16_data-write-parquet.ipynb),
[groupby and aggregate](ray_patterns/data/17_data-groupby-aggregate.ipynb),
[joining two datasets](ray_patterns/data/19_data-join.ipynb) and
[batch inference with an actor pool](ray_patterns/data/21_data-actor-pool-batch-inference.ipynb).
Each notebook shows the code, runs it on Ray 2.55.1 and checks the result.

## How do I deploy a model with Ray Serve?

The [compose two deployments](ray_patterns/serve/27_serve-greeter-driver.ipynb) pattern passes one
deployment into another with `.bind()` and starts both with `serve.run`. Further patterns add
[autoscaling](ray_patterns/serve/29_serve-autoscaling.ipynb), a
[FastAPI ingress](ray_patterns/serve/30_serve-fastapi-ingress.ipynb),
[dynamic request batching](ray_patterns/serve/31_serve-dynamic-batching.ipynb) and
[model multiplexing](ray_patterns/serve/32_serve-model-multiplexing.ipynb). On Ray 2.55.1, keep
`protobuf` below 7, and give `@serve.ingress` a builder function that creates the FastAPI app rather
than the app object, as the FastAPI pattern does.

## How do I tune hyperparameters with Ray Tune?

The [grid search with ASHA](ray_patterns/tune/40_tune-asha.ipynb) pattern sweeps the learning
rate of a TorchTrainer while the ASHA scheduler stops weak trials early. The
[Optuna search](ray_patterns/tune/41_tune-optuna.ipynb) pattern samples the learning rate with
OptunaSearch instead, and the
[stopper and ResultGrid](ray_patterns/tune/42_tune-stopper-and-result-grid.ipynb) pattern adds the
median stopping rule and reads the trial results back as a DataFrame. These patterns need `torch`
(and `optuna` for the second) besides Ray.

## Do these notebooks need Eugo or Eugo's visual Ray builder?

No. The Ray patterns pair a picture of a graph drawn in Eugo's visual Ray builder with the code it
generated, but that code is plain Ray: the builder is not publicly available and nothing in the
repository needs it. The notebooks use open-source Python packages that you install yourself, mostly
with pip, as listed in each notebook's first cell or in its folder README. Some demo notebooks also
need a running service, an account or a system tool, such as a database server, a cloud bucket, an
API key or Java.

## What do the package smoke tests check?

Each [Python package test](package_tests.md) is a notebook that imports one package and exercises
its main API with small examples; it fails if any call raises an error, and many also assert on
their results. The native checks verify that a library's files exist under an install prefix,
that its shared-library dependencies resolve, that static archives are position-independent where
required, and that small C/C++ test programs compile, link and run. They are quick checks that an
install works, not a replacement for each package's own test suite.

## Which Ray version do the Ray patterns use?

The Ray patterns were run on Ray 2.55.1 with Ray capped at four CPUs and no GPU; install it with
`pip install "ray[default,data,serve,train,tune]==2.55.1" "protobuf<7"`. A few patterns are marked
*code only* because they need a GPU, a model-serving engine or an external data catalog, and the
[catalog table](ray_patterns/data/25_data-catalog-table.ipynb) pattern also needs Ray 2.58.0 or
later. The [Ray patterns](ray_patterns/index.md) page lists the extra packages each pattern needs.

## What license is ganymede under?

Original content is under the
[BSD 3-Clause License](https://github.com/eugo-inc/ganymede/blob/main/LICENSE), Copyright (c) 2026,
Eugo, Inc. Some examples are adapted from third-party projects and stay under their own licenses.
[THIRD_PARTY_NOTICES.md](https://github.com/eugo-inc/ganymede/blob/main/THIRD_PARTY_NOTICES.md)
lists those projects and the notebooks that use them; adapted demo notebooks also say so in their
first cell. The public datasets used by the case studies carry their own terms, which
each case study states, such as NOAA's attribution request and the Copernicus Sentinel data notice.

## How do I cite ganymede?

Use the citation metadata in
[CITATION.cff](https://github.com/eugo-inc/ganymede/blob/main/CITATION.cff) at the root of the
repository. On the [repository page](https://github.com/eugo-inc/ganymede), GitHub turns that
file into a "Cite this repository" button with ready-made APA and BibTeX entries. The repository is
archived on Zenodo: [doi:10.5281/zenodo.24117548](https://doi.org/10.5281/zenodo.24117548) covers all versions and always resolves to the
newest release, and release v2026.11.0 is
[doi:10.5281/zenodo.24117549](https://doi.org/10.5281/zenodo.24117549). To cite the exact files you used, cite the release DOI.
In BibTeX, the double braces keep the company name from being read as a person's name:

```bibtex
@software{ganymede_2026_11_0,
  author    = {{Eugo, Inc.}},
  title     = {ganymede: open HPC notebooks from Eugo},
  version   = {2026.11.0},
  year      = {2026},
  publisher = {Zenodo},
  doi       = {10.5281/zenodo.24117549},
  url       = {https://doi.org/10.5281/zenodo.24117549}
}
```

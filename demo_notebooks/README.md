# Demo notebooks

Short, self-contained demos of popular Python libraries, grouped by topic:

| Folder | What is in it |
| --- | --- |
| [`aws/`](aws/) | The AWS Common Runtime bindings. |
| [`case_studies/`](case_studies/) | Longer worked examples on public data: the National Water Model, GOES-16 NetCDF files and Sentinel-2 resampling. |
| [`data_analytics/`](data_analytics/) | Bottleneck. |
| [`data_processing/`](data_processing/) | pandas, Dask, SciPy, scikit-learn, pydantic, pandera, NLTK and more. |
| [`data_storage/`](data_storage/) | Databases (PostgreSQL, MySQL, SQL Server, Oracle, SQLite and others), file systems and cloud storage, Arrow. |
| [`data_structures/`](data_structures/) | jsonschema, multidict and other data structure libraries. |
| [`data_visualization/`](data_visualization/) | Matplotlib, Bokeh, HoloViews, GeoViews, Cartopy, Plotly, Folium, Graphviz and more. |
| [`geospatial/`](geospatial/) | GeoPandas, GDAL, xarray, PySAL, NetCDF and HDF5, STAC, and Google and Azure geospatial services. |
| [`networking/`](networking/), [`security/`](security/), [`testing/`](testing/), [`utilities/`](utilities/), [`web_servers/`](web_servers/) | HTTP clients, `.env` files, Faker and Hypothesis, small utilities, Tornado. |

## Running them

Each notebook starts with a title cell that says what the library does and links its
documentation under *Resources*. Install the library it demonstrates, plus JupyterLab, and run
the cells from the top.

- The notebooks are committed without outputs.
- Notebooks that connect to a database or a cloud service read their connection settings from
  environment variables. The *Environment variables* cell right after the title lists them; set
  them before you start Jupyter.
- Many demos need a running service or an account (a database server, a cloud bucket, an API
  key), so they cannot all run on a plain laptop.

## Where the examples come from

Many examples follow the documentation, README, tutorials or example galleries of the library
they demonstrate, and some are adapted from them. Each notebook links those sources under
*Resources*. Material adapted from a project stays under that project's license (BSD, MIT,
Apache-2.0 and others), not under this repository's BSD-3-Clause license.

Notebooks that contain adapted code say so in a *Source and license* entry in their first cell.
[THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md) lists those projects with their copyright
notices and the full license texts.

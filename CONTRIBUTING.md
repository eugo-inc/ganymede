# Contributing

Thanks for your interest in improving these examples and tests.

## Ground rules

- Open an issue first for anything larger than a small fix, so we can agree on the approach.
- Keep notebooks self-contained: each one should run on its own, top to bottom.
- Never commit credentials, hostnames or account ids. Notebooks that talk to a service read their
  connection settings from environment variables (see the note at the top of each such notebook).
- Commit notebooks **without outputs** (`demo_notebooks/` and `package_tests/`). The pattern
  notebooks under `ray_patterns/` are the exception: they are committed with outputs from a
  real run, so readers can see what each pattern produces.
- Run the notebook checks before opening a pull request:

  ```bash
  python tools/check_notebooks.py
  ```

## Adding a package test

Add `package_tests/python/<package>.ipynb` with a few small examples that import the package
and exercise its main API. Tag cells that need extra capabilities (`requires-network`,
`requires-gpu`, …) as described in [package_tests/python/README.md](package_tests/python/README.md).

## License

By contributing you agree that your contributions are licensed under the
[BSD 3-Clause License](LICENSE).

If you adapt code from another project (its documentation, examples or tests), say so in a
comment or in the notebook's first cell, name the source and its license, and add the project
to [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). That code stays under its own license.
Only adapt code whose license allows it to be redistributed here (for example MIT, BSD or
Apache-2.0).

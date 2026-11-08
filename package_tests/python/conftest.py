# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo
"""pytest plugin that runs every notebook in this folder as one test.

Each notebook is a smoke test for one Python package: it imports the package and
exercises its main API. Cell tags decide what runs:

``requires-<capability>``
    The cell needs something a plain machine may not have (a GPU, network access,
    cloud credentials, ...). It is skipped unless the capability is enabled with
    ``--with <capability>`` (repeatable or comma-separated) or with the environment
    variable ``PACKAGE_TESTS_WITH=cap1,cap2``. ``--with all`` enables every capability.
``skip-by-default``
    The whole notebook is skipped unless ``--with all`` is given.
``allow-fail``
    The cell may raise; the notebook carries on.
``timeout-<seconds>``
    Time limit for that cell, capped at 600 seconds. Cells without the tag use
    ``--cell-timeout`` (default 120 seconds, not capped).

Each notebook runs in a fresh temporary working directory, in an IPython kernel started
with the interpreter that runs pytest (``sys.executable``), so it sees the packages of
that environment. A ``python3`` kernelspec installed elsewhere (for example with
``python -m ipykernel install --user``) is not used. Only the standard library, pytest,
nbclient, nbformat and jupyter_client are used here.
"""

from __future__ import annotations

import json
import os
import re
import sys
import tempfile
from pathlib import Path

import nbformat
import pytest
from jupyter_client.kernelspec import KernelSpecManager
from jupyter_client.manager import AsyncKernelManager
from nbclient import NotebookClient
from nbclient.exceptions import CellExecutionError, CellTimeoutError, DeadKernelError

HERE = Path(__file__).resolve().parent
ENV_VAR = "PACKAGE_TESTS_WITH"
REQUIRES_PREFIX = "requires-"
DEFAULT_CELL_TIMEOUT = 120
MAX_CELL_TIMEOUT = 600
KERNEL_STARTUP_TIMEOUT = 60
TIMEOUT_TAG = re.compile(r"^timeout-(\d+)$")
ANSI_ESCAPE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")
SKIPPED_CELLS_KEY = "skipped_cells"
MISSING_CAPS_KEY = "missing_capabilities"
KERNEL_NAME = "package-tests-python"


def pytest_addoption(parser):
    group = parser.getgroup("package-tests", "package smoke-test notebooks")
    group.addoption(
        "--with",
        action="append",
        dest="package_tests_with",
        default=[],
        metavar="CAPABILITY",
        help=(
            "run cells tagged requires-CAPABILITY (repeatable or comma-separated; "
            f"'all' enables everything). Also read from ${ENV_VAR}."
        ),
    )
    group.addoption(
        "--cell-timeout",
        dest="package_tests_cell_timeout",
        type=int,
        default=DEFAULT_CELL_TIMEOUT,
        metavar="SECONDS",
        help=(
            f"default per-cell time limit (default {DEFAULT_CELL_TIMEOUT}; not capped). "
            f"timeout-N cell tags override it, up to {MAX_CELL_TIMEOUT}."
        ),
    )


def _split_caps(values):
    caps = set()
    for value in values:
        for cap in str(value).split(","):
            cap = cap.strip().lower()
            if cap.startswith(REQUIRES_PREFIX):
                cap = cap[len(REQUIRES_PREFIX):]
            if cap:
                caps.add(cap)
    return caps


def enabled_capabilities(config) -> set[str]:
    from_cli = config.getoption("package_tests_with", default=None) or []
    return _split_caps(from_cli) | _split_caps([os.environ.get(ENV_VAR, "")])


def _tags(cell) -> list[str]:
    return list(cell.get("metadata", {}).get("tags", []) or [])


def _kernel_manager(kernels_dir: str) -> AsyncKernelManager:
    """A kernel manager whose only kernelspec runs ipykernel with this interpreter.

    Asking jupyter_client for "python3" would use the first such kernelspec in the
    user's or the system's Jupyter data directories, which may point at another Python
    that does not have the package under test.
    """
    spec_dir = Path(kernels_dir) / KERNEL_NAME
    spec_dir.mkdir(parents=True, exist_ok=True)
    spec = {
        "argv": [sys.executable, "-m", "ipykernel_launcher", "-f", "{connection_file}"],
        "display_name": "Python (pytest interpreter)",
        "language": "python",
    }
    (spec_dir / "kernel.json").write_text(json.dumps(spec), encoding="utf-8")
    specs = KernelSpecManager(kernel_dirs=[kernels_dir], ensure_native_kernel=False)
    return AsyncKernelManager(kernel_name=KERNEL_NAME, kernel_spec_manager=specs)


def pytest_collect_file(parent, file_path):
    if file_path.suffix != ".ipynb":
        return None
    try:
        file_path.resolve().relative_to(HERE)
    except ValueError:
        return None
    return NotebookFile.from_parent(parent, path=file_path)


class NotebookFile(pytest.File):
    def collect(self):
        yield NotebookItem.from_parent(self, name=self.path.stem)


class NotebookItem(pytest.Item):
    def runtest(self):
        config = self.config
        enabled = enabled_capabilities(config)
        everything = "all" in enabled
        nb = nbformat.read(self.path, as_version=4)

        if not everything and any("skip-by-default" in _tags(c) for c in nb.cells):
            pytest.skip("notebook is tagged skip-by-default; run it with --with all")

        code_cells = 0
        skipped = 0
        missing: set[str] = set()
        for cell in nb.cells:
            if cell.cell_type != "code":
                continue
            code_cells += 1
            tags = _tags(cell)
            needed = {t[len(REQUIRES_PREFIX):] for t in tags if t.startswith(REQUIRES_PREFIX)}
            lacking = set() if everything else needed - enabled
            extra = []
            if lacking:
                skipped += 1
                missing |= lacking
                extra.append("skip-execution")  # nbclient does not run cells with this tag
            if "allow-fail" in tags:
                extra.append("raises-exception")  # nbclient tolerates errors in cells with this tag
            if extra:
                cell.setdefault("metadata", {})["tags"] = tags + extra

        if skipped:
            self.user_properties.append((SKIPPED_CELLS_KEY, skipped))
            self.user_properties.append((MISSING_CAPS_KEY, ",".join(sorted(missing))))
        if code_cells and skipped == code_cells:
            pytest.skip(
                "every code cell needs a capability that is not enabled: "
                + ", ".join(sorted(missing))
                + " (enable with --with)"
            )

        # The command-line value is used as given; only timeout-N tags are capped.
        default_timeout = int(config.getoption("package_tests_cell_timeout", default=None) or DEFAULT_CELL_TIMEOUT)

        def cell_timeout(cell):
            for tag in _tags(cell):
                m = TIMEOUT_TAG.match(tag)
                if m:
                    return min(int(m.group(1)), MAX_CELL_TIMEOUT)
            return default_timeout

        with tempfile.TemporaryDirectory(prefix="package-test-") as workdir, \
                tempfile.TemporaryDirectory(prefix="package-test-kernel-") as kernels_dir:
            client = NotebookClient(
                nb,
                km=_kernel_manager(kernels_dir),
                timeout=default_timeout,
                timeout_func=cell_timeout,
                startup_timeout=KERNEL_STARTUP_TIMEOUT,
                allow_errors=False,
                resources={"metadata": {"path": workdir}},
            )
            # The client did not create the kernel manager, so ask it to shut the kernel down.
            client.execute(cleanup_kc=True)

    def repr_failure(self, excinfo, style=None):
        if isinstance(excinfo.value, (CellExecutionError, CellTimeoutError, DeadKernelError)):
            return ANSI_ESCAPE.sub("", f"{self.path.name}: {excinfo.value}")
        return super().repr_failure(excinfo)

    def reportinfo(self):
        return self.path, None, f"notebook {self.path.name}"


def pytest_terminal_summary(terminalreporter, exitstatus, config):
    rows = []
    for reports in terminalreporter.stats.values():
        for report in reports:
            if getattr(report, "when", None) != "call":
                continue
            props = dict(getattr(report, "user_properties", []) or [])
            if SKIPPED_CELLS_KEY in props:
                rows.append((report.nodeid, props[SKIPPED_CELLS_KEY], props.get(MISSING_CAPS_KEY, "")))
    if not rows:
        return
    terminalreporter.section("notebook cells skipped (capability not enabled)")
    for nodeid, count, caps in sorted(rows):
        terminalreporter.write_line(f"{nodeid}: {count} cell(s) skipped, needs: {caps}")
    terminalreporter.write_line(f"Enable a capability with --with <capability> or {ENV_VAR}=cap1,cap2.")

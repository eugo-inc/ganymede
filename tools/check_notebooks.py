#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo, Inc.
"""Structural checks for every notebook in this repository.

- every .ipynb parses as nbformat 4 JSON;
- notebook metadata holds only ``kernelspec`` and ``language_info``;
- cell metadata holds only ``tags``;
- ``ray_patterns/`` notebooks may carry any outputs (they are committed with outputs from a
  real run);
- the demo notebooks listed in ``FIGURE_NOTEBOOKS`` below (those that draw plots or maps) may
  carry outputs and execution counts, but only of these kinds: ``stdout`` streams, and
  display/execute results whose mime types are ``image/png``, ``image/jpeg``, ``text/plain`` or
  ``text/html``. Images must be real PNG or JPEG data. HTML may use only the plain table and text
  tags in ``HTML_TAGS`` and the attributes in ``HTML_ATTRS`` (no scripts, event handlers, embedded
  objects or remote resources). Output metadata may hold only image sizes. Error outputs,
  ``stderr`` streams and other mime types are never allowed, and no output text may show a local
  path, a local or private network address, or a Python object's memory address;
- everywhere else (the rest of ``demo_notebooks/`` and all of ``package_tests/`` included) code
  cells carry no outputs and no execution counts.

To let another demo notebook keep its outputs, add its path to ``FIGURE_NOTEBOOKS``.

Exit status 1 when any check fails.
"""
from __future__ import annotations

import base64
import binascii
import json
import re
import subprocess
import sys
from html import unescape
from html.parser import HTMLParser
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUTS_ALLOWED = ("ray_patterns/",)
# Demo notebooks committed with their figures; every other demo notebook stays output-free.
FIGURE_NOTEBOOKS = frozenset(
    "demo_notebooks/" + p
    for p in (
        "case_studies/goes16_netcdf.ipynb",
        "case_studies/national_water_model.ipynb",
        "case_studies/sentinel2_resampling.ipynb",
        "data_processing/scipy.ipynb",
        "data_visualization/bokeh/bokeh.ipynb",
        "data_visualization/branca/branca.ipynb",
        "data_visualization/cartopy.ipynb",
        "data_visualization/contourpy/contourpy.ipynb",
        "data_visualization/folium/folium.ipynb",
        "data_visualization/geoplot/geoplot.ipynb",
        "data_visualization/graphviz/graphviz.ipynb",
        "data_visualization/graphviz/pydot.ipynb",
        "data_visualization/graphviz/pygraphviz.ipynb",
        "data_visualization/holoviews/geoviews/geoviews.ipynb",
        "data_visualization/holoviews/holoviews/holoviews.ipynb",
        "data_visualization/holoviews/hvplot/hvplot.ipynb",
        "data_visualization/holoviews/panel/panel.ipynb",
        "data_visualization/matplotlib.ipynb",
        "data_visualization/mplleaflet/mplleaflet.ipynb",
        "data_visualization/plotly/plotly.ipynb",
        "data_visualization/railroad/railroad.ipynb",
        "data_visualization/seaborn/seaborn.ipynb",
        "geospatial/descartes.ipynb",
        "geospatial/pysal/mgwr.ipynb",
        "geospatial/pysal/spopt.ipynb",
    )
)
IMAGE_MAGIC = {"image/png": b"\x89PNG\r\n\x1a\n", "image/jpeg": b"\xff\xd8\xff"}
LIMITED_MIMES = {*IMAGE_MAGIC, "text/plain", "text/html"}
OUTPUT_KEYS = {
    "stream": {"output_type", "name", "text"},
    "display_data": {"output_type", "data", "metadata"},
    "execute_result": {"output_type", "data", "metadata", "execution_count"},
}
# Tags and attributes of a plain table or text block, such as a pandas DataFrame's HTML.
HTML_TAGS = {
    "div", "style", "table", "caption", "thead", "tbody", "tfoot", "tr", "th", "td",
    "span", "br", "p", "b", "i", "em", "strong", "code", "pre",
}
HTML_ATTRS = {"class", "style", "border", "colspan", "rowspan", "scoped", "valign", "halign", "align"}
# CSS that can load or run something; backslash escapes and "<" are refused outright, so that
# escaped spellings (\75rl) and parser differences around </style> cannot hide one.
CSS_FORBIDDEN = ("\\", "<", "url(", "image-set(", "image(", "src(", "@import", "expression(",
                 "javascript:", "behavior:", "-moz-binding")
LOCAL_PATTERNS = (
    re.compile(r"(?<![\w.~-])/(?:home|root|Users|opt|mnt|srv|tmp|var/folders)/"),
    re.compile(r"[A-Za-z]:\\Users\\"),
    re.compile(r"\bfile://"),
    re.compile(r"\b(?:localhost|127\.0\.0\.1|0\.0\.0\.0)\b"),
    re.compile(r"\b(?:10\.\d{1,3}|192\.168|172\.(?:1[6-9]|2\d|3[01]))\.\d{1,3}\.\d{1,3}\b"),
    re.compile(r"\bat 0x[0-9a-fA-F]{6,}>"),
)
NB_META = {"kernelspec", "language_info"}
CELL_META = {"tags"}
# Folders skipped when the tree is walked without git (dot-folders are always skipped).
SKIP_DIRS = {"venv", "env", "build", "dist", "node_modules", "site-packages", "__pycache__"}


def check(path: Path) -> list[str]:
    rel = path.relative_to(ROOT).as_posix()
    try:
        nb = json.loads(path.read_text(encoding="utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        return [f"{rel}: does not parse ({exc})"]
    if not isinstance(nb, dict):
        return [f"{rel}: is not a notebook object"]
    problems = []
    if nb.get("nbformat") != 4:
        problems.append(f"{rel}: nbformat {nb.get('nbformat')} (expected 4)")
    extra = set(nb.get("metadata", {})) - NB_META
    if extra:
        problems.append(f"{rel}: unexpected notebook metadata {sorted(extra)}")
    outputs_ok = rel.startswith(OUTPUTS_ALLOWED)
    outputs_limited = rel in FIGURE_NOTEBOOKS
    for i, cell in enumerate(nb.get("cells", [])):
        extra = set(cell.get("metadata", {})) - CELL_META
        if extra:
            problems.append(f"{rel}: cell {i} has unexpected metadata {sorted(extra)}")
        if cell.get("cell_type") != "code" or outputs_ok:
            continue
        if outputs_limited:
            outputs = cell.get("outputs", [])
            if not isinstance(outputs, list):
                problems.append(f"{rel}: cell {i} outputs is not a list")
                continue
            for k, output in enumerate(outputs):
                problems.extend(f"{rel}: cell {i} output {k} {msg}" for msg in check_output(output))
            continue
        if cell.get("outputs"):
            problems.append(f"{rel}: cell {i} has outputs")
        if cell.get("execution_count") is not None:
            problems.append(f"{rel}: cell {i} has an execution count")
    return problems


def _text(value) -> str | None:
    """A text field (a string or a list of strings) as one string; None when it is anything else."""
    if isinstance(value, str):
        return value
    if isinstance(value, list) and all(isinstance(part, str) for part in value):
        return "".join(value)
    return None


class _HtmlAllowlist(HTMLParser):
    """Collects the problems of an HTML output against ``HTML_TAGS`` / ``HTML_ATTRS``."""

    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.problems: list[str] = []
        self._in_style = False

    def _css(self, css: str, where: str) -> None:
        flat = re.sub(r"\s+", "", unescape(css)).lower()
        bad = [token for token in CSS_FORBIDDEN if token in flat]
        if bad:
            self.problems.append(f"has CSS with {bad} in {where}")

    def handle_starttag(self, tag, attrs):
        if tag not in HTML_TAGS:
            self.problems.append(f"has the HTML tag <{tag}> (allowed: {sorted(HTML_TAGS)})")
        for name, value in attrs:
            if name not in HTML_ATTRS:
                self.problems.append(f"has the HTML attribute {name!r} on <{tag}>")
            elif name == "style" and value:
                self._css(value, f"a style attribute on <{tag}>")
        self._in_style = tag == "style"

    def handle_startendtag(self, tag, attrs):
        if tag == "style":
            self.problems.append("has a self-closing <style/> tag")
        self.handle_starttag(tag, attrs)
        self._in_style = False

    def handle_endtag(self, tag):
        self._in_style = False

    def handle_data(self, data):
        if self._in_style:
            self._css(data, "a <style> block")

    def handle_comment(self, data):
        self.problems.append("has an HTML comment")

    def handle_decl(self, decl):
        self.problems.append("has an HTML declaration")

    def handle_pi(self, data):
        self.problems.append("has an HTML processing instruction")

    def unknown_decl(self, data):
        self.problems.append("has an HTML declaration")


def check_html(html: str) -> list[str]:
    parser = _HtmlAllowlist()
    parser.feed(html)
    parser.close()
    return sorted(set(parser.problems))


def check_image(mime: str, value) -> list[str]:
    text = _text(value)
    if text is None:
        return [f"has {mime} data that is not a base64 string"]
    try:
        raw = base64.b64decode("".join(text.split()), validate=True)
    except (binascii.Error, ValueError):
        return [f"has {mime} data that is not valid base64"]
    if not raw.startswith(IMAGE_MAGIC[mime]):
        return [f"has {mime} data that is not a {mime.split('/')[1].upper()} image"]
    return []


def check_metadata(metadata) -> list[str]:
    if not isinstance(metadata, dict):
        return ["has output metadata that is not an object"]
    for mime, value in metadata.items():
        if mime not in IMAGE_MAGIC or not isinstance(value, dict) or set(value) - {"width", "height"} \
                or not all(isinstance(v, int) for v in value.values()):
            return [f"has output metadata {mime!r} (only image width/height are allowed)"]
    return []


def check_output(output) -> list[str]:
    """Problems with one output of a ``FIGURE_NOTEBOOKS`` notebook (see the module docstring)."""
    if not isinstance(output, dict):
        return ["is not an output object"]
    kind = output.get("output_type")
    if kind not in OUTPUT_KEYS:
        return [f"is an output of type {kind} (only stdout streams and display/execute results are allowed)"]
    extra = sorted(set(output) - OUTPUT_KEYS[kind])
    if extra:
        return [f"has unexpected fields {extra}"]
    texts = []
    if kind == "stream":
        if output.get("name") != "stdout":
            return [f"is a {output.get('name')} stream (only stdout is allowed)"]
        texts.append(("stdout", _text(output.get("text"))))
    else:
        data = output.get("data", {})
        if not isinstance(data, dict):
            return ["has data that is not an object"]
        bad = sorted(set(data) - LIMITED_MIMES)
        if bad:
            return [f"has mime types {bad} (allowed: {sorted(LIMITED_MIMES)})"]
        problems = check_metadata(output.get("metadata", {}))
        for mime in sorted(set(data) & set(IMAGE_MAGIC)):
            problems += check_image(mime, data[mime])
        html = _text(data.get("text/html", ""))
        if html:
            problems += check_html(html)
        if problems:
            return problems
        texts += [(m, _text(data[m])) for m in ("text/plain", "text/html") if m in data]
    for mime, text in texts:
        if text is None:
            return [f"has {mime} text that is not a string or a list of strings"]
        found = sorted({m.group(0) for pattern in LOCAL_PATTERNS for m in pattern.finditer(text)})
        if found:
            return [f"shows a local path, a local address or a memory address {found[:3]}"]
    return []


def discover() -> list[Path]:
    """The notebooks git would commit: tracked files plus untracked files that are not ignored.

    Outside a git checkout (or when git lists nothing), walk the tree instead, skipping
    dot-folders and the usual virtual-environment and build folders.
    """
    try:
        out = subprocess.run(
            ["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard", "--", "*.ipynb"],
            cwd=ROOT, capture_output=True, check=True,
        ).stdout.decode("utf-8")
        listed = {ROOT / rel for rel in out.split("\0") if rel}
    except (OSError, subprocess.CalledProcessError):
        listed = set()
    found = sorted(p for p in listed if p.is_file())
    if found:
        return found
    return sorted(
        p for p in ROOT.rglob("*.ipynb")
        if not any(part.startswith(".") or part in SKIP_DIRS for part in p.relative_to(ROOT).parts[:-1])
    )


def main() -> int:
    notebooks = discover()
    problems = []
    for nb in notebooks:
        try:
            problems += check(nb)
        except Exception as exc:  # noqa: BLE001 - a malformed notebook is a problem, not a crash
            problems.append(f"{nb.relative_to(ROOT).as_posix()}: could not be checked ({type(exc).__name__}: {exc})")
    for msg in problems:
        print(msg)
    print(f"checked {len(notebooks)} notebooks, {len(problems)} problem(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())

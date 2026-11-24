#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo, Inc.
"""Assemble the MkDocs source of the ganymede website, and refresh its gallery images.

    python tools/build_site.py --out build/site
    mkdocs build --strict -f build/site/mkdocs.yml      # the site lands in build/site/site

    python tools/build_site.py --refresh-gallery        # rewrite docs/assets/gallery/*.png

``--out DIR`` writes a MkDocs project that mirrors the repository layout:

- ``DIR/mkdocs.yml``: the repository's ``mkdocs.yml`` plus a generated ``nav:`` section;
- ``DIR/overrides/``: the theme overrides;
- ``DIR/docs/``: ``docs/`` plus every notebook of ``demo_notebooks/`` and ``ray_patterns/`` at its
  repository path (so page URLs mirror repository paths), ``ray_patterns/images/``, an index page
  per notebook folder, ``package_tests.md`` with the package lists filled in, the home page
  gallery, and ``llms.txt``;
- ``DIR/pages.json``: title, description and GitHub link of every page, read by ``docs/hooks.py``.

Notebooks are copied unchanged and never executed. ``--refresh-gallery`` takes the first PNG
figure of each notebook in ``GALLERY``, drops every PNG chunk except image data and colour
information (so no text or EXIF chunks remain), scales it to at most ``GALLERY_MAX_WIDTH`` pixels
wide and writes it to ``docs/assets/gallery/<notebook stem>.png``.

Only the standard library is used.
"""
from __future__ import annotations

import argparse
import base64
import hashlib
import json
import posixpath
import re
import shutil
import struct
import subprocess
import sys
import zlib
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SITE_URL = "https://eugo-inc.github.io/ganymede/"
REPO_URL = "https://github.com/eugo-inc/ganymede"
BRANCH = "main"
NOTEBOOK_ROOTS = ("demo_notebooks", "ray_patterns")
MARKER = ".ganymede-site"  # marks an --out folder this script created and may clear

# Notebooks whose first PNG figure appears in the home page gallery, with a short caption.
GALLERY = (
    ("demo_notebooks/case_studies/goes16_netcdf.ipynb",
     "Land surface temperature from NOAA GOES-16, read straight from the public S3 bucket."),
    ("demo_notebooks/case_studies/national_water_model.ipynb",
     "A 10-day streamflow forecast from the NOAA National Water Model."),
    ("demo_notebooks/case_studies/sentinel2_resampling.ipynb",
     "A Sentinel-2 tile over central Iowa, read as Cloud-Optimized GeoTIFFs."),
    ("demo_notebooks/data_visualization/holoviews/geoviews/geoviews.ipynb",
     "Ocean, land and borders on an orthographic globe with GeoViews."),
    ("demo_notebooks/data_visualization/holoviews/holoviews/holoviews.ipynb",
     "The Lorenz attractor drawn with HoloViews."),
    ("demo_notebooks/data_visualization/seaborn/seaborn.ipynb",
     "A seaborn pair plot of the iris data set."),
    ("demo_notebooks/geospatial/pysal/spopt.ipynb",
     "Regions of Mexico built with PySAL spopt."),
    ("demo_notebooks/geospatial/pysal/mgwr.ipynb",
     "Georgia counties and their centroids, from the PySAL mgwr demo."),
)
GALLERY_DIR = ROOT / "docs" / "assets" / "gallery"
GALLERY_MAX_WIDTH = 1200

# Display names for notebook folders; any other folder is named after its last path part.
FOLDER_TITLES = {
    "demo_notebooks": "Demo notebooks",
    "demo_notebooks/aws": "AWS",
    "demo_notebooks/case_studies": "Case studies",
    "demo_notebooks/data_analytics": "Data analytics",
    "demo_notebooks/data_processing": "Data processing",
    "demo_notebooks/data_processing/dask": "Dask",
    "demo_notebooks/data_processing/ray": "Ray (RayDP)",
    "demo_notebooks/data_storage": "Data storage",
    "demo_notebooks/data_storage/databases": "Databases",
    "demo_notebooks/data_storage/databases/mariadb": "MariaDB",
    "demo_notebooks/data_storage/databases/mssql": "Microsoft SQL Server",
    "demo_notebooks/data_storage/databases/mysql": "MySQL",
    "demo_notebooks/data_storage/databases/oracle": "Oracle Database",
    "demo_notebooks/data_storage/databases/postgres": "PostgreSQL",
    "demo_notebooks/data_storage/databases/sqllite": "SQLite",
    "demo_notebooks/data_storage/databases/utilities": "Database utilities",
    "demo_notebooks/data_storage/filesystems": "File systems",
    "demo_notebooks/data_storage/filesystems/box": "Box",
    "demo_notebooks/data_storage/filesystems/dropbox": "Dropbox",
    "demo_notebooks/data_storage/filesystems/fsspec": "fsspec",
    "demo_notebooks/data_storage/filesystems/utilities": "File system utilities",
    "demo_notebooks/data_structures": "Data structures",
    "demo_notebooks/data_visualization": "Data visualization",
    "demo_notebooks/data_visualization/bokeh": "Bokeh",
    "demo_notebooks/data_visualization/branca": "Branca",
    "demo_notebooks/data_visualization/contourpy": "ContourPy",
    "demo_notebooks/data_visualization/folium": "Folium",
    "demo_notebooks/data_visualization/geoplot": "geoplot",
    "demo_notebooks/data_visualization/graphviz": "Graphviz",
    "demo_notebooks/data_visualization/holoviews": "HoloViz",
    "demo_notebooks/data_visualization/holoviews/geoviews": "GeoViews",
    "demo_notebooks/data_visualization/holoviews/holoviews": "HoloViews",
    "demo_notebooks/data_visualization/holoviews/hvplot": "hvPlot",
    "demo_notebooks/data_visualization/holoviews/panel": "Panel",
    "demo_notebooks/data_visualization/mplleaflet": "mplleaflet",
    "demo_notebooks/data_visualization/plotly": "Plotly",
    "demo_notebooks/data_visualization/railroad": "Railroad diagrams",
    "demo_notebooks/data_visualization/seaborn": "seaborn",
    "demo_notebooks/geospatial": "Geospatial",
    "demo_notebooks/geospatial/google": "Google Earth Engine and Google Cloud",
    "demo_notebooks/geospatial/microsoft": "Microsoft",
    "demo_notebooks/geospatial/microsoft/azure": "Azure",
    "demo_notebooks/geospatial/pysal": "PySAL",
    "demo_notebooks/networking": "Networking",
    "demo_notebooks/networking/utilities": "HTTP and URL utilities",
    "demo_notebooks/security": "Security",
    "demo_notebooks/testing": "Testing",
    "demo_notebooks/testing/faker": "Faker",
    "demo_notebooks/testing/hypothesis": "Hypothesis",
    "demo_notebooks/utilities": "Utilities",
    "demo_notebooks/web_servers": "Web servers",
    "ray_patterns": "Ray patterns",
    "ray_patterns/core": "Ray Core",
    "ray_patterns/data": "Ray Data",
    "ray_patterns/serve": "Ray Serve",
    "ray_patterns/train": "Ray Train",
    "ray_patterns/tune": "Ray Tune",
    "ray_patterns/cross": "Across libraries",
}
# Sub-folder order where it is not alphabetical (the order of ray_patterns/README.md).
FOLDER_ORDER = {"ray_patterns": ("core", "data", "serve", "train", "tune", "cross")}
# Folders shown as their own top-level tab instead of inside their parent.
PROMOTED = ("demo_notebooks/case_studies",)
DESCRIPTION_LIMIT = 160

# The palette, type and gradient the website shares with Eugo's other documentation sites: a copy
# of their brand.css under this repository's two licence lines. BRAND_SHA256 is the sha256 of the
# file without those two lines; when the shared stylesheet changes, copy it again (keeping the
# licence lines), check it with --check-brand PATH, and update the digest here.
BRAND_CSS = ROOT / "site" / "src" / "styles" / "brand.css"
BRAND_SHA256 = "18892f2b603cfdb58912003af37c0a7eaa3d2c2829e6fc8cf0ddb4fcc7035bc2"
BRAND_HEADER_LINES = 2


# --------------------------------------------------------------------------------------------
# Text helpers


def strip_markdown(text: str) -> str:
    """Markdown or HTML reduced to plain text on one line."""
    text = re.sub(r"!\[([^\]]*)\]\([^)]*\)", r"\1", text)          # images
    text = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", text)           # links
    text = re.sub(r"<(https?://[^>]+)>", r"\1", text)              # autolinks
    text = re.sub(r"<[^>]+>", "", text)                            # HTML tags
    text = text.replace("`", "")
    dunders: list[str] = []
    text = re.sub(r"(?<![\w*])__[A-Za-z][A-Za-z0-9]*__(?![\w*])",
                  lambda m: dunders.append(m.group(0)) or f"\x00{len(dunders) - 1}\x00", text)  # __name__ is code
    text = re.sub(r"(\*\*|__)(?=\S)(.+?)(?<=\S)\1", r"\2", text)   # bold
    text = re.sub(r"(?<![\w*])\*(?=\S)(.+?)(?<=\S)\*(?![\w*])", r"\1", text)  # italics
    text = re.sub(r"(?<!\w)_(?=\S)(.+?)(?<=\S)_(?!\w)", r"\1", text)
    text = re.sub(r"\x00(\d+)\x00", lambda m: dunders[int(m.group(1))], text)
    # straight double quotes would end an HTML attribute early in the page's meta tags
    text = re.sub(r'"([^"\n]*)"', "\u201c\\1\u201d", text).replace('"', "\u201d")
    return re.sub(r"\s+", " ", text).strip()


def trim(text: str, limit: int = DESCRIPTION_LIMIT) -> str:
    """Whole sentences that fit in ``limit`` characters, or a word-boundary cut with an ellipsis."""
    if len(text) <= limit:
        return text
    kept = ""
    for sentence in re.split(r"(?<=[.!?])\s+(?=[A-Z0-9])", text):
        candidate = f"{kept} {sentence}".strip()
        if len(candidate) > limit:
            break
        kept = candidate
    if len(kept) >= 60:
        return kept
    cut = text[: limit - 1].rsplit(" ", 1)[0].rstrip(",;:-–— ")
    return cut + "…"


def md_escape(text: str) -> str:
    """Text safe to use as Markdown link text."""
    return re.sub(r"([\[\]\\*_`<>])", r"\\\1", text)


def yaml_str(text: str) -> str:
    """A YAML double-quoted scalar (JSON strings are valid YAML)."""
    return json.dumps(text, ensure_ascii=False)


def front_matter(**fields: str) -> str:
    lines = ["---"] + [f"{key}: {yaml_str(value)}" for key, value in fields.items()] + ["---", ""]
    return "\n".join(lines) + "\n"


def read_front_matter(path: Path) -> dict[str, str]:
    """The ``key: value`` lines of a Markdown file's front matter (plain or double-quoted)."""
    text = path.read_text(encoding="utf-8")
    match = re.match(r"---\n(.*?)\n---\n", text, re.S)
    fields = {}
    for line in (match.group(1).splitlines() if match else []):
        key, sep, value = line.partition(":")
        if sep and re.fullmatch(r"[A-Za-z_]+", key.strip()):
            value = value.strip()
            fields[key.strip()] = json.loads(value) if value.startswith('"') else value
    return fields


def github_url(repo_path: str, is_dir: bool = False) -> str:
    return f"{REPO_URL}/{'tree' if is_dir else 'blob'}/{BRANCH}/{repo_path}".rstrip("/")


def page_url(src: str) -> str:
    """Absolute site URL of a page, given its path inside the docs folder."""
    path = re.sub(r"(^|/)index\.md$", r"\1", src)
    path = re.sub(r"\.(md|ipynb)$", "/", path)
    return SITE_URL + path


# --------------------------------------------------------------------------------------------
# Notebooks and folders


@dataclass
class Notebook:
    path: str          # repository path, e.g. "ray_patterns/core/01_map-reduce.ipynb"
    title: str
    description: str


@dataclass
class Folder:
    path: str
    title: str
    intro: str = ""
    notebooks: list[Notebook] = field(default_factory=list)
    children: list["Folder"] = field(default_factory=list)

    def count(self) -> int:
        return len(self.notebooks) + sum(child.count() for child in self.children)


def cell_text(cell: dict) -> str:
    source = cell.get("source", "")
    return source if isinstance(source, str) else "".join(source)


def notebook_info(path: Path) -> Notebook:
    """Title (the first Markdown H1) and a one-line description (the first prose paragraph)."""
    rel = path.relative_to(ROOT).as_posix()
    nb = json.loads(path.read_text(encoding="utf-8"))
    markdown = [cell_text(c) for c in nb.get("cells", []) if c.get("cell_type") == "markdown"]
    title = ""
    for text in markdown:
        match = re.search(r"^#\s+(.+?)\s*#*\s*$", text, re.M)
        if match:
            title = strip_markdown(match.group(1))
            break
    title = title or path.stem.replace("_", " ")

    description = ""
    label = None  # the "Label:" line that introduced the list we are in, if any
    first = markdown[0] if markdown else ""
    for block in re.split(r"\n\s*\n", first):
        lines = [line.strip() for line in block.strip().splitlines() if line.strip()]
        if not lines or lines[0].startswith("#"):
            continue
        head = re.fullmatch(r"\**([A-Za-z][A-Za-z ]{0,30}?)\**\s*:?\**", lines[0])
        if head and (len(lines) == 1 or lines[1].startswith(("-", "*"))):
            label, lines = head.group(1).strip().lower(), lines[1:]
            if not lines:
                continue
        if lines[0].startswith(("-", "* ", "+ ")):
            if label == "description":
                description = strip_markdown(re.sub(r"^[-*+]\s+", "", lines[0]))
                break
            continue
        if lines[0].startswith(("|", ">", "```", "<", "![")) or re.match(r"\d+\.\s", lines[0]):
            continue
        description = strip_markdown(" ".join(lines))
        break
    if not description:
        description = f"{title} examples in Python"
    if rel.startswith("demo_notebooks/") and not rel.startswith(PROMOTED):
        # Library demos open with a short blurb ("Work with geospatial data"): name the library
        # and say what the page is.
        if title.lower() not in description.lower():
            description = f"{title}: {description}"
        if len(description) < 100:
            description = f"{description.rstrip('.')}. A Python demo notebook from ganymede."
    return Notebook(rel, title, trim(description))


def folder_title(rel: str) -> str:
    return FOLDER_TITLES.get(rel) or rel.rsplit("/", 1)[-1].replace("_", " ").capitalize()


def readme_intros() -> dict[str, str]:
    """One-line folder descriptions taken from the folder READMEs."""
    intros = {}
    demo = (ROOT / "demo_notebooks" / "README.md").read_text(encoding="utf-8")
    for row in re.findall(r"^\|(.+)\|\s*$", demo, re.M):
        cells = [c.strip() for c in row.split("|")]
        folders = re.findall(r"\]\((\w+)/\)", cells[0])
        if len(cells) >= 2 and len(folders) == 1:
            intros[f"demo_notebooks/{folders[0]}"] = strip_markdown(cells[1])
    ray = (ROOT / "ray_patterns" / "README.md").read_text(encoding="utf-8")
    headings = {folder_title(f"ray_patterns/{name}"): name for name in FOLDER_ORDER["ray_patterns"]}
    for heading, body in re.findall(r"^## (.+)\n\n(.+?)\n\n", ray, re.M):
        if heading in headings:
            intros[f"ray_patterns/{headings[heading]}"] = strip_markdown(body)
    return intros


def scan_folder(rel: str, intros: dict[str, str]) -> Folder | None:
    """The folder tree under ``rel``, or None when it holds no notebooks."""
    path = ROOT / rel
    folder = Folder(rel, folder_title(rel), intros.get(rel, ""))
    folder.notebooks = [notebook_info(p) for p in sorted(path.glob("*.ipynb"))]
    names = sorted(p.name for p in path.iterdir() if p.is_dir() and not p.name.startswith("."))
    order = FOLDER_ORDER.get(rel)
    if order:
        names.sort(key=lambda n: order.index(n) if n in order else len(order))
    for name in names:
        child = scan_folder(f"{rel}/{name}", intros)
        if child:
            folder.children.append(child)
    return folder if folder.count() else None


def walk(folder: Folder):
    yield folder
    for child in folder.children:
        yield from walk(child)


# --------------------------------------------------------------------------------------------
# Generated pages


def rewrite_readme_links(text: str, folder_rel: str, sections: set[str]) -> str:
    """Point a README's relative links at site pages, or at GitHub for files the site lacks."""

    def fix(match: re.Match) -> str:
        prefix, target, suffix = match.group(1), match.group(2), match.group(3)
        if re.match(r"[a-z][a-z0-9+.-]*:|#|/", target):
            return match.group(0)
        resolved = (Path(folder_rel) / target).as_posix()
        parts = []
        for part in resolved.split("/"):
            if part == "..":
                if parts:
                    parts.pop()
            elif part not in ("", "."):
                parts.append(part)
        resolved = "/".join(parts)
        if resolved in sections:
            new = f"{target.rstrip('/')}/index.md"
        elif resolved.startswith(NOTEBOOK_ROOTS) and (ROOT / resolved).is_file() and (
            resolved.endswith(".ipynb") or "/images/" in resolved
        ):
            new = target
        else:
            new = github_url(resolved, (ROOT / resolved).is_dir())
        return f"{prefix}{new}{suffix}"

    text = re.sub(r"(\]\()([^)\s]+)(\))", fix, text)
    return re.sub(r'((?:href|src)=")([^"]+)(")', fix, text)


def folder_page(folder: Folder, sections: set[str]) -> str:
    readme = ROOT / folder.path / "README.md"
    titles = [nb.title for nb in folder.notebooks] + [c.title for c in folder.children]
    if readme.is_file():
        body = rewrite_readme_links(readme.read_text(encoding="utf-8"), folder.path, sections)
        first = re.split(r"\n\s*\n", body.split("\n", 1)[1].strip())[0]
        description = trim(strip_markdown(first))
    else:
        if len(folder.notebooks) == 1 and not folder.children:
            description = trim(f"{folder.title} in ganymede, one notebook: {folder.notebooks[0].description}")
        elif folder.intro:
            description = trim(f"{folder.title} in ganymede. {folder.intro} Notebooks: {', '.join(titles)}.")
        else:
            description = trim(f"{folder.title} notebooks in ganymede: {', '.join(titles)}. "
                               "Python demos with code you can download and run.")
        n = folder.count()
        body = f"# {folder.title}\n\n"
        if folder.intro:
            body += f"{folder.intro}\n\n"
        body += (
            f"This section holds {n} notebook{'s' if n != 1 else ''} from "
            f"[`{folder.path}/`]({github_url(folder.path, True)}) in the ganymede repository. "
            "Each page shows the notebook as committed; use the buttons at the top right of a page "
            "to download the `.ipynb` or open it on GitHub.\n"
        )
    page_title = folder.title
    if any(t.casefold() == folder.title.casefold() for t in titles):
        # A section named like a notebook inside it (dask/ holding dask.ipynb) would share that
        # page's <title>; search engines and assistants may merge the two or show the wrong one.
        page_title = f"{folder.title} section"
    lines = [front_matter(title=page_title, description=description), body.rstrip(), ""]
    if folder.notebooks and not readme.is_file():
        lines += ["## Notebooks", ""]
        for nb in folder.notebooks:
            name = Path(nb.path).name
            lines.append(f"- [{md_escape(nb.title)}]({name}): {nb.description}")
        lines.append("")
    if folder.children:
        lines += ["## Sections", ""]
        for child in folder.children:
            n = child.count()
            name = child.path.rsplit("/", 1)[-1]
            text = f"- [{md_escape(child.title)}]({name}/index.md): {n} notebook{'s' if n != 1 else ''}"
            if child.intro:
                text += f". {child.intro}"
            lines.append(text)
        lines.append("")
    return "\n".join(lines)


def gallery_block() -> str:
    """The home page gallery: one card per GALLERY notebook, linking to its page."""
    cards = ['<div class="grid cards gallery" markdown>', ""]
    for rel, caption in GALLERY:
        png = GALLERY_DIR / f"{Path(rel).stem}.png"
        if not png.is_file():
            sys.exit(f"missing {png.relative_to(ROOT)}: run python tools/build_site.py --refresh-gallery")
        width, height = struct.unpack(">II", png.read_bytes()[16:24])
        title = notebook_info(ROOT / rel).title
        image = f"assets/gallery/{png.name}"
        cards += [
            f"-   [![{md_escape(caption)}]({image}){{ width=\"{width}\" height=\"{height}\" "
            f"loading=lazy }}]({rel})",
            "",
            f"    **[{md_escape(title)}]({rel})**",
            "",
            f"    {caption}",
            "",
        ]
    return "\n".join(cards + ["</div>"])


def package_list() -> str:
    """The package lists of the package-tests page."""
    python = sorted((p.stem for p in (ROOT / "package_tests" / "python").glob("*.ipynb")), key=str.lower)
    native = sorted(
        (p.parent.name for p in (ROOT / "package_tests" / "native").glob("*/testspec.json")), key=str.lower
    )
    out = [
        "## Python packages",
        "",
        f"{len(python):,} notebooks, one per package, in "
        f"[`package_tests/python/`]({github_url('package_tests/python', True)}). "
        "Each name links to its notebook on GitHub.",
        "",
        '<div class="pkg-list" markdown>',
        "",
    ]
    out += [f"- [{md_escape(name)}]({github_url(f'package_tests/python/{name}.ipynb')})" for name in python]
    out += [
        "",
        "</div>",
        "",
        "## Native libraries and tools",
        "",
        f"{len(native):,} packages in [`package_tests/native/`]({github_url('package_tests/native', True)}), "
        "each a folder with a `testspec.json` and its test programs. Each name links to its folder on GitHub.",
        "",
        '<div class="pkg-list" markdown>',
        "",
    ]
    out += [f"- [{md_escape(name)}]({github_url(f'package_tests/native/{name}', True)})" for name in native]
    out += ["", "</div>", ""]
    return "\n".join(out)


# --------------------------------------------------------------------------------------------
# Assembling the site source


@dataclass
class Page:
    src: str             # path inside the docs folder
    title: str
    description: str
    github: str          # where the page's source lives on GitHub
    kind: str            # "doc", "section" or "notebook"


def nav_yaml(items: list, indent: int = 2) -> list[str]:
    """YAML lines for a nav list of ("title", src) pages and ("title", [items]) sections."""
    pad = " " * indent
    lines = []
    for title, value in items:
        if isinstance(value, list):
            lines.append(f"{pad}- {yaml_str(title)}:")
            lines += nav_yaml(value, indent + 4)
        elif title is None:
            lines.append(f"{pad}- {yaml_str(value)}")
        else:
            lines.append(f"{pad}- {yaml_str(title)}: {yaml_str(value)}")
    return lines


def folder_nav(folder: Folder, pages: dict[str, Page]) -> list:
    """Nav items of a folder: its index page, its notebooks, then its sub-folders."""
    items: list = [(None, f"{folder.path}/index.md")]
    items += [(pages[nb.path].title, nb.path) for nb in folder.notebooks]
    items += [(child.title, folder_nav(child, pages)) for child in folder.children if child.path not in PROMOTED]
    return items


def llms_txt(nav: list, pages: dict[str, Page]) -> str:
    """llms.txt (https://llmstxt.org/): a summary, then one link line per page, grouped as in the nav."""

    def flatten(items: list) -> list[str]:
        srcs = []
        for _title, value in items:
            srcs += flatten(value) if isinstance(value, list) else [value]
        return srcs

    def line(src: str) -> str:
        page = pages[src]
        return f"- [{md_escape(page.title)}]({page_url(src)}): {page.description}"

    out = [
        "# ganymede",
        "",
        "> Open Jupyter notebooks from Eugo, Inc. (https://eugo.io): case studies on public NOAA and "
        "Copernicus data (GOES-16, the National Water Model, Sentinel-2), Ray patterns for Ray Core, "
        "Data, Serve, Train and Tune, short demos of popular Python libraries, and smoke tests for "
        "Python packages and native libraries. BSD 3-Clause License.",
        "",
        f"Source repository: {REPO_URL}. Every notebook page below shows the notebook as committed, "
        "with its outputs where the repository keeps them, and links to the .ipynb file. "
        "Citable archive on Zenodo: https://doi.org/10.5281/zenodo.24117548 (all versions).",
        "",
        "## Overview",
        "",
    ]
    singles = [value for _title, value in nav if not isinstance(value, list)]
    out += [line(src) for src in singles] + [""]
    for title, value in nav:
        if isinstance(value, list):
            out += [f"## {title}", ""] + [line(src) for src in flatten(value)] + [""]
    return "\n".join(out)


COPIED_ROOTS = ("docs", "overrides", "demo_notebooks", "ray_patterns")


def check_tracked() -> None:
    """Stop when the folders copied into the site hold files git does not track (untracked or ignored), so a local
    build publishes exactly what the repository holds. Python byte-code caches are allowed."""
    try:
        res = subprocess.run(["git", "-C", str(ROOT), "status", "--porcelain=v1", "-z", "--untracked-files=all",
                              "--ignored", "--", *COPIED_ROOTS], capture_output=True, check=True)
    except (OSError, subprocess.CalledProcessError):
        print("build_site: not a git checkout; cannot check for untracked files", file=sys.stderr)
        return
    stray = []
    for entry in res.stdout.decode("utf-8", "replace").split("\0"):
        if entry[:2] in ("??", "!!"):
            path = entry[3:]
            if "__pycache__/" in path or path.endswith(".pyc"):
                continue
            stray.append(path)
    if stray:
        sys.exit("build_site: files git does not track would be copied into the site (move them, or pass "
                 f"--allow-untracked): {', '.join(stray[:10])}" + (" ..." if len(stray) > 10 else ""))


def assemble(out: Path, allow_untracked: bool = False) -> None:
    if not allow_untracked:
        check_tracked()
    out = out.resolve()
    if out == ROOT or ROOT.is_relative_to(out):
        sys.exit(f"--out must not be the repository or one of its parents: {out}")
    if out.exists():
        if any(out.iterdir()) and not (out / MARKER).is_file():
            sys.exit(f"{out} is not empty and was not created by this script; choose another --out")
        shutil.rmtree(out)
    docs = out / "docs"
    out.mkdir(parents=True)
    (out / MARKER).write_text("Created by tools/build_site.py; safe to delete.\n", encoding="utf-8")
    shutil.copytree(ROOT / "docs", docs)
    shutil.copytree(ROOT / "overrides", out / "overrides")

    intros = readme_intros()
    roots = [f for f in (scan_folder(r, intros) for r in NOTEBOOK_ROOTS) if f]
    folders = [f for root in roots for f in walk(root)]
    sections = {f.path for f in folders}
    pages: dict[str, Page] = {}

    for folder in folders:
        for nb in folder.notebooks:
            target = docs / nb.path
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / nb.path, target)
            pages[nb.path] = Page(nb.path, nb.title, nb.description, github_url(nb.path), "notebook")
        src = f"{folder.path}/index.md"
        text = folder_page(folder, sections)
        (docs / src).parent.mkdir(parents=True, exist_ok=True)
        (docs / src).write_text(text, encoding="utf-8")
        fm = read_front_matter(docs / src)
        readme = ROOT / folder.path / "README.md"
        source = github_url(readme.relative_to(ROOT).as_posix()) if readme.is_file() else github_url(folder.path, True)
        pages[src] = Page(src, fm["title"], fm["description"], source, "section")
    shutil.copytree(ROOT / "ray_patterns" / "images", docs / "ray_patterns" / "images")

    # Hand-written pages, with the generated parts filled in.
    index = docs / "index.md"
    index.write_text(index.read_text(encoding="utf-8").replace("<!-- gallery -->", gallery_block()), encoding="utf-8")
    tests = docs / "package_tests.md"
    tests.write_text(tests.read_text(encoding="utf-8").replace("<!-- package-list -->", package_list()), encoding="utf-8")
    for md in sorted(docs.glob("*.md")):
        fm = read_front_matter(md)
        if "description" not in fm:
            sys.exit(f"docs/{md.name} needs a description in its front matter")
        title = fm.get("nav_title") or fm.get("title") or md.stem
        pages[md.name] = Page(md.name, title, fm["description"], github_url(f"docs/{md.name}"), "doc")

    by_path = {f.path: f for f in folders}
    nav = [("Home", "index.md")]
    for rel in PROMOTED:
        if rel in by_path:
            nav.append((by_path[rel].title, folder_nav(by_path[rel], pages)))
    for root in sorted(roots, key=lambda f: f.path != "ray_patterns"):
        nav.append((root.title, folder_nav(root, pages)))
    nav += [(pages["package_tests.md"].title, "package_tests.md"), ("FAQ", "faq.md")]
    pages["index.md"].title = "ganymede"

    listed = {value for _t, value in _flat_nav(nav)}
    missing = sorted(set(pages) - listed)
    if missing:
        sys.exit(f"pages missing from the nav: {missing}")

    (docs / "llms.txt").write_text(llms_txt(nav, pages), encoding="utf-8")
    config = (ROOT / "mkdocs.yml").read_text(encoding="utf-8")
    if re.search(r"^nav:", config, re.M):
        sys.exit("mkdocs.yml must not define nav: this script generates it")
    config = config.rstrip() + "\n\n# Generated by tools/build_site.py\nnav:\n" + "\n".join(nav_yaml(nav)) + "\n"
    (out / "mkdocs.yml").write_text(config, encoding="utf-8")
    meta = {src: {"title": p.title, "description": p.description, "github": p.github, "kind": p.kind}
            for src, p in pages.items()}
    (out / "pages.json").write_text(json.dumps({"pages": meta}, indent=1, ensure_ascii=False), encoding="utf-8")
    notebooks = sum(p.kind == "notebook" for p in pages.values())
    print(f"assembled {len(pages)} pages ({notebooks} notebooks, {len(folders)} section indexes) in {out}")
    print(f"next: mkdocs build --strict -f {out / 'mkdocs.yml'}")


def _flat_nav(items: list):
    for title, value in items:
        if isinstance(value, list):
            yield from _flat_nav(value)
        else:
            yield title, value


# --------------------------------------------------------------------------------------------
# Notebook pages for the Starlight site: each notebook becomes one Markdown page


class SiteError(Exception):
    """A page or a link the site cannot be built with; the message names it and the fix."""


# The site's address: every page and file lives under BASE_PATH on NEW_SITE.
NEW_SITE = "https://eugo.io"
BASE_PATH = "/docs/ganymede/"
FENCE_LINE = re.compile(r"^ {0,3}(`{3,}|~{3,})")
H1_LINE = re.compile(r"^ {0,3}#[ \t]+\S")
MD_LINK = re.compile(r"(!?\[(?:[^\[\]]|\[[^\[\]]*\])*\]\()(<[^>]*>|[^()\s]+)((?:\s+\"[^\"]*\")?\))")
HTML_LINK = re.compile(r"""(\b(?:href|src)=")([^"]+)(")""")
SCHEME_OR_ANCHOR = re.compile(r"^(?:[a-zA-Z][a-zA-Z0-9+.-]*:|#|/)")
# The kinds of output a notebook page shows, in the order one is chosen from a bundle that
# holds several representations of the same result: a table, then a figure, then text.
OUTPUT_PREFERENCE = ("text/html", "image/png", "text/plain")


def site_path(repo_path: str) -> str:
    """The address of a page under the site's base, given the repository file it is made from:
    a notebook, a folder's README.md, a hand-written page of docs/ or a folder."""
    path = re.sub(r"(^|/)README\.md$", r"\1", repo_path)
    path = re.sub(r"^docs/", "", path)
    path = re.sub(r"(^|/)index\.md$", r"\1", path)
    path = re.sub(r"\.(md|ipynb)$", "", path).strip("/")
    return f"{BASE_PATH}{path}/" if path else BASE_PATH


def normalize(path: str) -> str | None:
    """A repository path with ``.`` and ``..`` resolved, or None when it leaves the repository."""
    parts: list[str] = []
    for part in path.split("/"):
        if part == "..":
            if not parts:
                return None
            parts.pop()
        elif part not in ("", "."):
            parts.append(part)
    return "/".join(parts)


def resolve(from_repo_path: str, target: str, sections: set[str]) -> str:
    """Where a relative link written in a repository file points on the site.

    A notebook, a folder's README.md, a section folder or a hand-written page of docs/ is a page
    of the site; a Ray pattern image is a file of the site; any other file or folder of the
    repository is linked on GitHub. A target that names nothing in the repository is refused.
    Absolute addresses, anchors and paths from the root are left as they are."""
    if SCHEME_OR_ANCHOR.match(target):
        return target
    path, hash_, anchor = target.partition("#")
    path, query, rest = path.partition("?")
    if not path:
        return target
    base = posixpath.dirname(from_repo_path)
    resolved = normalize(posixpath.join(base, path))
    if resolved is None or not (ROOT / resolved).exists():
        raise SiteError(f"{from_repo_path}: the link {target!r} names nothing in the repository")
    suffix = (query + rest) + (hash_ + anchor)
    folder = posixpath.dirname(resolved)
    if resolved.endswith(".ipynb") and resolved.startswith(NOTEBOOK_ROOTS):
        return site_path(resolved) + suffix
    if resolved.endswith("README.md") and folder in sections:
        return site_path(resolved) + suffix
    if resolved in sections:
        return site_path(resolved + "/") + suffix
    if re.fullmatch(r"docs/[^/]+\.md", resolved):
        return site_path(resolved) + suffix
    if resolved.startswith("ray_patterns/images/"):
        return BASE_PATH + resolved + suffix
    return github_url(resolved, (ROOT / resolved).is_dir()) + suffix


def rewrite_links(text: str, from_repo_path: str, sections: set[str]) -> str:
    """The Markdown links, images and HTML ``href``/``src`` of a text pointed at the site,
    outside fenced code and code spans."""

    def link(match: re.Match) -> str:
        target = match.group(2)
        bare = target[1:-1] if target.startswith("<") else target
        return f"{match.group(1)}{resolve(from_repo_path, bare, sections)}{match.group(3)}"

    def html(match: re.Match) -> str:
        return f"{match.group(1)}{resolve(from_repo_path, match.group(2), sections)}{match.group(3)}"

    out, fence = [], None
    for line in text.split("\n"):
        opened = FENCE_LINE.match(line)
        if fence:
            if opened and opened.group(1)[0] == fence[0] and len(opened.group(1)) >= len(fence):
                fence = None
            out.append(line)
            continue
        if opened:
            fence = opened.group(1)
            out.append(line)
            continue
        pieces = re.split(r"(`+[^`]*?`+)", line)
        for i in range(0, len(pieces), 2):
            pieces[i] = HTML_LINK.sub(html, MD_LINK.sub(link, pieces[i]))
        out.append("".join(pieces))
    return "\n".join(out)


def drop_first_h1(text: str) -> tuple[str, bool]:
    """A Markdown text without its first level-one heading outside fenced code (the site
    shows the page's title above the text), and whether there was one."""
    lines, fence = text.split("\n"), None
    for index, line in enumerate(lines):
        opened = FENCE_LINE.match(line)
        if fence:
            if opened and opened.group(1)[0] == fence[0] and len(opened.group(1)) >= len(fence):
                fence = None
            continue
        if opened:
            fence = opened.group(1)
            continue
        if H1_LINE.match(line):
            return "\n".join(lines[:index] + lines[index + 1:]).strip("\n"), True
    return text, False


ATX_HEADING = re.compile(r"^( {0,3}#{1,6}[ \t]+)(.*?)([ \t]+#+)?[ \t]*$")


def mkdocs_slug(text: str) -> str:
    """The id MkDocs gave a heading (Python-Markdown's toc): runs of spaces and hyphens become
    one hyphen."""
    text = re.sub(r"[^\w\s-]", "", text).strip().lower()
    return re.sub(r"[-\s]+", "-", text)


def site_slug(text: str) -> str:
    """The id Astro gives a heading (github-slugger): each space becomes a hyphen, nothing is
    collapsed."""
    return re.sub(r" ", "-", re.sub(r"[^\w\- ]", "", text.lower()))


class LegacyIds:
    """Keeps the heading ids of the MkDocs site working: a heading whose id on this site differs
    from the one MkDocs gave it carries the old id too, on an empty span at its start. Repeated
    headings are numbered as each generator numbers them (MkDocs ``_1``, Astro ``-1``)."""

    def __init__(self, title: str):
        self.old: dict[str, int] = {}
        self.new: dict[str, int] = {}
        self._take(self.old, mkdocs_slug(strip_markdown(title)), "_")  # the title kept its id on MkDocs

    @staticmethod
    def _take(seen: dict[str, int], slug: str, joiner: str) -> str:
        if slug in seen:
            seen[slug] += 1
            return f"{slug}{joiner}{seen[slug]}"
        seen[slug] = 0
        return slug

    def heading(self, line: str) -> str:
        match = ATX_HEADING.match(line)
        if not match:
            return line
        text = strip_markdown(match.group(2)).replace("\u201c", "").replace("\u201d", "")
        old = self._take(self.old, mkdocs_slug(text), "_")
        new = self._take(self.new, site_slug(text), "-")
        if not old or old == new:
            return line
        return f'{match.group(1)}<span id="{old}"></span>{match.group(2)}{match.group(3) or ""}'

    def text(self, text: str) -> str:
        """Every heading of a Markdown text outside fenced code, with its old id where it changed."""
        out, fence = [], None
        for line in text.split("\n"):
            opened = FENCE_LINE.match(line)
            if fence:
                if opened and opened.group(1)[0] == fence[0] and len(opened.group(1)) >= len(fence):
                    fence = None
                out.append(line)
            elif opened:
                fence = opened.group(1)
                out.append(line)
            else:
                out.append(self.heading(line))
        return "\n".join(out)


def fenced(text: str, info: str) -> str:
    """A fenced code block holding ``text`` exactly, its fence longer than any run of
    backticks inside it."""
    longest = max((len(run) for run in re.findall(r"`+", text)), default=0)
    fence = "`" * max(3, longest + 1)
    return f"{fence}{info}\n{text.rstrip(chr(10))}\n{fence}"


def joined(value) -> str:
    return value if isinstance(value, str) else "".join(value)


def ray_images() -> dict[str, str]:
    """The sha256 of each image in ray_patterns/images/, with the image's repository path."""
    return {hashlib.sha256(p.read_bytes()).hexdigest(): p.relative_to(ROOT).as_posix()
            for p in sorted((ROOT / "ray_patterns" / "images").glob("*.png"))}


def html_output(html: str) -> str:
    """A table a notebook printed, as one HTML block: blank lines would end the block early
    and turn the rest into Markdown (the indented CSS of a pandas table into a code block)."""
    body = "\n".join(line for line in html.strip("\n").split("\n") if line.strip())
    return f'<div class="gn-output gn-html">\n{body}\n</div>'


def png_output(data: bytes, metadata: dict, src: str, alt: str) -> str:
    """A figure a notebook drew: the image at the size the notebook shows it (the width its
    metadata gives, else the image's own), on a white backing (ganymede.css)."""
    width, height = struct.unpack(">II", dict(png_chunks(data))[b"IHDR"][:8])
    shown = metadata.get("width")
    if isinstance(shown, (int, float)) and 0 < shown < width:
        width, height = int(shown), max(1, round(height * shown / width))
    return (f'<figure class="gn-output"><img src="{src}" width="{width}" height="{height}" '
            f'alt="{alt}" loading="lazy" decoding="async"></figure>')


@dataclass
class NotebookPage:
    repo_path: str
    front: dict          # front matter: title, description, slug, github, download, kind, legacyTitleId
    body: str            # Markdown
    files: dict[str, bytes] = field(default_factory=dict)  # path under the base -> bytes


def notebook_page(repo_path: str, sections: set[str], images: dict[str, str]) -> NotebookPage:
    """One notebook as a Markdown page: its Markdown cells (without the first level-one heading,
    which becomes the page's title, links pointed at the site, attachments as files), its code
    cells as Python blocks and its outputs (text as blocks titled Output, figures as images,
    tables as HTML), plus the files the page serves: its figures and the notebook itself.
    An output the page cannot show (an error, standard error, another kind of data) is refused."""
    path = ROOT / repo_path
    raw = path.read_bytes()
    nb = json.loads(raw)
    info = notebook_info(path)
    slug = repo_path[: -len(".ipynb")]
    page = site_path(repo_path)
    stem = Path(repo_path).stem
    result = NotebookPage(repo_path, {
        "title": info.title,
        "description": info.description,
        "slug": slug,
        "github": github_url(repo_path),
        "download": f"{page}{stem}.ipynb",
        "kind": "notebook",
        "legacyTitleId": mkdocs_slug(strip_markdown(info.title).replace("\u201c", "").replace("\u201d", "")),
    }, "")
    result.files[f"{slug}/{stem}.ipynb"] = raw
    blocks: list[str] = []
    legacy = LegacyIds(info.title)
    dropped = False
    code_number = 0
    for index, cell in enumerate(nb.get("cells", [])):
        kind = cell.get("cell_type")
        source = joined(cell.get("source", ""))
        if kind == "markdown":
            if not dropped:
                source, dropped = drop_first_h1(source)
            attachments = cell.get("attachments") or {}

            def attachment(match: re.Match) -> str:
                name = match.group(1)
                bundle = attachments.get(name)
                if not bundle or "image/png" not in bundle:
                    raise SiteError(f"{repo_path}: cell {index + 1} names attachment:{name}, which it does not hold as PNG")
                data = base64.b64decode(joined(bundle["image/png"]))
                known = images.get(hashlib.sha256(data).hexdigest())
                if known:
                    return BASE_PATH + known
                result.files[f"{slug}/{name}"] = png_bytes([c for c in png_chunks(data) if c[0] in KEEP_CHUNKS])
                return f"{page}{name}"

            source = re.sub(r"attachment:([^)\s\"]+)", attachment, source)
            source = legacy.text(rewrite_links(source, repo_path, sections))
            if source.strip():
                blocks.append(source.strip("\n"))
        elif kind == "code":
            if source.strip():
                code_number += 1
                blocks.append(fenced(source, "python"))
            stream: list[str] = []

            def flush() -> None:
                if stream:
                    blocks.append(fenced("".join(stream), 'text title="Output"'))
                    stream.clear()

            for k, output in enumerate(cell.get("outputs", [])):
                otype = output.get("output_type")
                if otype == "stream" and output.get("name") == "stdout":
                    stream.append(joined(output.get("text", "")))
                    continue
                flush()
                if otype not in ("execute_result", "display_data"):
                    what = f"{otype} ({output.get('name')})" if otype == "stream" else otype
                    raise SiteError(f"{repo_path}: cell {index + 1} has an output the site cannot show: {what}")
                data = output.get("data", {})
                chosen = next((mime for mime in OUTPUT_PREFERENCE if mime in data), None)
                if chosen is None:
                    raise SiteError(f"{repo_path}: cell {index + 1} has an output of a kind the site cannot "
                                    f"show: {', '.join(sorted(data)) or 'no data'}")
                if chosen == "text/html":
                    blocks.append(html_output(joined(data[chosen])))
                elif chosen == "image/png":
                    raw_png = base64.b64decode(joined(data[chosen]))
                    clean = png_bytes([c for c in png_chunks(raw_png) if c[0] in KEEP_CHUNKS])
                    name = f"output-{index + 1}-{k + 1}.png"
                    result.files[f"{slug}/{name}"] = clean
                    meta = (output.get("metadata") or {}).get("image/png") or {}
                    blocks.append(png_output(clean, meta, f"{page}{name}",
                                             f"Figure drawn by code cell {code_number}"))
                else:
                    blocks.append(fenced(joined(data[chosen]), 'text title="Output"'))
            flush()
        elif kind == "raw":
            raise SiteError(f"{repo_path}: cell {index + 1} is a raw cell, which the site cannot show")
    if not dropped:
        raise SiteError(f"{repo_path}: no level-one heading to take the page's title from")
    result.body = "\n\n".join(blocks) + "\n"
    return result


def page_text(front: dict, body: str) -> str:
    """A page as Starlight reads it: JSON front matter (JSON is YAML), then the body."""
    return f"---\n{json.dumps(front, ensure_ascii=False)}\n---\n\n{body}"


# --------------------------------------------------------------------------------------------
# Gallery images (PNG, standard library only)

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
# Chunks kept in gallery images: image data and colour information only. Text (tEXt, iTXt,
# zTXt), EXIF (eXIf), time stamps and every other ancillary chunk are dropped.
KEEP_CHUNKS = {b"IHDR", b"PLTE", b"tRNS", b"IDAT", b"IEND", b"sRGB", b"gAMA", b"cHRM", b"pHYs"}
CHANNELS = {0: 1, 2: 3, 3: 1, 4: 2, 6: 4}


def png_chunks(data: bytes) -> list[tuple[bytes, bytes]]:
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("not a PNG file")
    chunks, pos = [], len(PNG_SIGNATURE)
    while pos < len(data):
        (length,) = struct.unpack(">I", data[pos:pos + 4])
        kind, body = data[pos + 4:pos + 8], data[pos + 8:pos + 8 + length]
        chunks.append((kind, body))
        pos += 12 + length
        if kind == b"IEND":
            break
    return chunks


def png_bytes(chunks: list[tuple[bytes, bytes]]) -> bytes:
    out = [PNG_SIGNATURE]
    for kind, body in chunks:
        out.append(struct.pack(">I", len(body)) + kind + body + struct.pack(">I", zlib.crc32(kind + body)))
    return b"".join(out)


def png_decode(chunks: list[tuple[bytes, bytes]]) -> tuple[int, int, int, list[bytearray]]:
    """Width, height, channels and unfiltered rows of an 8-bit, non-interlaced PNG.

    Palette images come back as RGB (or RGBA when they have a tRNS chunk)."""
    header = dict(chunks)[b"IHDR"]
    width, height, depth, color, _comp, _filt, interlace = struct.unpack(">IIBBBBB", header)
    if depth != 8 or interlace or color not in CHANNELS:
        raise ValueError(f"unsupported PNG (bit depth {depth}, colour type {color}, interlace {interlace})")
    ch = CHANNELS[color]
    raw = zlib.decompress(b"".join(body for kind, body in chunks if kind == b"IDAT"))
    stride = width * ch
    rows, prev = [], bytearray(stride)
    for y in range(height):
        start = y * (stride + 1)
        kind, row = raw[start], bytearray(raw[start + 1:start + 1 + stride])
        if kind == 1:
            for i in range(ch, stride):
                row[i] = (row[i] + row[i - ch]) & 0xFF
        elif kind == 2:
            for i in range(stride):
                row[i] = (row[i] + prev[i]) & 0xFF
        elif kind == 3:
            for i in range(stride):
                left = row[i - ch] if i >= ch else 0
                row[i] = (row[i] + ((left + prev[i]) >> 1)) & 0xFF
        elif kind == 4:
            for i in range(stride):
                a = row[i - ch] if i >= ch else 0
                b = prev[i]
                c = prev[i - ch] if i >= ch else 0
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                row[i] = (row[i] + pred) & 0xFF
        elif kind != 0:
            raise ValueError(f"bad PNG filter type {kind}")
        rows.append(row)
        prev = row
    if color == 3:
        found = dict(chunks)
        palette = found[b"PLTE"]
        alpha = found.get(b"tRNS")
        ch = 4 if alpha else 3
        lut = []
        for i in range(len(palette) // 3):
            entry = palette[3 * i:3 * i + 3]
            if alpha:
                entry += bytes([alpha[i] if i < len(alpha) else 255])
            lut.append(entry)
        rows = [bytearray(b"".join(lut[v] for v in row)) for row in rows]
    return width, height, ch, rows


def resample_weights(src: int, dst: int) -> list[list[tuple[int, float]]]:
    """Area-average weights: which source pixels cover each destination pixel, and how much."""
    scale = src / dst
    weights = []
    for j in range(dst):
        lo, hi = j * scale, (j + 1) * scale
        row = []
        for i in range(int(lo), min(src, int(hi) + 1)):
            overlap = min(hi, i + 1) - max(lo, i)
            if overlap > 1e-9:
                row.append((i, overlap / scale))
        weights.append(row)
    return weights


def png_resize(width: int, height: int, ch: int, rows: list[bytearray], new_w: int, new_h: int) -> list[bytearray]:
    wx, wy = resample_weights(width, new_w), resample_weights(height, new_h)
    horizontal = []
    for row in rows:
        planes = [row[c::ch] for c in range(ch)]
        out = [0.0] * (new_w * ch)
        for j, ws in enumerate(wx):
            for c in range(ch):
                plane = planes[c]
                out[j * ch + c] = sum(plane[i] * w for i, w in ws)
        horizontal.append(out)
    result = []
    for ws in wy:
        acc = [0.0] * (new_w * ch)
        for i, w in ws:
            src = horizontal[i]
            for k in range(len(acc)):
                acc[k] += src[k] * w
        result.append(bytearray(min(255, max(0, int(v + 0.5))) for v in acc))
    return result


def png_encode(width: int, height: int, ch: int, rows: list[bytearray], extra: list[tuple[bytes, bytes]]) -> bytes:
    """An 8-bit PNG; each row uses whichever of the None, Sub and Up filters looks smallest."""
    color = {1: 0, 2: 4, 3: 2, 4: 6}[ch]
    raw = bytearray()
    prev = bytearray(width * ch)
    for row in rows:
        sub = bytearray(row[i] if i < ch else (row[i] - row[i - ch]) & 0xFF for i in range(len(row)))
        up = bytearray((row[i] - prev[i]) & 0xFF for i in range(len(row)))
        candidates = [(0, row), (1, sub), (2, up)]
        kind, data = min(candidates, key=lambda c: sum(b if b < 128 else 256 - b for b in c[1]))
        raw.append(kind)
        raw += data
        prev = row
    header = struct.pack(">IIBBBBB", width, height, 8, color, 0, 0, 0)
    chunks = [(b"IHDR", header)] + extra + [(b"IDAT", zlib.compress(bytes(raw), 9)), (b"IEND", b"")]
    return png_bytes(chunks)


def gallery_png(data: bytes, max_width: int = GALLERY_MAX_WIDTH) -> bytes:
    """A PNG with only the KEEP_CHUNKS chunks, scaled down to at most ``max_width`` pixels wide."""
    chunks = [(kind, body) for kind, body in png_chunks(data) if kind in KEEP_CHUNKS]
    width, height = struct.unpack(">II", dict(chunks)[b"IHDR"][:8])
    if width <= max_width:
        return png_bytes(chunks)
    w, h, ch, rows = png_decode(chunks)
    new_w, new_h = max_width, max(1, round(height * max_width / width))
    colour = [(kind, body) for kind, body in chunks if kind in (b"sRGB", b"gAMA", b"cHRM")]
    return png_encode(new_w, new_h, ch, png_resize(w, h, ch, rows, new_w, new_h), colour)


def first_png(path: Path) -> bytes:
    nb = json.loads(path.read_text(encoding="utf-8"))
    for cell in nb.get("cells", []):
        for output in cell.get("outputs", []):
            value = output.get("data", {}).get("image/png")
            if value:
                return base64.b64decode(value if isinstance(value, str) else "".join(value))
    raise ValueError(f"{path.relative_to(ROOT)} has no PNG output")


def refresh_gallery() -> None:
    GALLERY_DIR.mkdir(parents=True, exist_ok=True)
    wanted = set()
    for rel, _caption in GALLERY:
        target = GALLERY_DIR / f"{Path(rel).stem}.png"
        data = gallery_png(first_png(ROOT / rel))
        target.write_bytes(data)
        wanted.add(target.name)
        width, height = struct.unpack(">II", data[16:24])
        print(f"{target.relative_to(ROOT)}: {width}x{height}, {len(data) / 1024:.0f} KiB")
    for stale in GALLERY_DIR.glob("*.png"):
        if stale.name not in wanted:
            stale.unlink()
            print(f"removed {stale.relative_to(ROOT)}")
    total = sum(p.stat().st_size for p in GALLERY_DIR.glob("*.png"))
    print(f"gallery: {len(wanted)} images, {total / 1024 / 1024:.2f} MiB")


def brand_body(path: Path) -> bytes:
    """A brand stylesheet without its licence lines: the part every copy shares."""
    return b"".join(path.read_bytes().splitlines(keepends=True)[BRAND_HEADER_LINES:])


def brand_refusal(against: Path | None = None) -> str | None:
    """Why site/src/styles/brand.css is not the shared stylesheet, or None when it is: its body
    must have the digest BRAND_SHA256 and, when ``against`` names another copy (the stylesheet of
    another Eugo documentation site), the same body as that copy."""
    if not BRAND_CSS.is_file():
        return f"{BRAND_CSS.relative_to(ROOT)} is missing"
    body = brand_body(BRAND_CSS)
    digest = hashlib.sha256(body).hexdigest()
    if digest != BRAND_SHA256:
        return (f"{BRAND_CSS.relative_to(ROOT)} has changed (sha256 {digest} without its licence lines, "
                f"not {BRAND_SHA256}): copy the shared stylesheet again, or update BRAND_SHA256")
    if against is not None:
        other = hashlib.sha256(brand_body(against)).hexdigest()
        if other != digest:
            return (f"{against} differs from {BRAND_CSS.relative_to(ROOT)} (sha256 {other}, not {digest}): "
                    "copy it under this repository's licence lines and update BRAND_SHA256")
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--out", type=Path, help="folder to assemble the MkDocs project in")
    parser.add_argument("--refresh-gallery", action="store_true",
                        help="rewrite docs/assets/gallery/*.png from the notebooks' figures")
    parser.add_argument("--allow-untracked", action="store_true",
                        help="build even when the copied folders hold files git does not track")
    parser.add_argument("--check-brand", nargs="?", const=True, metavar="PATH",
                        help="check site/src/styles/brand.css against its digest and, given PATH, "
                             "against another copy of the shared stylesheet")
    args = parser.parse_args()
    if args.check_brand:
        cause = brand_refusal(None if args.check_brand is True else Path(args.check_brand))
        if cause:
            print(f"build_site: {cause}", file=sys.stderr)
            return 1
        print(f"brand.css: sha256 {BRAND_SHA256} without its licence lines")
        if not args.out and not args.refresh_gallery:
            return 0
    if not args.out and not args.refresh_gallery:
        parser.error("give --out DIR, --refresh-gallery, --check-brand, or a combination")
    if args.refresh_gallery:
        refresh_gallery()
    if args.out:
        assemble(args.out, args.allow_untracked)
    return 0


if __name__ == "__main__":
    sys.exit(main())

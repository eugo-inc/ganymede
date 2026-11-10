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
import json
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
        "with its outputs where the repository keeps them, and links to the .ipynb file.",
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


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--out", type=Path, help="folder to assemble the MkDocs project in")
    parser.add_argument("--refresh-gallery", action="store_true",
                        help="rewrite docs/assets/gallery/*.png from the notebooks' figures")
    parser.add_argument("--allow-untracked", action="store_true",
                        help="build even when the copied folders hold files git does not track")
    args = parser.parse_args()
    if not args.out and not args.refresh_gallery:
        parser.error("give --out DIR, --refresh-gallery, or both")
    if args.refresh_gallery:
        refresh_gallery()
    if args.out:
        assemble(args.out, args.allow_untracked)
    return 0


if __name__ == "__main__":
    sys.exit(main())

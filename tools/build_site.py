#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo, Inc.
"""Build the ganymede website (Astro and Starlight) from the repository's notebooks and pages.

    python3 tools/build_site.py build       # prepare, npm ci when needed, astro build, finish
    python3 tools/build_site.py prepare     # write the pages, settings and files Astro reads
    python3 tools/build_site.py finish      # Markdown copies, sitemap.xml, then check
    python3 tools/build_site.py check       # check site/dist/ again
    python3 tools/build_site.py redirects   # the redirect site for the old address, in build/redirects/
    python3 tools/build_site.py clean       # remove everything the build writes (run it before a push)
    python3 tools/build_site.py check-brand [--against PATH]
    python3 tools/build_site.py refresh-gallery

``prepare`` checks the sources first (no file git does not track in the source folders, the
shared brand stylesheet, the share image and the marks) and writes nothing until every check has
passed. It then writes:

- ``site/src/content/docs/``: one page per notebook of ``demo_notebooks/`` and ``ray_patterns/``
  (converted to Markdown at its repository path, so addresses mirror repository paths), an index
  page per notebook folder (its README.md, or a generated list), and the pages of ``docs/``;
- ``site/src/generated/site.json``: the title, description, sidebar and share-image tags;
- ``site/.public/``: ``site/public/`` plus the gallery, the Ray pattern images, each notebook's
  figures and ``.ipynb`` download, and ``llms.txt``;
- ``site/src/generated/markdown/``: each page's Markdown copy, which ``finish`` puts beside the
  page in ``site/dist/``.

``check`` refuses a broken link, anchor or asset, anything fetched from another host, an id used
twice, a page without one canonical link or its Markdown copy, a file the host could not serve,
and any address or heading id of the old MkDocs site (``site/legacy-urls.txt``) that is gone.
Notebooks are never executed. ``refresh-gallery`` takes the first PNG figure of each notebook in
``GALLERY``, drops every PNG chunk except image data and colour information (so no text or EXIF
chunks remain), scales it to at most ``GALLERY_MAX_WIDTH`` pixels wide and writes it to
``docs/assets/gallery/<notebook stem>.png``.

Only the standard library is used; the site itself needs Node.js 22 and the packages that
``site/package-lock.json`` pins.
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
REPO_URL = "https://github.com/eugo-inc/ganymede"
BRANCH = "main"
NOTEBOOK_ROOTS = ("demo_notebooks", "ray_patterns")

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
# licence lines), check it with check-brand --against PATH, and update the digest here.
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
        '<div class="gn-pkg-list">',
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
        '<div class="gn-pkg-list">',
        "",
    ]
    out += [f"- [{md_escape(name)}]({github_url(f'package_tests/native/{name}', True)})" for name in native]
    out += ["", "</div>", ""]
    return "\n".join(out)


# --------------------------------------------------------------------------------------------
# Notebook pages: each notebook becomes one Markdown page


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
# The Starlight site: every page, the sidebar, the head of each page and the served files

SITE = ROOT / "site"
CONTENT = SITE / "src" / "content" / "docs"
GENERATED = SITE / "src" / "generated"
COPIES = GENERATED / "markdown"         # each page's Markdown copy, moved into dist/ by finish
PUBLIC = SITE / "public"
SERVED = SITE / ".public"               # what Astro serves as it is: public/ plus the generated files
DIST = SITE / "dist"
ASTRO_CACHE = SITE / ".astro"
MANIFEST = SITE / "legacy-urls.txt"
REDIRECTS = ROOT / "build" / "redirects"
NEW_ROOT = NEW_SITE + BASE_PATH         # https://eugo.io/docs/ganymede/
OLD_ROOT = "https://eugo-inc.github.io/ganymede/"   # the address of the MkDocs site this one replaces
OLD_BASE = "/ganymede/"
TITLE = "ganymede"
SUMMARY = ("Open Jupyter notebooks from Eugo: case studies on public NOAA and Copernicus data, Ray "
           "patterns, demos of popular Python libraries, and smoke tests for Python packages and "
           "native libraries.")
DOI_ALL = "https://doi.org/10.5281/zenodo.24117548"
LICENSE_URL = "https://opensource.org/license/bsd-3-clause"
PUBLISHER = "Eugo, Inc."
KEYWORDS = ("high-performance computing", "HPC", "Ray", "Ray Core", "Ray Data", "Ray Serve",
            "Ray Train", "Ray Tune", "Jupyter notebooks", "scientific Python", "geospatial Python",
            "xarray", "NetCDF", "GOES-16", "National Water Model", "Sentinel-2", "STAC",
            "Cloud-Optimized GeoTIFF", "package smoke tests")
SCRIPT_SAFE = str.maketrans({"<": "\\u003c", ">": "\\u003e", "&": "\\u0026"})
HAND_PAGES = ("index.md", "faq.md", "package_tests.md")
# Generated folders under site/ that git ignores: the tracked-files check skips them.
GENERATED_DIRS = ("site/node_modules/", "site/.astro/", "site/dist/", "site/.public/",
                  "site/src/content/docs/", "site/src/generated/")
SITE_ROOTS = ("docs", "demo_notebooks", "ray_patterns", "site")
# Heading ids of the MkDocs site that no page of this site carries, each with the reason.
ANCHOR_EXCEPTIONS: dict[str, dict[str, str]] = {}


@dataclass
class SitePage:
    id: str                  # the page's path under the content folder, without .md: "faq", "ray_patterns/core"
    source: str              # the repository file the page is made from
    title: str
    description: str
    kind: str                # "home", "doc", "section" or "notebook"
    github: str
    body: str                # Markdown, as Astro reads it
    label: str = ""          # the sidebar's name for the page, when it is not the title
    front: dict = field(default_factory=dict)    # further front matter: download, legacyTitleId, hero
    files: dict[str, bytes] = field(default_factory=dict)
    mdx: bool = False
    trail: list[tuple[str, str]] = field(default_factory=list)  # breadcrumbs: (name, address)
    copy_body: str = ""      # Markdown for the page's copy, when the body is MDX

    @property
    def path(self) -> str:
        """The page's address under the host: /docs/ganymede/faq/."""
        return BASE_PATH if self.id == "index" else f"{BASE_PATH}{self.id}/"

    @property
    def url(self) -> str:
        return NEW_SITE + self.path


def hand_link(target: str) -> str:
    """A link written in a page of docs/, as a link relative to docs/: the pages of docs/ were
    written for a site whose root mirrors the repository (a folder's page is ``folder/index.md``,
    ``llms.txt`` is the site's own list)."""
    if SCHEME_OR_ANCHOR.match(target):
        return target
    path, hash_, anchor = target.partition("#")
    suffix = hash_ + anchor
    if path == "llms.txt":
        return f"{BASE_PATH}llms.txt{suffix}"
    if (ROOT / "docs" / path).is_file():
        return target
    if path.endswith("index.md"):
        folder = path[: -len("index.md")].rstrip("/")
        readme = f"{folder}/README.md"
        return f"../{readme if (ROOT / readme).is_file() else folder + '/'}{suffix}"
    return f"../{target}"


def rewrite_hand_links(text: str, sections: set[str], repo_path: str) -> str:
    """The links of a page of docs/ pointed at the site."""
    def link(match: re.Match) -> str:
        target = match.group(2)
        bare = target[1:-1] if target.startswith("<") else target
        return f"{match.group(1)}{hand_link(bare)}{match.group(3)}"

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
            pieces[i] = MD_LINK.sub(link, pieces[i])
        out.append("".join(pieces))
    return rewrite_links("\n".join(out), repo_path, sections)


def take_first_h1(text: str) -> tuple[str, str]:
    """A Markdown text without its first level-one heading, and that heading's text ("" when
    there is none)."""
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
            heading = re.sub(r"^ {0,3}#[ \t]+|[ \t]+#+[ \t]*$", "", line).strip()
            return "\n".join(lines[:index] + lines[index + 1:]).strip("\n"), heading
    return text, ""


def old_id(heading: str) -> str:
    return mkdocs_slug(strip_markdown(heading).replace("“", "").replace("”", ""))


def body_text(text: str, heading: str) -> str:
    """A page's Markdown with the old heading ids kept where Astro gives new ones."""
    return LegacyIds(heading).text(text).strip("\n") + "\n"


def sections_list(folder: Folder) -> str:
    """The list of a folder's sub-folders, each with its number of notebooks ("" when it has none)."""
    if not folder.children:
        return ""
    lines = ["## Sections", ""]
    for child in folder.children:
        count = child.count()
        item = (f"- [{md_escape(child.title)}]({site_path(child.path + '/')}): "
                f"{count} notebook{'s' if count != 1 else ''}")
        if child.intro:
            item += f". {child.intro}"
        lines.append(item)
    return "\n".join(lines) + "\n"


def site_folder_page(folder: Folder, sections: set[str]) -> SitePage:
    """A folder's page: its README.md when it has one, else a generated list of its notebooks
    and sub-folders."""
    readme = ROOT / folder.path / "README.md"
    titles = [nb.title for nb in folder.notebooks] + [c.title for c in folder.children]
    if readme.is_file():
        source = f"{folder.path}/README.md"
        text, heading = take_first_h1(readme.read_text(encoding="utf-8"))
        first = re.split(r"\n\s*\n", text.strip())[0]
        description = trim(strip_markdown(first))
        body = rewrite_links(text, source, sections).rstrip("\n") + "\n\n" + sections_list(folder)
        github = github_url(source)
    else:
        source = f"{folder.path}/"
        heading = folder.title
        if len(folder.notebooks) == 1 and not folder.children:
            description = trim(f"{folder.title} in ganymede, one notebook: {folder.notebooks[0].description}")
        elif folder.intro:
            description = trim(f"{folder.title} in ganymede. {folder.intro} Notebooks: {', '.join(titles)}.")
        else:
            description = trim(f"{folder.title} notebooks in ganymede: {', '.join(titles)}. "
                               "Python demos with code you can download and run.")
        n = folder.count()
        lines = []
        if folder.intro:
            lines += [folder.intro, ""]
        lines += [
            f"This section holds {n} notebook{'s' if n != 1 else ''} from "
            f"[`{folder.path}/`]({github_url(folder.path, True)}) in the ganymede repository. "
            "Each page shows the notebook as committed; the links under a page's title download "
            "the `.ipynb` or open it on GitHub.",
            "",
        ]
        if folder.notebooks:
            lines += ["## Notebooks", ""]
            for nb in folder.notebooks:
                lines.append(f"- [{md_escape(nb.title)}]({site_path(nb.path)}): {nb.description}")
            lines.append("")
        body = "\n".join(lines) + "\n" + sections_list(folder)
        github = github_url(folder.path, True)
    title = folder.title
    if any(t.casefold() == folder.title.casefold() for t in titles):
        # A section named like a notebook inside it (dask/ holding dask.ipynb) would share that
        # page's <title>; search engines and assistants may merge the two or show the wrong one.
        title = f"{folder.title} section"
    return SitePage(folder.path, source, title, description, "section", github, body_text(body, heading),
                    label="Overview", front={"legacyTitleId": old_id(heading)})


def site_notebook_page(nb: Notebook, sections: set[str], images: dict[str, str]) -> SitePage:
    converted = notebook_page(nb.path, sections, images)
    front = {key: converted.front[key] for key in ("download", "legacyTitleId")}
    return SitePage(converted.front["slug"], nb.path, converted.front["title"], converted.front["description"],
                    "notebook", converted.front["github"], converted.body, front=front, files=converted.files)


def html_escape(text: str) -> str:
    return text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")


def hand_page(name: str, sections: set[str]) -> SitePage:
    """A page of docs/: its front matter's title and description, its first level-one heading
    dropped (the site shows the title), its links pointed at the site."""
    path = ROOT / "docs" / name
    fm = read_front_matter(path)
    if not fm.get("description"):
        raise SiteError(f"docs/{name} needs a description in its front matter")
    text = re.sub(r"\A---\n.*?\n---\n", "", path.read_text(encoding="utf-8"), flags=re.S)
    text, heading = take_first_h1(text)
    if name == "package_tests.md":
        text = text.replace("<!-- package-list -->", package_list())
    body = rewrite_hand_links(text, sections, f"docs/{name}")
    page_id = name[: -len(".md")]
    kind = "home" if name == "index.md" else "doc"
    title = fm.get("title") or heading
    page = SitePage(page_id, f"docs/{name}", title, fm["description"], kind, github_url(f"docs/{name}"),
                    body_text(body, heading), label=fm.get("nav_title", ""))
    if kind == "home":
        page.copy_body = home_markdown(page.body)
        # A page with a hero has no page title of its own: the old id of the home page's heading
        # is kept on an empty element at the start of its text.
        page.body = f'<span id="{old_id(heading)}"></span>\n\n{home_mdx(page.body)}'
        page.mdx = True
        page.front["template"] = "splash"
        page.front["hero"] = home_hero(page.description)
    else:
        page.front["legacyTitleId"] = old_id(heading)
    return page


# The home page: a hero (a question as its title, the description as its tagline, the mark, three
# buttons), then its text, in which three marked regions become Starlight's components:
#   <!-- linkcards --> ... <!-- /linkcards -->  each line "- [Title](target): text" is a link card;
#   <!-- gallery -->                            the gallery of figures (GALLERY);
#   <!-- steps --> ... <!-- /steps -->          an ordered list shown as numbered steps.
HERO_TITLE = "Need a Python notebook that already runs?"
HERO_ACTIONS = (
    ("Case studies", "demo_notebooks/case_studies/", "primary", "right-arrow"),
    ("Ray patterns", "ray_patterns/", "secondary", None),
    ("Source", REPO_URL, "minimal", "external"),
)
HOME_IMPORTS = ("import { CardGrid, LinkCard, Steps } from '@astrojs/starlight/components';\n"
                "import GalleryCard from '../../components/GalleryCard.astro';")
CARD_LINE = re.compile(r"^- \[([^\]]+)\]\(([^)\s]+)\):\s*(.+)$")
HOME_MARKER = re.compile(r"^<!--\s*(/?)(linkcards|steps|gallery)\s*-->$")


def home_hero(description: str) -> dict:
    actions = []
    for text, target, variant, icon in HERO_ACTIONS:
        action = {"text": text, "link": target if "://" in target else BASE_PATH + target, "variant": variant}
        if icon:
            action["icon"] = icon
        actions.append(action)
    return {"title": HERO_TITLE, "tagline": description,
            "image": {"alt": "", "light": "../../assets/mark-light.svg", "dark": "../../assets/mark-dark.svg"},
            "actions": actions}


def gallery_cards() -> list[tuple[str, str, int, int, str, str]]:
    """Each gallery card: the notebook's page, the figure's address, its size, title and caption."""
    cards = []
    for rel, caption in GALLERY:
        png = GALLERY_DIR / f"{Path(rel).stem}.png"
        if not png.is_file():
            raise SiteError(f"missing {png.relative_to(ROOT)}: run python3 tools/build_site.py refresh-gallery")
        width, height = struct.unpack(">II", png.read_bytes()[16:24])
        cards.append((site_path(rel), f"{BASE_PATH}assets/gallery/{png.name}", width, height,
                      notebook_info(ROOT / rel).title, caption))
    return cards


def outside_fences(text: str):
    """Each line of a text, with whether it is inside fenced code (its fences included)."""
    fence = None
    for line in text.split("\n"):
        opened = FENCE_LINE.match(line)
        if fence:
            if opened and opened.group(1)[0] == fence[0] and len(opened.group(1)) >= len(fence):
                fence = None
            yield line, True
        elif opened:
            fence = opened.group(1)
            yield line, True
        else:
            yield line, False


def home_markdown(body: str) -> str:
    """The home page's text as plain Markdown: the markers left out, the gallery as a list."""
    out = []
    for line, fenced in outside_fences(body):
        marker = None if fenced else HOME_MARKER.match(line.strip())
        if marker and marker.group(2) == "gallery":
            out += [f"- [{md_escape(title)}]({href}): {caption}" for href, _src, _w, _h, title, caption in gallery_cards()]
        elif not marker:
            out.append(line)
    return "\n".join(out)


def mdx_prose(line: str) -> str:
    """A line of prose made safe for MDX: braces outside code spans are escaped."""
    pieces = re.split(r"(`+[^`]*?`+)", line)
    for i in range(0, len(pieces), 2):
        pieces[i] = re.sub(r"([{}])", r"\\\1", pieces[i])
    return "".join(pieces)


def jsx(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def home_mdx(body: str) -> str:
    """The home page's text as MDX, its marked regions as Starlight's components. A region left
    open, or closed without being opened, is refused."""
    out, region = [], None
    for line, fenced in outside_fences(body):
        if fenced:
            out.append(line)
            continue
        marker = HOME_MARKER.match(line.strip())
        if marker:
            closing, name = marker.groups()
            if name == "gallery":
                out.append('<div class="gn-gallery not-content">')
                out += [f"  <GalleryCard href={jsx(h)} src={jsx(s)} width={{{w}}} height={{{ht}}} "
                        f"title={jsx(t)} caption={jsx(c)} />" for h, s, w, ht, t, c in gallery_cards()]
                out.append("</div>")
            elif closing:
                if region != name:
                    raise SiteError(f"docs/index.md: <!-- /{name} --> closes no open {name} region")
                out += ["", "</CardGrid>" if name == "linkcards" else "</Steps>", ""]
                region = None
            else:
                region = name
                out += ["", "<CardGrid>" if name == "linkcards" else "<Steps>", ""]
            continue
        if line.strip().startswith("<!--"):
            continue
        card = CARD_LINE.match(line) if region == "linkcards" else None
        if card:
            title, href, text = card.groups()
            out.append(f"  <LinkCard title={jsx(strip_markdown(title))} href={jsx(href)} "
                       f"description={jsx(strip_markdown(text))} />")
            continue
        out.append(mdx_prose(line))
    if region:
        raise SiteError(f"docs/index.md: <!-- {region} --> is never closed")
    text = re.sub(r"\n{3,}", "\n\n", "\n".join(out)).strip("\n")
    return f"{HOME_IMPORTS}\n\n{text}\n"


@dataclass
class Site:
    pages: dict[str, SitePage]          # by id
    sidebar: list[dict]
    roots: list[Folder]


def collect_site() -> Site:
    """Every page of the site, with its sidebar and its breadcrumbs. Nothing is written."""
    intros = readme_intros()
    roots = [f for f in (scan_folder(r, intros) for r in NOTEBOOK_ROOTS) if f]
    folders = [f for root in roots for f in walk(root)]
    sections = {f.path for f in folders}
    images = ray_images()
    pages: dict[str, SitePage] = {}
    for folder in folders:
        page = site_folder_page(folder, sections)
        pages[page.id] = page
        for nb in folder.notebooks:
            page = site_notebook_page(nb, sections, images)
            pages[page.id] = page
    for name in HAND_PAGES:
        page = hand_page(name, sections)
        pages[page.id] = page

    by_path = {f.path: f for f in folders}

    def group(folder: Folder, collapsed: bool) -> dict:
        items: list[dict] = [{"label": "Overview", "slug": folder.path}]
        items += [{"label": pages[nb.path[: -len(".ipynb")]].title, "slug": nb.path[: -len(".ipynb")]}
                  for nb in folder.notebooks]
        items += [group(child, True) for child in folder.children if child.path not in PROMOTED]
        return {"label": folder.title, "collapsed": collapsed, "items": items}

    sidebar: list[dict] = [{"label": "Home", "link": "/"}]
    for rel in PROMOTED:
        if rel in by_path:
            sidebar.append(group(by_path[rel], False))
    for root in sorted(roots, key=lambda f: f.path != "ray_patterns"):
        sidebar.append(group(root, True))
    sidebar += [{"label": pages["package_tests"].title, "slug": "package_tests"},
                {"label": pages["faq"].label or pages["faq"].title, "slug": "faq"}]

    listed = set()

    def gather(items: list[dict]) -> None:
        for item in items:
            if "items" in item:
                gather(item["items"])
            elif "slug" in item:
                listed.add(item["slug"])
    gather(sidebar)
    listed.add("index")
    missing = sorted(set(pages) - listed)
    if missing:
        raise SiteError(f"pages missing from the sidebar: {missing}")

    # Breadcrumbs follow the sidebar: the home page, the folders above the page (a promoted
    # folder starts its own trail), then the page.
    home = pages["index"]
    for page in pages.values():
        trail = [(TITLE, home.url)]
        if page.kind in ("section", "notebook"):
            parts = page.id.split("/")
            upto = len(parts) - (1 if page.kind == "section" else 1)
            chain = ["/".join(parts[:i]) for i in range(1, upto + 1)]
            chain = [c for c in chain if c in by_path]
            for promoted in PROMOTED:
                if promoted in chain:
                    chain = chain[chain.index(promoted):]
            trail += [(by_path[c].title, pages[c].url) for c in chain if c != page.id]
        if page.kind != "home":
            trail.append((page.label if page.kind == "doc" and page.label else page.title, page.url))
        page.trail = trail
    return Site(pages, sidebar, roots)


def share_head() -> list[dict]:
    """The tags every page names the share image with: its address, size and alt text."""
    tool = share_tool()
    image = f"{NEW_ROOT}assets/social-preview.png"
    tags = [("property", "og:image", image), ("property", "og:image:width", str(tool.WIDTH)),
            ("property", "og:image:height", str(tool.HEIGHT)), ("property", "og:image:alt", tool.ALT),
            ("name", "twitter:image", image), ("name", "twitter:image:alt", tool.ALT)]
    return [{"tag": "meta", "attrs": {key: name, "content": value}} for key, name, value in tags]


def share_tool():
    import importlib.util
    spec = importlib.util.spec_from_file_location("share_image", ROOT / "tools" / "share_image.py")
    module = importlib.util.module_from_spec(spec)
    sys.modules.setdefault("share_image", module)
    spec.loader.exec_module(module)
    return module


def structured_data(page: SitePage, home: SitePage) -> dict:
    """What a page says of itself in schema.org terms: the home page describes the notebooks'
    source code and the organisation that publishes them; a notebook's page is a technical
    article whose notebook can be downloaded; every page carries its breadcrumbs."""
    organization = {"@type": "Organization", "@id": f"{NEW_SITE}/#organization", "name": PUBLISHER,
                    "url": f"{NEW_SITE}/"}
    source = f"{home.url}#source"
    publisher = {"@id": organization["@id"]}
    nodes: list[dict] = []
    if page.kind == "home":
        nodes = [{
            "@type": "SoftwareSourceCode", "@id": source, "name": TITLE, "description": SUMMARY,
            "url": home.url, "codeRepository": REPO_URL, "identifier": DOI_ALL, "license": LICENSE_URL,
            "programmingLanguage": "Python", "author": publisher, "publisher": publisher,
            "image": f"{NEW_ROOT}assets/social-preview.png", "keywords": list(KEYWORDS),
        }, organization]
    elif page.kind == "notebook":
        nodes = [{
            "@type": "TechArticle", "@id": f"{page.url}#article", "headline": page.title,
            "description": page.description, "url": page.url, "mainEntityOfPage": page.url, "inLanguage": "en",
            "author": publisher, "publisher": publisher, "isPartOf": {"@id": source}, "sameAs": page.github,
            "encoding": {"@type": "MediaObject", "encodingFormat": "application/x-ipynb+json",
                         "contentUrl": NEW_SITE + page.front["download"]},
        }]
    else:
        nodes = [{
            "@type": "TechArticle", "@id": f"{page.url}#article", "headline": page.title,
            "description": page.description, "url": page.url, "inLanguage": "en",
            "publisher": publisher, "isPartOf": {"@id": source},
        }]
    if page.kind != "home":
        nodes.append(organization)
    trail = {"@type": "BreadcrumbList", "@id": f"{page.url}#breadcrumb", "itemListElement": [
        {"@type": "ListItem", "position": n, "name": name, "item": item}
        for n, (name, item) in enumerate(page.trail, start=1)]}
    return {"@context": "https://schema.org", "@graph": [*nodes, trail]}


def page_head(page: SitePage, home: SitePage) -> list[dict]:
    data = json.dumps(structured_data(page, home), ensure_ascii=False, separators=(",", ":"))
    return [
        {"tag": "link", "attrs": {"rel": "alternate", "type": "text/markdown", "href": f"{page.path}index.md"}},
        {"tag": "script", "attrs": {"type": "application/ld+json"}, "content": data.translate(SCRIPT_SAFE)},
    ]


def site_page_text(page: SitePage, home: SitePage) -> str:
    front = {"title": page.title, "description": page.description}
    if page.kind == "notebook":
        front["slug"] = page.id
    front["github"] = page.github
    front["kind"] = "doc" if page.kind == "home" else page.kind
    front.update(page.front)
    front["head"] = page_head(page, home)
    return page_text(front, page.body)


def absolute(text: str) -> str:
    """A text whose site addresses (/docs/ganymede/...) are written in full, for its Markdown copy."""
    text = re.sub(r"(\]\()" + re.escape(BASE_PATH), r"\1" + NEW_ROOT, text)
    return re.sub(r'((?:href|src)=")' + re.escape(BASE_PATH), r"\1" + NEW_ROOT, text)


def markdown_copy(page: SitePage) -> str:
    """A page as Markdown, for readers that ask for it: where the list of pages is, the title,
    the description and the page's address, then the text with every address in full."""
    lines = [f"> Every page of the ganymede website is listed in {NEW_ROOT}llms.txt.", "",
             f"# {page.title}", "", f"> {page.description}", "", f"Web page: {page.url}"]
    if page.kind == "notebook":
        lines.append(f"Notebook: {NEW_SITE}{page.front['download']} (source: {page.github})")
    body = re.sub(r'<span id="[^"]*"></span>', "", page.copy_body or page.body)
    return "\n".join(lines) + "\n\n" + absolute(body).strip("\n") + "\n"


def site_llms(site: Site) -> str:
    """llms.txt (https://llmstxt.org/): a summary, then one line per page in the sidebar's order,
    each linking the page's Markdown copy."""
    pages = site.pages

    def line(page_id: str) -> str:
        page = pages[page_id]
        return f"- [{md_escape(page.title)}]({page.url}index.md): {page.description}"

    def ids(items: list[dict]) -> list[str]:
        out = []
        for item in items:
            if "items" in item:
                out += ids(item["items"])
            elif "slug" in item:
                out.append(item["slug"])
        return out

    out = [
        "# ganymede", "",
        "> Open Jupyter notebooks from Eugo, Inc. (https://eugo.io): case studies on public NOAA and "
        "Copernicus data (GOES-16, the National Water Model, Sentinel-2), Ray patterns for Ray Core, "
        "Data, Serve, Train and Tune, short demos of popular Python libraries, and smoke tests for "
        "Python packages and native libraries. BSD 3-Clause License.", "",
        f"Source repository: {REPO_URL}. Every notebook page below shows the notebook as committed, "
        "with its outputs where the repository keeps them, and links to the .ipynb file. Each link "
        "below is the Markdown copy of a page; the web page is the same address without index.md. "
        f"Citable archive on Zenodo: {DOI_ALL} (all versions).", "",
        "## Overview", "", line("index"),
    ]
    singles = [item["slug"] for item in site.sidebar if "slug" in item]
    out += [line(s) for s in singles] + [""]
    for item in site.sidebar:
        if "items" in item:
            out += [f"## {item['label']}", ""] + [line(s) for s in ids(item["items"])] + [""]
    return "\n".join(out).rstrip("\n") + "\n"


def check_tracked_site() -> None:
    """Stop when the folders the site is made from hold files git does not track (untracked or
    ignored), other than the generated folders, so a local build publishes what the repository
    holds. Python byte-code caches are allowed."""
    try:
        res = subprocess.run(["git", "-C", str(ROOT), "status", "--porcelain=v1", "-z", "--untracked-files=all",
                              "--ignored", "--", *SITE_ROOTS], capture_output=True, check=True)
    except (OSError, subprocess.CalledProcessError):
        print("build_site: not a git checkout; cannot check for untracked files", file=sys.stderr)
        return
    stray = []
    for entry in res.stdout.decode("utf-8", "replace").split("\0"):
        if entry[:2] in ("??", "!!"):
            path = entry[3:]
            if "__pycache__/" in path or path.endswith(".pyc") or path.startswith(GENERATED_DIRS):
                continue
            stray.append(path)
    if stray:
        raise SiteError("files git does not track would go into the site (move them, or pass --allow-untracked): "
                        + ", ".join(stray[:10]) + (" ..." if len(stray) > 10 else ""))


def prepare(allow_untracked: bool = False) -> Site:
    """Check the sources, then write the pages Astro reads, the generated settings, the files the
    site serves as they are and each page's Markdown copy. Nothing is removed or written before
    every check has passed."""
    if not allow_untracked:
        check_tracked_site()
    cause = brand_refusal()
    if cause:
        raise SiteError(cause)
    tool = share_tool()
    cause = tool.refusal(ROOT / tool.OUTPUT) or tool.mark_refusal(ROOT)
    if cause:
        raise SiteError(f"the share image or the mark: {cause}: run python3 tools/share_image.py")
    site = collect_site()
    home = site.pages["index"]
    texts = {page.id: site_page_text(page, home) for page in site.pages.values()}
    llms = site_llms(site)
    for folder in (CONTENT, GENERATED, SERVED):
        if folder.exists():
            shutil.rmtree(folder)
    for page in site.pages.values():
        name = {"home": "index", "section": f"{page.id}/index"}.get(page.kind, page.id)
        target = CONTENT / f"{name}.{'mdx' if page.mdx else 'md'}"
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(texts[page.id], encoding="utf-8")
        copy = COPIES / (page.path[len(BASE_PATH):]) / "index.md"
        copy.parent.mkdir(parents=True, exist_ok=True)
        copy.write_text(markdown_copy(page), encoding="utf-8")
    shutil.copytree(PUBLIC, SERVED)
    shutil.copytree(GALLERY_DIR, SERVED / "assets" / "gallery")
    shutil.copytree(ROOT / "ray_patterns" / "images", SERVED / "ray_patterns" / "images")
    for page in site.pages.values():
        for rel, data in page.files.items():
            target = SERVED / rel
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
    (SERVED / "llms.txt").write_text(llms, encoding="utf-8")
    settings = {"title": TITLE, "description": SUMMARY, "sidebar": site.sidebar, "head": share_head()}
    (GENERATED / "site.json").write_text(json.dumps(settings, indent=1, ensure_ascii=False) + "\n", encoding="utf-8")
    kinds = {}
    for page in site.pages.values():
        kinds[page.kind] = kinds.get(page.kind, 0) + 1
    print(f"prepared {len(site.pages)} pages ({', '.join(f'{n} {k}' for k, n in sorted(kinds.items()))})")
    return site


# --------------------------------------------------------------------------------------------
# Astro

WARNING = re.compile(r"\[(WARN|ERROR)\]|^\s*\(!\)|\bwarning\b", re.IGNORECASE | re.MULTILINE)


def node_environment() -> dict[str, str]:
    import os
    env = dict(os.environ)
    env.update(ASTRO_TELEMETRY_DISABLED="1", npm_config_update_notifier="false", npm_config_fund="false",
               npm_config_audit="false")
    env.setdefault("NO_COLOR", "1")
    return env


def install() -> None:
    """Install the site's packages exactly as the lockfile pins them."""
    npm = shutil.which("npm")
    if npm is None:
        raise SiteError("npm is needed to install the site's packages; install Node.js 22")
    done = subprocess.run([npm, "ci"], cwd=SITE, env=node_environment(), check=False)
    if done.returncode != 0:
        raise SiteError(f"npm ci in site/ failed with exit code {done.returncode}: fix what npm printed above, "
                        "then run python3 tools/build_site.py build again")


def astro_build() -> str:
    """Run ``astro build``; a warning or an error in its output fails the build."""
    binary = SITE / "node_modules" / ".bin" / "astro"
    if not binary.exists():
        raise SiteError(f"{binary.relative_to(ROOT)} is missing: run npm ci in site/")
    if DIST.exists():
        shutil.rmtree(DIST)
    done = subprocess.run([str(binary), "build"], cwd=SITE, env=node_environment(), capture_output=True,
                          text=True, check=False)
    log = done.stdout + done.stderr
    if done.returncode != 0:
        raise SiteError(f"astro build failed (exit {done.returncode}):\n{log}")
    flagged = [line for line in log.splitlines() if WARNING.search(line)]
    if flagged:
        raise SiteError("astro build printed warnings:\n" + "\n".join(flagged))
    return log


OWN_ADDRESS = re.compile(r'<link rel="canonical"[^>]*>|<meta property="og:url"[^>]*>')


def finish() -> None:
    """Write what Astro does not (each page's Markdown copy, sitemap.xml and its gzip copy, the
    old addresses' files), then check the whole site."""
    if not (DIST / "index.html").is_file():
        raise SiteError(f"no built site in {DIST.relative_to(ROOT)}: run python3 tools/build_site.py build")
    if not COPIES.is_dir():
        raise SiteError("no Markdown copies: run python3 tools/build_site.py prepare first")
    for copy in sorted(COPIES.rglob("index.md")):
        target = DIST / copy.relative_to(COPIES)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(copy, target)
    sitemap = DIST / "sitemap-0.xml"
    if not sitemap.is_file():
        raise SiteError("Astro wrote no sitemap-0.xml")
    data = sitemap.read_bytes()
    (DIST / "sitemap.xml").write_bytes(data)
    (DIST / "sitemap.xml.gz").write_bytes(gzip_bytes(data))
    missing = DIST / "404.html"
    if missing.is_file():
        # The page a host serves for any missing address has no address of its own.
        missing.write_text(OWN_ADDRESS.sub("", missing.read_text(encoding="utf-8")), encoding="utf-8")
    problems = check_site()
    if problems:
        raise SiteError(f"the built site has {len(problems)} problem(s):\n" + "\n".join(problems[:200]))


def gzip_bytes(data: bytes) -> bytes:
    """``data`` compressed with gzip, with no name and no time in its header, so the bytes
    depend on ``data`` alone."""
    import gzip
    import io
    buffer = io.BytesIO()
    with gzip.GzipFile(filename="", mode="wb", fileobj=buffer, mtime=0, compresslevel=9) as out:
        out.write(data)
    return buffer.getvalue()


def build(allow_untracked: bool = False) -> None:
    prepare(allow_untracked)
    if not (SITE / "node_modules").is_dir():
        install()
    log = astro_build()
    print("\n".join(log.splitlines()[-3:]))
    finish()
    print(f"built and checked {DIST.relative_to(ROOT)}")


# --------------------------------------------------------------------------------------------
# Checking the built site

import html.parser  # noqa: E402
from urllib.parse import urljoin, urlsplit  # noqa: E402

RESOURCE_TAGS = {"script": "src", "img": "src", "source": "src", "iframe": "src", "audio": "src",
                 "video": "src", "embed": "src"}
FETCHED_RELS = {"stylesheet", "preload", "modulepreload", "prefetch", "preconnect", "dns-prefetch", "icon",
                "shortcut", "manifest"}
IMAGE_META = frozenset({"og:image", "og:image:url", "og:image:secure_url", "twitter:image"})
TRACKERS = re.compile(
    r"googletagmanager|google-analytics|fonts\.googleapis|fonts\.gstatic|plausible\.io|"
    r"cdn\.jsdelivr|unpkg\.com|cdnjs\.|segment\.(com|io)|hotjar|clarity\.ms|intercom", re.IGNORECASE)
# The host's address function treats a last path part with a dot, a letter, then letters, digits
# or underscores as a file; any other address is sent to its slash form.
EXTENSION = re.compile(r"\.[A-Za-z][A-Za-z0-9_]*$")


class ParsedPage(html.parser.HTMLParser):
    """The ids of a page and every address it names, with the tag and whether it is fetched."""

    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.ids: list[str] = []
        self.refs: list[tuple[str, str, bool]] = []
        self.canonical: list[str] = []

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        values = {name: value or "" for name, value in attrs}
        if "id" in values:
            self.ids.append(values["id"])
        if tag == "a" and "href" in values:
            self.refs.append((tag, values["href"], False))
        if tag == "link" and "href" in values:
            rels = set(values.get("rel", "").lower().split())
            if "canonical" in rels:
                self.canonical.append(values["href"])
            self.refs.append((tag, values["href"], bool(rels & FETCHED_RELS)))
        attribute = RESOURCE_TAGS.get(tag)
        if attribute and attribute in values:
            self.refs.append((tag, values[attribute], True))
        if "srcset" in values:
            for part in values["srcset"].split(","):
                if part.strip():
                    self.refs.append((tag, part.split()[0], True))
        named = values.get("property") or values.get("name")
        if tag == "meta" and named in IMAGE_META and values.get("content"):
            self.refs.append((tag, values["content"], True))


def parse_page(path: Path) -> ParsedPage:
    parser = ParsedPage()
    parser.feed(path.read_text(encoding="utf-8"))
    parser.close()
    return parser


def built_file(dist: Path, path: str) -> Path | None:
    """The built file an address path names, or None."""
    if not path.startswith(BASE_PATH):
        return None
    rest = path[len(BASE_PATH):]
    if rest == "" or rest.endswith("/"):
        rest += "index.html"
    candidate = dist / rest
    return candidate if candidate.is_file() else None


def read_manifest() -> tuple[list[tuple[str, str, list[str]]], list[str]]:
    """The pages (address, title id, other ids) and the files of the MkDocs site."""
    pages, files = [], []
    for line in MANIFEST.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#"):
            continue
        fields = line.split("\t")
        if fields[0] == "page":
            pages.append((fields[1], fields[2], fields[3].split() if len(fields) > 3 else []))
        elif fields[0] == "file":
            files.append(fields[1])
        else:
            raise SiteError(f"{MANIFEST.relative_to(ROOT)}: a line of an unknown kind: {line!r}")
    return pages, files


def check_site(dist: Path = DIST) -> list[str]:
    """Every problem of a built site: a link, anchor or asset that does not resolve; an address
    outside the base folder; anything fetched from another host; a page without exactly one
    canonical link or without its Markdown copy; an id used twice; a file the host's address
    function could not serve; and any address or heading id of the MkDocs site that the site no
    longer serves."""
    problems: list[str] = []
    parsed: dict[Path, ParsedPage] = {}

    def page(path: Path) -> ParsedPage:
        if path not in parsed:
            parsed[path] = parse_page(path)
        return parsed[path]

    exists: dict[str, Path | None] = {}

    def target(path: str) -> Path | None:
        if path not in exists:
            exists[path] = built_file(dist, path)
        return exists[path]

    def check_ref(base_url: str, tag: str, ref: str, fetched: bool) -> str | None:
        if ref.startswith("//"):
            return f"an address without a scheme: {ref}"
        if ref.startswith(("mailto:", "data:")):
            return None
        parts = urlsplit(urljoin(base_url, ref))
        if parts.scheme not in ("http", "https"):
            return f"an address of another kind: {ref}"
        if f"{parts.scheme}://{parts.netloc}" != NEW_SITE:
            if fetched or tag not in ("a", "md"):
                return f"fetches from another host: {ref}"
            return None
        if not fetched and tag in ("a", "md") and parts.path in ("", "/") and not parts.query:
            return None  # the home page of Eugo, which makes the notebooks
        if not parts.path.startswith(BASE_PATH):
            return f"an address outside {BASE_PATH}: {ref}"
        found = target(parts.path)
        if found is None:
            return f"a link to nothing: {ref}"
        if parts.fragment and found.suffix == ".html" and parts.fragment not in page(found).ids:
            return f"a link to an anchor that does not exist: {ref}"
        return None

    for path in sorted(dist.rglob("*.html")):
        name = path.relative_to(dist).as_posix()
        here = page(path)
        twice = sorted({i for i in here.ids if here.ids.count(i) > 1})
        if twice:
            problems.append(f"{name}: ids used twice: {', '.join(twice)}")
        if name != "404.html":
            if len(here.canonical) != 1:
                problems.append(f"{name}: {len(here.canonical)} canonical links")
            if name.endswith("index.html") and not (path.parent / "index.md").is_file():
                problems.append(f"{name}: no Markdown copy beside it (index.md)")
        rel = name[: -len("index.html")] if name.endswith("index.html") else name
        base_url = NEW_ROOT + rel
        for tag, ref, fetched in here.refs:
            problem = check_ref(base_url, tag, ref, fetched)
            if problem:
                problems.append(f"{name}: {problem}")
    for path in sorted(dist.rglob("*")):
        if not path.is_file():
            continue
        name = path.relative_to(dist).as_posix()
        if not EXTENSION.search(path.name):
            problems.append(f"{name}: a file without an extension, which the host would never serve")
        if path.suffix not in (".html", ".css", ".js", ".md", ".txt", ".xml", ".json"):
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        if TRACKERS.search(text):
            problems.append(f"{name}: names a host of scripts, fonts or counters")
        if path.suffix == ".css":
            for url in re.findall(r"url\(\s*['\"]?([^'\")]+)", text):
                if url.startswith(("http:", "https:", "//")):
                    problems.append(f"{name}: a style loads {url}")
        if path.suffix in (".md", ".txt"):
            for ref in re.findall(r"\]\((https?://[^)\s]+)\)", text):
                problem = check_ref(NEW_ROOT, "md", ref, False)
                if problem:
                    problems.append(f"{name}: {problem}")
    for folder in sorted(p for p in dist.rglob("*") if p.is_dir()):
        if EXTENSION.search(folder.name) and (folder / "index.html").is_file():
            problems.append(f"{folder.relative_to(dist).as_posix()}/: a page whose address looks like a file's")
    problems += check_parity(dist, page)
    return problems


def check_parity(dist: Path, page) -> list[str]:
    """Every address of the MkDocs site (site/legacy-urls.txt) is served, and every heading id
    it had is on the same page, unless ANCHOR_EXCEPTIONS gives the reason it is not."""
    problems = []
    pages, files = read_manifest()
    for address, title_id, ids in pages:
        built = dist / address.lstrip("/") / "index.html"
        if not built.is_file():
            problems.append(f"old address not served: {address}")
            continue
        have = set(page(built).ids)
        excused = ANCHOR_EXCEPTIONS.get(address, {})
        for wanted in [title_id, *ids]:
            if wanted not in have and wanted not in excused:
                problems.append(f"{address}: the old heading id #{wanted} is gone")
    for address in files:
        if not (dist / address.lstrip("/")).is_file():
            problems.append(f"old file not served: {address}")
    return problems


# --------------------------------------------------------------------------------------------
# The old address: redirect pages for GitHub Pages

REDIRECT_PAGE = """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>{title}</title>
<script>location.replace({target_js} + location.search + location.hash);</script>
<meta http-equiv="refresh" content="0; url={target}">
<link rel="canonical" href="{target}">
</head>
<body>
<p>This page has moved to <a href="{target}">{target}</a>.</p>
</body>
</html>
"""

REDIRECT_404 = """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>ganymede has moved</title>
<meta name="robots" content="noindex">
<script>
(function () {{
  var prefix = {old_base_js};
  var path = location.pathname;
  var rest = path.indexOf(prefix) === 0 ? path.slice(prefix.length) : "";
  location.replace({new_root_js} + rest + location.search + location.hash);
}})();
</script>
</head>
<body>
<p>The ganymede website has moved to <a href="{new_root}">{new_root}</a>.</p>
</body>
</html>
"""


def redirect_file_source(address: str) -> Path | bytes:
    """Where the redirect site takes a file of the old site from: the repository's own copy."""
    rel = address.lstrip("/")
    if rel.endswith(".ipynb"):
        folder, _, name = rel.rpartition("/")
        repo = f"{folder}.ipynb"
        if Path(repo).name != name or not (ROOT / repo).is_file():
            raise SiteError(f"{address}: no notebook {repo} in the repository")
        return ROOT / repo
    if rel.startswith("ray_patterns/images/"):
        return ROOT / rel
    if rel.startswith("assets/gallery/"):
        return GALLERY_DIR / rel.rsplit("/", 1)[1]
    if rel == "assets/social-preview.png":
        return PUBLIC / rel
    raise SiteError(f"{address}: the redirect site does not know where this file comes from")


def redirects(out: Path = REDIRECTS) -> int:
    """Write the site that replaces the MkDocs site at its old address: a page per old page
    that sends the reader to the same page on the new site (keeping the query and the anchor),
    a 404 page that does the same for any other address, the files the old site served (a
    redirect cannot move a download), llms.txt and a sitemap of the old addresses. Every entry
    of site/legacy-urls.txt is checked to be in it."""
    out = out.resolve()
    if out == ROOT or ROOT.is_relative_to(out) or not out.is_relative_to(ROOT / "build"):
        raise SiteError(f"the redirect site goes under build/ of the repository, not {out}")
    pages, files = read_manifest()
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    for address, _title, _ids in pages:
        new = NEW_ROOT + address.lstrip("/")
        target = out / address.lstrip("/") / "index.html"
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(REDIRECT_PAGE.format(title="ganymede: this page has moved", target=html_escape(new),
                                               target_js=json.dumps(new)), encoding="utf-8")
    for address in files:
        if address in ("/404.html", "/llms.txt", "/sitemap.xml", "/sitemap.xml.gz"):
            continue
        target = out / address.lstrip("/")
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(redirect_file_source(address), target)
    (out / "404.html").write_text(REDIRECT_404.format(old_base_js=json.dumps(OLD_BASE), new_root=NEW_ROOT,
                                                      new_root_js=json.dumps(NEW_ROOT)), encoding="utf-8")
    (out / "llms.txt").write_text(
        "# ganymede\n\n> The ganymede website has moved. Every page is now under "
        f"{NEW_ROOT}, and its list of pages is {NEW_ROOT}llms.txt.\n", encoding="utf-8")
    urls = "".join(f"<url><loc>{html_escape(OLD_ROOT + a.lstrip('/'))}</loc></url>" for a, _t, _i in pages)
    sitemap = ('<?xml version="1.0" encoding="UTF-8"?>\n'
               f'<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">{urls}</urlset>\n').encode()
    (out / "sitemap.xml").write_bytes(sitemap)
    (out / "sitemap.xml.gz").write_bytes(gzip_bytes(sitemap))
    missing = [a for a, _t, _i in pages if not (out / a.lstrip("/") / "index.html").is_file()]
    missing += [a for a in files if not (out / a.lstrip("/")).is_file()]
    if missing:
        raise SiteError(f"the redirect site lacks {len(missing)} old address(es): {missing[:10]}")
    print(f"redirect site: {len(pages)} pages and {len(files)} files of the old site, in {out.relative_to(ROOT)}")
    return 0


def clean() -> None:
    """Remove everything the build writes; the installed packages stay."""
    for folder in (DIST, ASTRO_CACHE, SERVED, CONTENT, GENERATED, REDIRECTS):
        if folder.exists():
            shutil.rmtree(folder)
            print(f"removed {folder.relative_to(ROOT)}")
    build_dir = ROOT / "build"
    if build_dir.is_dir() and not any(build_dir.iterdir()):
        build_dir.rmdir()


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


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="build_site.py", description=__doc__.split("\n\n")[0])
    commands = parser.add_subparsers(dest="command", required=True)
    for name, text in (("build", "prepare, install the packages if needed, run Astro, finish"),
                       ("prepare", "write the pages, settings and files Astro reads")):
        sub = commands.add_parser(name, help=text)
        sub.add_argument("--allow-untracked", action="store_true",
                         help="build even when the source folders hold files git does not track")
    commands.add_parser("finish", help="write the Markdown copies and the sitemap, then check the site")
    commands.add_parser("check", help="check a site already built in site/dist/")
    sub = commands.add_parser("redirects", help="write the redirect site for the old address")
    sub.add_argument("--out", type=Path, default=REDIRECTS, help="its folder, under build/ (default build/redirects)")
    commands.add_parser("clean", help="remove everything the build writes (not site/node_modules/)")
    sub = commands.add_parser("check-brand", help="check site/src/styles/brand.css against its digest")
    sub.add_argument("--against", type=Path, metavar="PATH",
                     help="also compare it with another copy of the shared stylesheet")
    commands.add_parser("refresh-gallery", help="rewrite docs/assets/gallery/*.png from the notebooks' figures")
    args = parser.parse_args(argv)
    try:
        if args.command == "build":
            build(args.allow_untracked)
        elif args.command == "prepare":
            prepare(args.allow_untracked)
        elif args.command == "finish":
            finish()
            print(f"checked {DIST.relative_to(ROOT)}")
        elif args.command == "check":
            problems = check_site()
            for problem in problems:
                print(problem, file=sys.stderr)
            if problems:
                return 1
            print(f"{DIST.relative_to(ROOT)}: no problems")
        elif args.command == "redirects":
            redirects(args.out)
        elif args.command == "clean":
            clean()
        elif args.command == "check-brand":
            cause = brand_refusal(args.against)
            if cause:
                raise SiteError(cause)
            print(f"brand.css: sha256 {BRAND_SHA256} without its licence lines")
        elif args.command == "refresh-gallery":
            refresh_gallery()
    except SiteError as error:
        print(f"build_site: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())

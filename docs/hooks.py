# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo, Inc.
"""MkDocs hooks for the ganymede site.

- give every page a meta description and a link to its source on GitHub;
- in notebook pages, point relative links (``../README.md``, ``THIRD_PARTY_NOTICES.md``) at the
  matching site page or at GitHub, and replace embedded copies of ``ray_patterns/images/*.png``
  with links to those files;
- move the large stylesheet that every notebook page inlines into one shared, cacheable file.

Reads ``pages.json``, which ``tools/build_site.py`` writes next to the assembled ``mkdocs.yml``.
"""
from __future__ import annotations

import base64
import hashlib
import json
import posixpath
import re
from pathlib import Path

from mkdocs.utils import get_relative_url

REPO_BLOB = "https://github.com/eugo-inc/ganymede/blob/main/"
PAGES: dict[str, dict] = {}
IMAGES: dict[str, str] = {}  # sha256 of an image file -> its path in the docs folder
HREF = re.compile(r'href="(?![a-zA-Z][a-zA-Z0-9+.-]*:|#|/)([^"#?]+)([^"]*)"')
DATA_PNG = re.compile(r'src="data:image/png;base64,([A-Za-z0-9+/=\s]+)"')
STYLE = re.compile(r'<style type="text/css">(.*?)</style>', re.S)
SHARE_MIN = 100_000  # inline stylesheets at least this long move to a shared file
SHARED_CSS: set[str] = set()  # stylesheets already written to the site folder


def on_config(config):
    meta = Path(config.config_file_path).parent / "pages.json"
    if not meta.is_file():
        raise SystemExit(f"{meta} not found: build the copy of mkdocs.yml that tools/build_site.py writes")
    PAGES.clear()
    PAGES.update(json.loads(meta.read_text(encoding="utf-8"))["pages"])
    SHARED_CSS.clear()
    IMAGES.clear()
    images = Path(config.docs_dir) / "ray_patterns" / "images"
    for png in sorted(images.glob("*.png")):
        IMAGES[hashlib.sha256(png.read_bytes()).hexdigest()] = f"ray_patterns/images/{png.name}"
    return config


def _resolve(page, target: str, files) -> str:
    """Site-relative URL for a link written relative to a notebook's folder in the repository."""
    repo_path = posixpath.normpath(posixpath.join(posixpath.dirname(page.file.src_uri), target))
    if repo_path.endswith("README.md"):
        index = posixpath.join(posixpath.dirname(repo_path), "index.md")
        if index in PAGES:
            repo_path = index
    found = files.get_file_from_path(repo_path)
    if found is not None and not repo_path.startswith(".."):
        return get_relative_url(found.url, page.url)
    return REPO_BLOB + repo_path.lstrip("./")


def on_page_content(html, page, config, files):
    if not page.file.src_uri.endswith(".ipynb"):
        return html

    def fix_href(match: re.Match) -> str:
        return f'href="{_resolve(page, match.group(1), files)}{match.group(2)}"'

    def fix_image(match: re.Match) -> str:
        try:
            data = base64.b64decode(re.sub(r"\s+", "", match.group(1)))
        except ValueError:
            return match.group(0)
        known = IMAGES.get(hashlib.sha256(data).hexdigest())
        if known is None:
            return match.group(0)
        return f'src="{get_relative_url(known, page.url)}" loading="lazy"'

    html = HREF.sub(fix_href, html)
    if page.file.src_uri.startswith("ray_patterns/"):
        html = DATA_PNG.sub(fix_image, html)
    return html


def on_page_context(context, page, config, nav):
    info = PAGES.get(page.file.src_uri)
    if info:
        if not page.meta.get("description"):
            page.meta["description"] = info["description"]
        page.meta["github_url"] = info["github"]
        if info["kind"] == "notebook":
            page.meta["notebook"] = True
            page.title = info["title"]
    return context


def on_post_page(output, page, config):
    if not page.file.src_uri.endswith(".ipynb"):
        return output

    def share(match: re.Match) -> str:
        css = match.group(1)
        if len(css) < SHARE_MIN:
            return match.group(0)
        name = f"assets/stylesheets/notebook-{hashlib.sha256(css.encode()).hexdigest()[:12]}.css"
        if name not in SHARED_CSS:
            target = Path(config.site_dir) / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(css, encoding="utf-8")
            SHARED_CSS.add(name)
        return f'<link rel="stylesheet" href="{get_relative_url(name, page.url)}">'

    return STYLE.sub(share, output)

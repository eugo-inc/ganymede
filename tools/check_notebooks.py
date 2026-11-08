#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo
"""Structural checks for every notebook in this repository.

- every .ipynb parses as nbformat 4 JSON;
- notebook metadata holds only ``kernelspec`` and ``language_info``;
- cell metadata holds only ``tags``;
- outside ``ray_patterns/`` code cells carry no outputs and no execution counts
  (``ray_patterns/`` notebooks are committed with outputs from a real run).

Exit status 1 when any check fails.
"""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUTS_ALLOWED = ("ray_patterns/",)
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
    problems = []
    if nb.get("nbformat") != 4:
        problems.append(f"{rel}: nbformat {nb.get('nbformat')} (expected 4)")
    extra = set(nb.get("metadata", {})) - NB_META
    if extra:
        problems.append(f"{rel}: unexpected notebook metadata {sorted(extra)}")
    outputs_ok = rel.startswith(OUTPUTS_ALLOWED)
    for i, cell in enumerate(nb.get("cells", [])):
        extra = set(cell.get("metadata", {})) - CELL_META
        if extra:
            problems.append(f"{rel}: cell {i} has unexpected metadata {sorted(extra)}")
        if cell.get("cell_type") == "code" and not outputs_ok:
            if cell.get("outputs"):
                problems.append(f"{rel}: cell {i} has outputs")
            if cell.get("execution_count") is not None:
                problems.append(f"{rel}: cell {i} has an execution count")
    return problems


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
    problems = [msg for nb in notebooks for msg in check(nb)]
    for msg in problems:
        print(msg)
    print(f"checked {len(notebooks)} notebooks, {len(problems)} problem(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())

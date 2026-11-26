# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo, Inc.
"""Draw the ganymede mark, the favicon, the README's banner and the share image of the website.

The mark is Ganymede as a moon in its phase, its lit side azure and its night side violet, inside
a thick arc of its orbit in the site's violet-to-azure gradient, on a 64-unit square, with a soft
shadow under it. This script writes it, for the light and the dark theme, to
``site/src/assets/mark-light.svg`` and ``mark-dark.svg`` (the header's logo and the home page's
picture), ``site/public/favicon.svg``, which follows the reader's colour scheme, and the README's
banner, ``docs/readme/banner-light.svg`` and ``banner-dark.svg``, which also carries the maker's
logo (``docs/readme/eugo-logo.svg``, nested as it is, in its own gradients on either theme) in its
top right corner. It also copies that logo to ``site/src/assets/eugo-logo.svg``, the website's
header and footer.

The share image, ``site/public/assets/social-preview.png``, is what a page shared on a social site
or in a chat shows beside its title. It holds a band of the site's accent gradient along its top,
a faint indigo glow, the mark in its dark colours beside the name ganymede, and one line on what
the site holds, on the dark ground of the site, with the maker's logo in its top right corner: the
outlines of ``docs/readme/eugo-logo.svg``, each filled with its own gradient.

The name and the line are set in Red Hat Display, the font of the site's headings: the name at
weight 800 with its letters 0.03 of its size closer together, the mark 1.1 times the size high
and 0.3 of the size before the name, as the site's header sets them; the line at weight 400.
Their outlines, shaped by HarfBuzz from the Red Hat Display file the site installs, are kept in
``tools/share_image_letters.json``,
so neither the banner nor this script needs a font: ``--letters`` shapes them again (it needs
``pip install uharfbuzz fonttools brotli`` and ``npm ci`` in ``site/``).

The drawing uses only the standard library and arithmetic that gives the same result on every
machine: sums, products, quotients and square roots, never the system's fonts or its
trigonometry. A shape covers each pixel by its distance from the pixel's middle; a letter covers
it by the crossings of its outline with 16 lines across each row of pixels. The shadow is
blurred by three passes of a box filter, whose widths come from a square root and whose sums are
whole numbers, never by the exponential of a Gaussian. The build copies the committed files as
they are, once ``refusal`` finds the image is the file this script writes, up to how its rows are
compressed, and ``mark_refusal`` finds the SVG files are the ones it writes; ``is_current`` says
exactly what is accepted and refused.

Usage::

    python3 tools/share_image.py            # write the marks, the favicon, the banner and the share image
    python3 tools/share_image.py --check    # exit 1, naming the cause, when a file differs
    python3 tools/share_image.py --letters  # shape the letters again, then write every file
"""

from __future__ import annotations

import argparse
import functools
import io
import itertools
import json
import math
import re
import struct
import sys
import zlib
from array import array
from collections.abc import Callable, Sequence
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
OUTPUT = "site/public/assets/social-preview.png"
WIDTH = 1200
HEIGHT = 630
NAME = "ganymede"
LINE = "Open notebooks for HPC, Ray and geospatial Python"
ALT = (
    f"The name {NAME} in white beside its mark, a moon lit blue on one side and violet on its "
    f"night side inside an arc of its orbit, above the line “{LINE}”, on a dark background "
    "under a violet-to-blue band, with the maker's logo in the top right corner."
)

Point = tuple[float, float]
Colour = tuple[int, int, int]
# The colour of a layer at a pixel, given its column and its row.
Paint = Callable[[int, int], Colour]

# The two themes: the moon's night side and lit side, each shaded from its top colour to its
# bottom one; the orbit's gradient, from its lower left to its upper right; the shadow's colour
# and opacity; and the README banner's ground, frame, name and line.
THEMES: dict[str, dict] = {
    "light": {
        "violet": ("#8e45ff", "#7a36ec"),
        "azure": ("#2aa6ff", "#1a86dd"),
        "orbit": ("#7a36ec", "#1a86dd"),
        "shadow": ("#160f3d", 0.22),
        "ground": "#ffffff", "border": "#e6e4f0", "ink": "#160f3d", "muted": "#55566b",
    },
    "dark": {
        "violet": ("#b26bff", "#a24bff"),
        "azure": ("#4db6ff", "#2aa6ff"),
        "orbit": ("#a24bff", "#2aa6ff"),
        "shadow": ("#000000", 0.55),
        "ground": "#160f3d", "border": "#352a7f", "ink": "#ffffff", "muted": "#d6d3ee",
    },
}

# The mark, in its 64-unit square. The drawing is moved by OFFSET to sit in the middle of the
# square; every other place below is before that move.
MARK_UNITS = 64.0
OFFSET: Point = (6.0, -2.5)
# The moon: its centre and radius. Its lit side is the part of a second disc, of the same
# radius, that lies inside it.
MOON = (35.0, 29.0, 15.0)
LIT = (42.0, 22.0, 15.0)
# The orbit: a stroke ORBIT_WIDTH wide with round ends along a circle round ORBIT_CENTRE, from
# its first end clockwise to its second (95 and 235 degrees, written to two places, so that no
# cosine is ever taken), with a clear ring GAP wide cut out of it round the moon.
ORBIT_CENTRE: Point = (32.0, 32.0)
ORBIT_RADIUS = 26.0
ORBIT_WIDTH = 8.0
ORBIT_ENDS: tuple[Point, Point] = ((29.73, 57.9), (17.09, 10.7))
ORBIT_GRADIENT: tuple[Point, Point] = ((4.0, 30.0), (40.0, 62.0))
GAP = 4.5
# The shadow: straight down by its offset, blurred with its standard deviation, inside the
# filter's region (left, top, width, height), all in the square's units.
SHADOW_OFFSET = 1.2
SHADOW_DEVIATION = 1.1
SHADOW_REGION = (-4.0, -4.0, 72.0, 72.0)

# The share image: the dark palette's ground, name and line; the band's gradient, each stop a
# colour and its place in hundredths of the width, which the README banner's band uses too; the
# band's height; and a glow of indigo that fades to the ground, round a point a little above the
# middle.
BACKGROUND: Colour = (0x16, 0x0F, 0x3D)
NAME_COLOUR: Colour = (0xFF, 0xFF, 0xFF)
LINE_COLOUR: Colour = (0xD6, 0xD3, 0xEE)
GRADIENT: tuple[tuple[Colour, int], ...] = (
    ((0x8E, 0x45, 0xFF), 0),
    ((0x6A, 0x5C, 0xFF), 45),
    ((0x2A, 0xA6, 0xFF), 100),
)
BAND_HEIGHT = 10
GLOW: Colour = (0x35, 0x2A, 0x7F)
GLOW_OPACITY = 0.55
# The glow's middle and its reach across and down, as shares of the image's width and height.
GLOW_MIDDLE: Point = (0.5, 0.42)
GLOW_REACH = 0.6
# The opacity of the share image's shadow: the dark theme's.
SHADOW_OPACITY = THEMES["dark"]["shadow"][1]
# The box passes that blur the shadow, and the whole steps of coverage the blur sums.
BLUR_PASSES = 3
COVER_STEPS = 1024
# The lines across each row of pixels that a letter's outline is crossed with, and the straight
# pieces each curve of an outline is drawn as.
SAMPLES = 16
CURVE_PIECES = 8
# The maker's logo in the share image's top right corner: the file that draws it (each outline
# filled with its own gradient), its width, and its distance from the top and from the right
# edge, in pixels.
LOGO = "docs/readme/eugo-logo.svg"
SHARE_LOGO = (160.0, 40.0, 44.0)


def _rgb(text: str) -> Colour:
    return int(text[1:3], 16), int(text[3:5], 16), int(text[5:7], 16)


# --- the letters ---------------------------------------------------------------------------
# tools/share_image_letters.json holds, for the name and for the line, the glyphs HarfBuzz
# shaped them into: each glyph's outline in the font's units (M, L, Q and C commands with their
# points, and Z), and the run of glyphs with each one's advance and offsets. It holds the
# highest and lowest ink of the name too, so that the mark can be centred on its letters.

LETTERS = "tools/share_image_letters.json"
FONT_PACKAGE = "site/node_modules/@fontsource-variable/red-hat-display"
FONT = f"{FONT_PACKAGE}/files/red-hat-display-latin-wght-normal.woff2"
# Each text the letters file holds: its key, the text and the weight it is set at.
TEXTS = (("name", NAME, 800), ("line", LINE, 400))


@functools.cache
def letters() -> dict:
    """The letters file, read once."""
    data = json.loads((REPO / LETTERS).read_text(encoding="utf-8"))
    for key, text, weight in TEXTS:
        entry = data["texts"][key]
        if entry["text"] != text or entry["weight"] != weight:
            raise ValueError(f"{LETTERS} does not hold {text!r} at weight {weight}: "
                             "run python3 tools/share_image.py --letters")
    return data


def _commands(outline: str) -> list[tuple[str, list[Point]]]:
    """An outline's commands, each its letter and its points."""
    commands: list[tuple[str, list[float]]] = []
    for token in outline.split():
        if token.isalpha():
            commands.append((token, []))
        else:
            commands[-1][1].append(float(token))
    return [(op, list(zip(values[0::2], values[1::2], strict=True))) for op, values in commands]


def _placed(key: str, x: float, baseline: float, size: float, spacing: float):
    """Each glyph of a text set from ``x`` on ``baseline`` at ``size`` pixels: its commands,
    and the place and scale that turn its font units into pixels."""
    data = letters()
    entry = data["texts"][key]
    scale = size / data["upem"]
    pen_x = x
    for glyph, advance, x_offset, y_offset in entry["run"]:
        origin = pen_x + x_offset * scale, baseline - y_offset * scale
        yield _commands(entry["glyphs"][glyph]), *origin, scale
        pen_x += advance * scale + spacing


def text_width(key: str, size: float, spacing: float = 0.0) -> float:
    """The advance of a text at ``size`` pixels, its letters ``spacing`` apart beyond the font's."""
    data = letters()
    entry = data["texts"][key]
    scale = size / data["upem"]
    return sum(run[1] for run in entry["run"]) * scale + spacing * (len(entry["text"]) - 1)


def text_path(key: str, x: float, baseline: float, size: float, spacing: float = 0.0) -> str:
    """A text's outlines as the commands of one SVG path."""
    parts = []
    for commands, ox, oy, scale in _placed(key, x, baseline, size, spacing):
        for op, points in commands:
            places = (f"{ox + px * scale:.2f} {oy - py * scale:.2f}" for px, py in points)
            parts.append(op + " ".join(places))
    return "".join(parts)


def text_contours(
    key: str, x: float, baseline: float, size: float, spacing: float = 0.0
) -> list[list[Point]]:
    """A text's outlines in pixels, each closed contour a list of points, its curves drawn as
    ``CURVE_PIECES`` straight pieces each."""
    contours: list[list[Point]] = []
    for commands, ox, oy, scale in _placed(key, x, baseline, size, spacing):
        current: list[Point] = []
        for op, points in commands:
            pixels_ = [(ox + px * scale, oy - py * scale) for px, py in points]
            if op == "M":
                if len(current) > 2:
                    contours.append(current)
                current = [pixels_[0]]
            elif op == "L":
                current.append(pixels_[0])
            elif op in ("Q", "C"):
                ends = [current[-1], *pixels_]
                for piece in range(1, CURVE_PIECES + 1):
                    current.append(_bezier(ends, piece / CURVE_PIECES))
            elif op == "Z":
                if len(current) > 2:
                    contours.append(current)
                current = []
        if len(current) > 2:
            contours.append(current)
    return contours


def _bezier(points: Sequence[Point], t: float) -> Point:
    """The point at ``t`` of a Bézier curve, by de Casteljau's construction."""
    row = list(points)
    while len(row) > 1:
        row = [(a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t) for a, b in itertools.pairwise(row)]
    return row[0]


def name_ink(size: float) -> tuple[float, float]:
    """The highest and the lowest ink of the name at ``size`` pixels, above the baseline."""
    data = letters()
    scale = size / data["upem"]
    top, bottom = data["texts"]["name"]["ink"]
    return top * scale, bottom * scale


def wordmark(cx: float, baseline: float, size: float) -> tuple[float, float, float, float, float]:
    """The mark beside the name, centred on ``cx``, as the header sets them: the left and top of
    the mark's square, its side, where the name starts, and the spacing of its letters."""
    mark, gap, spacing = 1.1 * size, 0.3 * size, -0.03 * size
    left = cx - (mark + gap + text_width("name", size, spacing)) / 2
    top, bottom = name_ink(size)
    middle = baseline - (top + bottom) / 2
    return left, middle - mark / 2, mark, left + mark + gap, spacing


def shape_letters() -> dict:
    """The letters file's data, shaped by HarfBuzz from the Red Hat Display file the site installs."""
    try:
        import uharfbuzz as hb
        from fontTools.ttLib import woff2
    except ImportError:
        raise SystemExit("--letters needs: pip install uharfbuzz fonttools brotli") from None
    font_file = REPO / FONT
    if not font_file.is_file():
        raise SystemExit(f"{FONT} is missing: run npm ci in site/")
    version = json.loads((REPO / FONT_PACKAGE / "package.json").read_text(encoding="utf-8"))["version"]
    plain = io.BytesIO()
    woff2.decompress(io.BytesIO(font_file.read_bytes()), plain)
    face = hb.Face(plain.getvalue())

    class Recorder:
        def __init__(self) -> None:
            self.tokens: list[str] = []

        def _add(self, op: str, points) -> None:
            self.tokens.append(op)
            self.tokens.extend(repr(float(value)) for point in points for value in point)

        def moveTo(self, point) -> None:  # noqa: N802 (the pen protocol's names)
            self._add("M", [point])

        def lineTo(self, point) -> None:  # noqa: N802
            self._add("L", [point])

        def qCurveTo(self, *points) -> None:  # noqa: N802
            if len(points) != 2:
                raise SystemExit("a quadratic curve with more than one control point")
            self._add("Q", points)

        def curveTo(self, *points) -> None:  # noqa: N802
            self._add("C", points)

        def closePath(self) -> None:  # noqa: N802
            self.tokens.append("Z")

    texts = {}
    for key, text, weight in TEXTS:
        font = hb.Font(face)
        font.set_variations({"wght": weight})
        buffer = hb.Buffer()
        buffer.add_str(text)
        buffer.guess_segment_properties()
        hb.shape(font, buffer, {"kern": True, "liga": True})
        run, glyphs, tops, bottoms = [], {}, [], []
        for info, position in zip(buffer.glyph_infos, buffer.glyph_positions, strict=True):
            glyph = str(info.codepoint)
            run.append([glyph, position.x_advance, position.x_offset, position.y_offset])
            if glyph not in glyphs:
                pen = Recorder()
                font.draw_glyph_with_pen(info.codepoint, pen)
                glyphs[glyph] = " ".join(pen.tokens)
            extents = font.get_glyph_extents(info.codepoint)
            tops.append(extents.y_bearing)
            bottoms.append(extents.y_bearing + extents.height)
        texts[key] = {"text": text, "weight": weight, "ink": [max(tops), min(bottoms)],
                      "run": run, "glyphs": glyphs}
    return {
        "font": (f"Red Hat Display {version} (@fontsource-variable/red-hat-display, OFL-1.1), "
                 f"{Path(FONT).name}"),
        "upem": face.upem,
        "texts": texts,
    }


def letters_text(data: dict) -> str:
    """The letters file as it is written: one glyph or one glyph of the run to a line."""
    out = ["{", f' "font": {json.dumps(data["font"])},', f' "upem": {data["upem"]},', ' "texts": {']
    keys = list(data["texts"])
    for k, key in enumerate(keys):
        entry = data["texts"][key]
        out.append(f"  {json.dumps(key)}: {{")
        out.append(f'   "text": {json.dumps(entry["text"])}, "weight": {entry["weight"]}, '
                   f'"ink": {json.dumps(entry["ink"])},')
        out.append('   "run": [')
        out.append(",\n".join(f"    {json.dumps(item)}" for item in entry["run"]))
        out.append("   ],")
        out.append('   "glyphs": {')
        out.append(",\n".join(f"    {json.dumps(g)}: {json.dumps(o)}" for g, o in entry["glyphs"].items()))
        out.append("   }")
        out.append("  }" + ("," if k < len(keys) - 1 else ""))
    out.append(" }")
    out.append("}")
    return "\n".join(out) + "\n"


# --- drawing -------------------------------------------------------------------------------


class Layer:
    """How much of each pixel one colour covers, from 0 to 1."""

    def __init__(self, width: int, height: int) -> None:
        self.width = width
        self.height = height
        self.cover = array("d", bytes(8 * width * height))

    def _put(self, x: int, y: int, amount: float) -> None:
        index = y * self.width + x
        if amount > self.cover[index]:
            self.cover[index] = amount

    def box(self, left: int, top: int, right: int, bottom: int) -> None:
        """Cover the pixels of a box whole: its first column and row, and the ones after its last."""
        for y in range(max(0, top), min(self.height, bottom)):
            for x in range(max(0, left), min(self.width, right)):
                self._put(x, y, 1.0)

    def shape(self, box: tuple[int, int, int, int], amount: Callable[[float, float], float]) -> None:
        """Cover each pixel of a box (its first column and row, and the ones after its last) by
        ``amount`` at the pixel's middle."""
        x0, y0, x1, y1 = box
        for y in range(max(0, y0), min(self.height, y1)):
            for x in range(max(0, x0), min(self.width, x1)):
                value = amount(x + 0.5, y + 0.5)
                if value > 0.0:
                    self._put(x, y, value)

    def fill(self, contours: Sequence[Sequence[Point]]) -> None:
        """Fill closed outlines by the nonzero rule, smoothed at their edges: each row of pixels
        is crossed by ``SAMPLES`` lines, and each pixel is covered by the share of those lines'
        length inside the outlines that lies in it."""
        edges = []
        for contour in contours:
            for (xa, ya), (xb, yb) in itertools.pairwise([*contour, contour[0]]):
                if ya == yb:
                    continue
                winding = 1 if yb > ya else -1
                if ya > yb:
                    xa, ya, xb, yb = xb, yb, xa, ya
                edges.append((ya, yb, xa, (xb - xa) / (yb - ya), winding))
        if not edges:
            return
        edges.sort()
        xs = [x for contour in contours for x, _y in contour]
        x0, x1 = max(0, math.floor(min(xs))), min(self.width, math.ceil(max(xs)) + 1)
        y0 = max(0, math.floor(edges[0][0]))
        y1 = min(self.height, math.ceil(max(edge[1] for edge in edges)) + 1)
        start, active = 0, []
        for y in range(y0, y1):
            row = [0.0] * (x1 - x0)
            for sample in range(SAMPLES):
                line_y = y + (sample + 0.5) / SAMPLES
                while start < len(edges) and edges[start][0] <= line_y:
                    active.append(edges[start])
                    start += 1
                active = [edge for edge in active if edge[1] > line_y]
                crossings = sorted((edge[2] + (line_y - edge[0]) * edge[3], edge[4]) for edge in active)
                winding = 0
                for (xa, turn), (xb, _turn) in itertools.pairwise(crossings):
                    winding += turn
                    if winding:
                        _span(row, xa - x0, xb - x0)
            for column, value in enumerate(row):
                if value > 0.0:
                    self._put(x0 + column, y, min(1.0, value / SAMPLES))


def _span(row: list[float], left: float, right: float) -> None:
    """Add to a row of pixels the length of a line from ``left`` to ``right`` in each."""
    left, right = max(0.0, left), min(float(len(row)), right)
    if right <= left:
        return
    first, last = int(left), int(right)
    if first == last:
        row[first] += right - left
        return
    row[first] += first + 1 - left
    for column in range(first + 1, min(last, len(row))):
        row[column] += 1.0
    if last < len(row):
        row[last] += right - last


def _clamp(value: float) -> float:
    return 0.0 if value < 0.0 else 1.0 if value > 1.0 else value


def _disc(cx: float, cy: float, radius: float) -> Callable[[float, float], float]:
    """How much of a pixel a disc covers, given the pixel's middle; all in pixels."""

    def amount(x: float, y: float) -> float:
        dx, dy = x - cx, y - cy
        return _clamp(radius + 0.5 - math.sqrt(dx * dx + dy * dy))

    return amount


def box_widths(deviation: float, passes: int = BLUR_PASSES) -> list[int]:
    """The odd widths, in pixels, of the box passes whose blur together has the variance of a
    Gaussian of the given standard deviation, as near as odd widths come: some passes of one
    width and the rest two wider. Only a square root and plain arithmetic are used."""
    variance = 12.0 * deviation * deviation
    lower = int(math.sqrt(variance / passes + 1.0))
    if lower % 2 == 0:
        lower -= 1
    narrow = round(
        (variance - passes * lower * lower - 4 * passes * lower - 3 * passes) / (-4 * lower - 4)
    )
    narrow = max(0, min(passes, narrow))
    return [lower] * narrow + [lower + 2] * (passes - narrow)


def _box_pass(values: list[int], width: int) -> list[int]:
    """The sums of a box ``width`` values wide centred on each value, with nothing beyond
    the ends: whole numbers, so the same on every machine."""
    half = width // 2
    count = len(values)
    out = [0] * count
    total = sum(values[: min(half, count)])
    for index in range(count):
        if index + half < count:
            total += values[index + half]
        if index - half - 1 >= 0:
            total -= values[index - half - 1]
        out[index] = total
    return out


def blurred_shadow(source: Layer, box: tuple[int, int, int, int], deviation: float) -> Layer:
    """A layer of a shadow: the cover of ``source`` blurred by box passes along the rows and
    then the columns, at ``SHADOW_OPACITY``, inside a box of pixels (its first column, first
    row, and the column and row after its last) and nothing outside it.

    The cover is counted in ``COVER_STEPS`` whole steps and the passes add whole numbers; the
    sums are divided once, at the end."""
    x0, y0, x1, y1 = box
    widths = box_widths(deviation)
    grid = [
        [int(source.cover[y * source.width + x] * COVER_STEPS + 0.5) for x in range(x0, x1)]
        for y in range(y0, y1)
    ]
    for width in widths:
        grid = [_box_pass(row, width) for row in grid]
    columns = [list(column) for column in zip(*grid, strict=True)]
    for width in widths:
        columns = [_box_pass(column, width) for column in columns]
    divisor = COVER_STEPS * math.prod(widths) ** 2
    shadow = Layer(source.width, source.height)
    for column, values in enumerate(columns):
        for row, value in enumerate(values):
            if value:
                index = (y0 + row) * source.width + x0 + column
                shadow.cover[index] = SHADOW_OPACITY * value / divisor
    return shadow


def shade(ends: tuple[Colour, Colour], top: float, height: float) -> Paint:
    """The paint of a tile shaded from top to bottom: at a row, its colour mixed from its two
    ends in a straight line from the tile's top to its bottom, rounded to the nearest whole
    value."""
    (r0, g0, b0), (r1, g1, b1) = ends

    def paint(x: int, y: int) -> Colour:
        middle = y + 0.5
        t = (middle - top) / height
        t = 0.0 if t < 0.0 else 1.0 if t > 1.0 else t
        return (
            int(r0 + (r1 - r0) * t + 0.5),
            int(g0 + (g1 - g0) * t + 0.5),
            int(b0 + (b1 - b0) * t + 0.5),
        )

    return paint


def gradient(x: int, width: int = WIDTH) -> Colour:
    """The colour of the gradient at a column: between the two stops around the column, each
    channel goes from the first stop's to the second's in a straight line, rounded to the
    nearest whole value. Only whole numbers are used, so it is the same on every machine."""
    last = width - 1
    # The pair of stops whose columns, as hundredths of the last column, hold the column: the
    # last pair when the column is past the end of every other.
    pairs = list(itertools.pairwise(GRADIENT))
    (start, at), (end, until) = next(
        (pair for pair in pairs if x * 100 <= pair[1][1] * last), pairs[-1]
    )
    left, right = at * last, until * last
    span = right - left
    step = x * 100 - left
    return (
        (start[0] * (span - step) + end[0] * step + span // 2) // span,
        (start[1] * (span - step) + end[1] * step + span // 2) // span,
        (start[2] * (span - step) + end[2] * step + span // 2) // span,
    )


def compose(
    width: int, height: int, layers: Sequence[tuple[Layer, Colour | Paint | None]]
) -> bytes:
    """The rows of the image, each led by the byte of PNG's filter 0, layers drawn in order.
    A layer whose colour is ``None`` is drawn in the colour of the gradient at each column,
    and a layer whose colour is a paint in the colour the paint gives at each pixel."""
    rows = bytearray()
    for y in range(height):
        rows.append(0)
        for x in range(width):
            r, g, b = BACKGROUND
            index = y * width + x
            for layer, colour in layers:
                amount = layer.cover[index]
                if amount > 0.0:
                    if colour is None:
                        lr, lg, lb = gradient(x, width)
                    elif callable(colour):
                        lr, lg, lb = colour(x, y)
                    else:
                        lr, lg, lb = colour
                    r = int(r + (lr - r) * amount + 0.5)
                    g = int(g + (lg - g) * amount + 0.5)
                    b = int(b + (lb - b) * amount + 0.5)
            rows += bytes((r, g, b))
    return bytes(rows)



def linear(ends: tuple[Colour, Colour], start: Point, end: Point) -> Paint:
    """The paint of a gradient along a line from ``start`` to ``end``, in pixels: at a pixel,
    its colour mixed from its two ends by how far along the line the pixel's middle lies,
    rounded to the nearest whole value."""
    (r0, g0, b0), (r1, g1, b1) = ends
    dx, dy = end[0] - start[0], end[1] - start[1]
    length = dx * dx + dy * dy

    def paint(x: int, y: int) -> Colour:
        t = _clamp(((x + 0.5 - start[0]) * dx + (y + 0.5 - start[1]) * dy) / length)
        return (
            int(r0 + (r1 - r0) * t + 0.5),
            int(g0 + (g1 - g0) * t + 0.5),
            int(b0 + (b1 - b0) * t + 0.5),
        )

    return paint


def _glow_reach(x: int, y: int) -> float:
    """How far a pixel's middle lies from the glow's middle, as a share of the glow's reach."""
    dx = (x + 0.5 - GLOW_MIDDLE[0] * WIDTH) / (GLOW_REACH * WIDTH)
    dy = (y + 0.5 - GLOW_MIDDLE[1] * HEIGHT) / (GLOW_REACH * HEIGHT)
    return math.sqrt(dx * dx + dy * dy)


def glow() -> tuple[Layer, Paint]:
    """The glow: its colour at its middle and its opacity both fade in a straight line to the
    ground's, and to nothing, at its reach."""
    layer = Layer(WIDTH, HEIGHT)
    for y in range(HEIGHT):
        for x in range(WIDTH):
            t = _glow_reach(x, y)
            if t < 1.0:
                layer.cover[y * WIDTH + x] = GLOW_OPACITY * (1.0 - t)

    (r0, g0, b0), (r1, g1, b1) = GLOW, BACKGROUND

    def paint(x: int, y: int) -> Colour:
        t = min(1.0, _glow_reach(x, y))
        return (
            int(r0 + (r1 - r0) * t + 0.5),
            int(g0 + (g1 - g0) * t + 0.5),
            int(b0 + (b1 - b0) * t + 0.5),
        )

    return layer, paint


def mark_layers(left: float, top: float, side: float) -> list[tuple[Layer, Colour | Paint]]:
    """The mark's layers in a square ``side`` pixels wide at ``left`` and ``top``, as the dark
    theme's file draws it: the shadow cast by the whole drawing, the orbit, the moon's night
    side and its lit side, each in its own gradient."""
    look = THEMES["dark"]
    scale = side / MARK_UNITS

    def at(point: Point) -> Point:
        return left + (point[0] + OFFSET[0]) * scale, top + (point[1] + OFFSET[1]) * scale

    cx, cy = at(ORBIT_CENTRE)
    radius, half = ORBIT_RADIUS * scale, ORBIT_WIDTH / 2 * scale
    (ax, ay), (bx, by) = (at(end) for end in ORBIT_ENDS)
    mx, my = at(MOON[:2])
    lx, ly = at(LIT[:2])
    moon_radius = MOON[2] * scale
    hole = _disc(mx, my, (MOON[2] + GAP) * scale)
    moon = _disc(mx, my, moon_radius)
    lit_disc = _disc(lx, ly, LIT[2] * scale)
    # The two ends from the orbit's centre: a point lies between them, on the arc's side,
    # when it is clockwise of the first and anticlockwise of the second (the arc turns less
    # than half a circle).
    ux, uy, vx, vy = ax - cx, ay - cy, bx - cx, by - cy

    def orbit(x: float, y: float) -> float:
        px, py = x - cx, y - cy
        if ux * py - uy * px >= 0.0 and px * vy - py * vx >= 0.0:
            distance = abs(math.sqrt(px * px + py * py) - radius)
        else:
            ex, ey, fx, fy = x - ax, y - ay, x - bx, y - by
            distance = min(math.sqrt(ex * ex + ey * ey), math.sqrt(fx * fx + fy * fy))
        amount = _clamp(half + 0.5 - distance)
        return amount * (1.0 - hole(x, y)) if amount > 0.0 else 0.0

    def lit(x: float, y: float) -> float:
        return min(lit_disc(x, y), moon(x, y))

    # The shadow's region, and the box every layer is drawn in: the pixels whose middles lie
    # inside the filter's region, within the image.
    x, y, w, h = SHADOW_REGION
    box = (
        max(0, math.ceil(left + x * scale - 0.5)),
        max(0, math.ceil(top + y * scale - 0.5)),
        min(WIDTH, math.ceil(left + (x + w) * scale - 0.5)),
        min(HEIGHT, math.ceil(top + (y + h) * scale - 0.5)),
    )
    lowered = Layer(WIDTH, HEIGHT)
    drop = SHADOW_OFFSET * scale
    lowered.shape(box, lambda x, y: max(orbit(x, y - drop), moon(x, y - drop)))
    shadow = blurred_shadow(lowered, box, SHADOW_DEVIATION * scale)
    layers: list[tuple[Layer, Colour | Paint]] = [(shadow, _rgb(look["shadow"][0]))]
    moon_top, moon_height = my - moon_radius, 2 * moon_radius
    for amount, paint in (
        (orbit, linear(_ends(look["orbit"]), at(ORBIT_GRADIENT[0]), at(ORBIT_GRADIENT[1]))),
        (moon, shade(_ends(look["violet"]), moon_top, moon_height)),
        (lit, shade(_ends(look["azure"]), moon_top, moon_height)),
    ):
        layer = Layer(WIDTH, HEIGHT)
        layer.shape(box, amount)
        layers.append((layer, paint))
    return layers


def _ends(ends: tuple[str, str]) -> tuple[Colour, Colour]:
    return _rgb(ends[0]), _rgb(ends[1])


# Where the share image sets the name (its centre, its baseline and its size) and the line
# (its baseline and its size).
SHARE_NAME = (WIDTH / 2, 318.0, 128.0)
SHARE_LINE = (430.0, 34.0)


# A gradient of the logo file: its line, from (x1, y1) to (x2, y2) in shares of the box of the
# outline it fills, and its stops, each its place along that line and its colour.
LogoGradient = tuple[tuple[float, float, float, float], tuple[tuple[float, Colour], ...]]


@functools.cache
def logo() -> tuple[str, str, tuple[tuple[str, LogoGradient], ...]]:
    """The logo file, read once: its text, its view box (left, top, width and height), then each
    of its outlines (the commands of an SVG path, every point given whole) with the gradient that
    fills it."""
    text = (REPO / LOGO).read_text(encoding="utf-8")
    view = re.search(r'viewBox="([^"]+)"', text)
    gradients: dict[str, LogoGradient] = {}
    for ident, attributes, body in re.findall(
        r'<linearGradient id="([^"]+)"([^>]*)>(.*?)</linearGradient>', text
    ):
        x1, y1, x2, y2 = (
            float(re.findall(rf'\b{name}="([^"]+)"', attributes)[0]) for name in ("x1", "y1", "x2", "y2")
        )
        stops = tuple(
            (float(at), _rgb(colour))
            for at, colour in re.findall(r'<stop offset="([^"]+)" stop-color="(#[0-9a-f]{6})"/>', body)
        )
        gradients[ident] = (x1, y1, x2, y2), stops
    paths = re.findall(r'<path fill="url\(#([^)]+)\)" d="([^"]+)"/>', text)
    if view is None or not paths or any(ident not in gradients for ident, _ in paths):
        raise SystemExit(f"{LOGO} must have a viewBox and paths filled with its own gradients")
    return text, view.group(1), tuple((d, gradients[ident]) for ident, d in paths)


def outline_contours(d: str, place: Callable[[float, float], Point]) -> list[list[Point]]:
    """The closed contours of an SVG path of M, L, Q, C and Z commands with absolute points,
    each point put in the image by ``place``, its curves drawn as ``CURVE_PIECES`` straight
    pieces each."""
    contours: list[list[Point]] = []
    current: list[Point] = []
    for op, numbers in re.findall(r"([MLQCZ])([^MLQCZ]*)", d):
        values = [float(n) for n in numbers.split()]
        points = [place(x, y) for x, y in zip(values[0::2], values[1::2], strict=True)]
        if op == "M":
            if len(current) > 2:
                contours.append(current)
            current = [points[0]]
        elif op == "L":
            current += points
        elif op in ("Q", "C"):
            ends = [current[-1], *points]
            current += [_bezier(ends, piece / CURVE_PIECES) for piece in range(1, CURVE_PIECES + 1)]
        else:
            if len(current) > 2:
                contours.append(current)
            current = []
    if len(current) > 2:
        contours.append(current)
    return contours


def along(gradient: LogoGradient, box: tuple[float, float, float, float]) -> Paint:
    """The paint of a logo gradient over the box (left, top, width, height) of the outline it
    fills, as SVG lays a gradient in the units of that box: at a pixel, how far along the line
    its middle lies, then the colour mixed from the two stops around that place, rounded to the
    nearest whole value."""
    (x1, y1, x2, y2), stops = gradient
    left, top, width, height = box
    dx, dy = x2 - x1, y2 - y1
    length = dx * dx + dy * dy

    def paint(x: int, y: int) -> Colour:
        u, v = (x + 0.5 - left) / width, (y + 0.5 - top) / height
        t = min(max(((u - x1) * dx + (v - y1) * dy) / length, stops[0][0]), stops[-1][0])
        (at, first), (until, second) = next(
            (pair for pair in itertools.pairwise(stops) if t <= pair[1][0]), (stops[-2], stops[-1])
        )
        share = (t - at) / (until - at) if until > at else 0.0
        return (
            int(first[0] + (second[0] - first[0]) * share + 0.5),
            int(first[1] + (second[1] - first[1]) * share + 0.5),
            int(first[2] + (second[2] - first[2]) * share + 0.5),
        )

    return paint


def logo_layers() -> list[tuple[Layer, Paint]]:
    """The maker's logo in the share image's top right corner: a layer for each outline of its
    file, in that outline's gradient."""
    _, view, outlines = logo()
    view_left, view_top, view_width, _ = (float(n) for n in view.split())
    width, top, right = SHARE_LOGO
    scale = width / view_width
    left = WIDTH - right - width

    def place(x: float, y: float) -> Point:
        return left + (x - view_left) * scale, top + (y - view_top) * scale

    layers: list[tuple[Layer, Paint]] = []
    for d, gradient in outlines:
        contours = outline_contours(d, place)
        xs = [x for contour in contours for x, _y in contour]
        ys = [y for contour in contours for _x, y in contour]
        layer = Layer(WIDTH, HEIGHT)
        layer.fill(contours)
        layers.append((layer, along(gradient, (min(xs), min(ys), max(xs) - min(xs), max(ys) - min(ys)))))
    return layers


def render() -> bytes:
    """The image as rows of RGB pixels, each row led by its filter byte."""
    left, top, side, name_left, spacing = wordmark(*SHARE_NAME)
    band = Layer(WIDTH, HEIGHT)
    band.box(0, 0, WIDTH, BAND_HEIGHT)
    name = Layer(WIDTH, HEIGHT)
    name.fill(text_contours("name", name_left, SHARE_NAME[1], SHARE_NAME[2], spacing))
    baseline, size = SHARE_LINE
    tagline = Layer(WIDTH, HEIGHT)
    tagline.fill(text_contours("line", (WIDTH - text_width("line", size)) / 2, baseline, size))
    layers: list[tuple[Layer, Colour | Paint | None]] = [
        glow(),
        (band, None),
        *mark_layers(left, top, side),
        (name, NAME_COLOUR),
        (tagline, LINE_COLOUR),
        *logo_layers(),
    ]
    return compose(WIDTH, HEIGHT, layers)


# --- PNG -----------------------------------------------------------------------------------

SIGNATURE = b"\x89PNG\r\n\x1a\n"
# Eight bits per sample, colour type 2 (RGB), standard compression, filter and no interlace.
HEADER_LAYOUT = ">IIBBBBB"
HEADER = struct.pack(HEADER_LAYOUT, WIDTH, HEIGHT, 8, 2, 0, 0, 0)
HEADER_FIELDS = (
    "width",
    "height",
    "bit depth",
    "colour type",
    "compression method",
    "filter method",
    "interlace method",
)
# A row of the image: its filter byte, then three samples of eight bits for each pixel.
ROW_LENGTH = 1 + WIDTH * 3
# The four bytes of a chunk's length, the four of its type, and the four of its CRC.
CHUNK_FRAME = 12
PLAIN_CHUNKS = (b"IHDR", b"IDAT", b"IEND")
# What the check prints after a refusal's cause: how to write the file it wants.
REMEDY = "run python3 tools/share_image.py"


def _chunk(kind: bytes, data: bytes) -> bytes:
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))


def png(rows: bytes) -> bytes:
    """A PNG file of the rows: a header, the compressed rows and an end, nothing else."""
    return (
        SIGNATURE
        + _chunk(b"IHDR", HEADER)
        + _chunk(b"IDAT", zlib.compress(rows, 9))
        + _chunk(b"IEND", b"")
    )


def _chunk_at(data: bytes, at: int) -> tuple[bytes, bytes, bool, int] | None:
    """The chunk that starts at an offset of a PNG file: its type, its data, whether its CRC
    is the CRC of its type and data, and the offset after it. ``None`` when the file ends
    before the chunk does."""
    if at + CHUNK_FRAME > len(data):
        return None
    (length,) = struct.unpack(">I", data[at : at + 4])
    end = at + CHUNK_FRAME + length
    if end > len(data):
        return None
    kind, body = data[at + 4 : at + 8], data[at + 8 : end - 4]
    (crc,) = struct.unpack(">I", data[end - 4 : end])
    return kind, body, crc == zlib.crc32(kind + body), end


def chunks(data: bytes) -> list[tuple[bytes, bytes]]:
    """The chunks of a PNG file in order, each its type and its data. ``ValueError`` when
    the file does not start with the PNG signature, when a chunk is cut short and when a
    chunk's CRC is not the CRC of its type and data."""
    if not data.startswith(SIGNATURE):
        raise ValueError("not a PNG file")
    found: list[tuple[bytes, bytes]] = []
    at = len(SIGNATURE)
    while at < len(data):
        read = _chunk_at(data, at)
        if read is None:
            raise ValueError("a chunk is cut short")
        kind, body, matches, at = read
        if not matches:
            raise ValueError(f"the CRC of a {kind!r} chunk does not match")
        found.append((kind, body))
    return found


def pixels(data: bytes) -> bytes:
    """The rows a PNG file holds: its IDAT data joined and decompressed, each row led by
    its filter byte as the file has it. The rows of one image saved with another row filter
    are other bytes. ``ValueError`` when the compressed stream does not end where the IDAT
    data ends."""
    stream = zlib.decompressobj()
    rows = stream.decompress(b"".join(body for kind, body in chunks(data) if kind == b"IDAT"))
    if not stream.eof or stream.unused_data:
        raise ValueError("the compressed rows do not end where the IDAT data ends")
    return rows


def size(data: bytes) -> tuple[int, int]:
    """The width and height a PNG file declares."""
    header = dict(chunks(data))[b"IHDR"]
    width, height = struct.unpack(">II", header[:8])
    return width, height


def kind_name(kind: bytes) -> str:
    """A chunk's type as a message shows it: its four letters, or its bytes in hexadecimal
    when they are not four ASCII letters, so no byte of the file reaches a terminal as is."""
    if len(kind) == 4 and kind.isascii() and kind.isalpha():
        return kind.decode("ascii")
    return f"0x{kind.hex()}"


def header_change(header: bytes) -> str:
    """How a header differs from the one this script writes, field by field."""
    if len(header) != len(HEADER):
        return f"of {len(header)} bytes, not {len(HEADER)}"
    found = struct.unpack(HEADER_LAYOUT, header)
    wanted = struct.unpack(HEADER_LAYOUT, HEADER)
    return "; ".join(
        f"{name} {value}, not {want}"
        for name, value, want in zip(HEADER_FIELDS, found, wanted, strict=True)
        if value != want
    )


def _layout_refusal(data: bytes) -> str | list[tuple[bytes, bytes]]:
    """The chunks of a file up to its IEND chunk, or why they are not the ones this script
    writes. Reading stops at IEND: what follows it is data after the end, whatever it is."""
    if not data.startswith(SIGNATURE):
        return "is not a PNG file"
    found: list[tuple[bytes, bytes]] = []
    at = len(SIGNATURE)
    while not found or found[-1][0] != b"IEND":
        read = _chunk_at(data, at)
        if read is None:
            return "is cut short (it ends before its IEND chunk)"
        kind, body, matches, at = read
        if not matches:
            return f"has a bad checksum (in its {kind_name(kind)} chunk)"
        found.append((kind, body))
    if at < len(data):
        return "has data after the end (after its IEND chunk)"
    kinds = [kind for kind, _ in found]
    extra = [kind for kind in kinds if kind not in PLAIN_CHUNKS]
    if extra:
        return f"has an extra chunk ({kind_name(extra[0])})"
    if kinds[0] != b"IHDR":
        return f"has another header (its first chunk is {kind_name(kinds[0])}, not IHDR)"
    if kinds.count(b"IHDR") > 1:
        return "has an extra chunk (a second IHDR)"
    if b"IDAT" not in kinds:
        return "has no rows (no IDAT chunk)"
    if found[-1][1]:
        return "has data in its IEND chunk"
    if found[0][1] != HEADER:
        return f"has another header ({header_change(found[0][1])})"
    return found


def _rows_refusal(stream: bytes) -> str | None:
    """Why the joined IDAT data of a file with this script's header are not the rows it
    writes, or ``None`` when they are."""
    rows_stream = zlib.decompressobj()
    try:
        rows = rows_stream.decompress(stream)
    except zlib.error:
        return "has compressed rows that do not decompress"
    if not rows_stream.eof:
        return "has compressed rows cut short (their zlib stream does not end)"
    if rows_stream.unused_data:
        return "has trailing compressed data (bytes after the end of the zlib stream of its rows)"
    if len(rows) != HEIGHT * ROW_LENGTH:
        return "has rows of another length than its header gives"
    if any(rows[::ROW_LENGTH]):
        return "has another row filter (this script writes filter 0, None, on every row)"
    if rows != render():
        return "is out of date (its pixels are not the ones this script draws)"
    return None


def refusal(path: Path) -> str | None:
    """Why a file is not the PNG this script writes, as the check's message says it after the
    file's name; ``None`` when it is. ``is_current`` gives the rule.

    The file is read from its start and the first cause found is named: a missing file, not
    a PNG file, a file cut short, a bad checksum, data after the end, an extra chunk, another
    header, no rows, data in IEND, compressed rows that do not decompress or are cut short,
    trailing compressed data, rows of another length, another row filter; and last, pixels
    that are not the ones this script draws, the one cause that makes the file out of date.
    """
    if not path.is_file():
        return "is missing"
    layout = _layout_refusal(path.read_bytes())
    if isinstance(layout, str):
        return layout
    return _rows_refusal(b"".join(body for kind, body in layout if kind == b"IDAT"))


def is_current(path: Path) -> bool:
    """Whether a file is the PNG this script writes, up to how its rows are compressed.

    It must hold exactly the chunks IHDR, one or more IDAT and IEND, in that order, each
    with the CRC of its type and data, and nothing after IEND. The IHDR must be this
    script's header: its width and height, eight bits per sample, RGB, no interlace. The
    IEND must be empty. The IDAT data, joined, must be one zlib stream with nothing after
    its end, and it must decompress to the rows this script draws, each led by the filter
    byte 0 (None).

    So the same rows compressed by another zlib compressor or at another level, or split
    across several IDAT chunks, are accepted. The same pixels saved with another row filter
    are refused, as are any other chunk (text, Exif data, a time or a colour profile),
    another header, an IEND that holds data, a wrong CRC, a chunk cut short, and bytes after
    IEND or after the end of the compressed rows.
    """
    return refusal(path) is None



# --- the mark as SVG ------------------------------------------------------------------------
# Where the mark is written: the header logo (and the home page's picture) in each theme, and
# the favicon; and the README's banner in each theme.
MARK_FILES = {"light": "site/src/assets/mark-light.svg", "dark": "site/src/assets/mark-dark.svg"}
FAVICON = "site/public/favicon.svg"
BANNER_FILES = {"light": "docs/readme/banner-light.svg", "dark": "docs/readme/banner-dark.svg"}
# The website's copy of the maker's logo, which its header and footer show.
SITE_LOGO = "site/src/assets/eugo-logo.svg"


def _number(value: float) -> str:
    """A length as an SVG file writes it: no trailing zeros, no point for a whole number."""
    return f"{value:.2f}".rstrip("0").rstrip(".")


def _gradient(ident: str, ends: tuple[str, str], start: Point, end: Point) -> str:
    """A gradient from ``start`` to ``end`` in the drawing's own units."""
    return (
        f'<linearGradient id="{ident}" gradientUnits="userSpaceOnUse" x1="{_number(start[0])}" '
        f'y1="{_number(start[1])}" x2="{_number(end[0])}" y2="{_number(end[1])}">'
        f'<stop offset="0" stop-color="{ends[0]}"/><stop offset="1" stop-color="{ends[1]}"/></linearGradient>'
    )


def _mark_body(theme: str, prefix: str = "") -> str:
    """The mark's definitions and drawing in one theme, its ids led by ``prefix``."""
    look = THEMES[theme]
    p = f"{prefix}gn{theme[0]}"
    (mx, my, mr), (lx, ly, lr) = MOON, LIT
    (ax, ay), (bx, by) = ORBIT_ENDS
    flood, opacity = look["shadow"]
    x, y, w, h = SHADOW_REGION
    n = _number
    defs = (
        _gradient(f"{p}n", look["violet"], (0, my - mr), (0, my + mr))
        + _gradient(f"{p}l", look["azure"], (0, my - mr), (0, my + mr))
        + _gradient(f"{p}o", look["orbit"], *ORBIT_GRADIENT)
        + f'<filter id="{p}s" filterUnits="userSpaceOnUse" x="{n(x)}" y="{n(y)}" width="{n(w)}" '
        f'height="{n(h)}" color-interpolation-filters="sRGB"><feDropShadow dx="0" '
        f'dy="{n(SHADOW_OFFSET)}" stdDeviation="{n(SHADOW_DEVIATION)}" flood-color="{flood}" '
        f'flood-opacity="{n(opacity)}"/></filter>'
        f'<clipPath id="{p}c"><circle cx="{n(mx)}" cy="{n(my)}" r="{n(mr)}"/></clipPath>'
        f'<mask id="{p}k" maskUnits="userSpaceOnUse" x="0" y="0" width="64" height="64">'
        f'<rect width="64" height="64" fill="#fff"/>'
        f'<circle cx="{n(mx)}" cy="{n(my)}" r="{n(mr + GAP)}" fill="#000"/></mask>'
    )
    r = n(ORBIT_RADIUS)
    body = (
        f'<g filter="url(#{p}s)"><g transform="translate({n(OFFSET[0])} {n(OFFSET[1])})">'
        f'<path d="M{n(ax)} {n(ay)}A{r} {r} 0 0 1 {n(bx)} {n(by)}" fill="none" stroke="url(#{p}o)" '
        f'stroke-width="{n(ORBIT_WIDTH)}" stroke-linecap="round" mask="url(#{p}k)"/>'
        f'<circle cx="{n(mx)}" cy="{n(my)}" r="{n(mr)}" fill="url(#{p}n)"/>'
        f'<g clip-path="url(#{p}c)"><circle cx="{n(lx)}" cy="{n(ly)}" r="{n(lr)}" fill="url(#{p}l)"/></g></g></g>'
    )
    return f"<defs>{defs}</defs>{body}"


def mark_svg(theme: str) -> str:
    """The mark in one theme's colours, as the header and the home page show it."""
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" role="img" '
        f'aria-label="{NAME}">{_mark_body(theme)}</svg>\n'
    )


def favicon_svg() -> str:
    """The mark as the favicon: the light theme's drawing, the dark theme's when the reader's
    system asks for a dark colour scheme."""
    style = ".d{display:none}@media (prefers-color-scheme:dark){.l{display:none}.d{display:inline}}"
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64"><style>{style}</style>'
        f'<g class="l">{_mark_body("light", "f")}</g><g class="d">{_mark_body("dark", "f")}</g></svg>\n'
    )


# --- the README's banner ------------------------------------------------------------------
# The banner at the top of README.md, for GitHub's light and dark themes: a framed panel with a
# band of the accent gradient along its top, the mark beside the name as the header sets them,
# and the share image's line under them. Its letters are outlines, so it needs no font.
BANNER_WIDTH, BANNER_HEIGHT = 1200, 260
# Where the banner sets the name (its centre, its baseline and its size) and the line (its
# baseline and its size).
BANNER_NAME = (BANNER_WIDTH / 2, 128.0, 88.0)
BANNER_LINE = (196.0, 30.0)
# The maker's logo in the banner's top right corner: its width, and its distance from the top and
# from the right edge. The ids of its gradients take this prefix, to be the banner's own.
BANNER_LOGO = (110.0, 26.0, 30.0)
BANNER_IDS = "gnBanner-"


def _logo() -> str:
    """The maker's logo file, nested as it is in the banner's top right corner, its ids made the
    banner's own: the same drawing, in its own gradients, on either theme."""
    text, view, _ = logo()
    _, _, box_width, box_height = (float(n) for n in view.split())
    inner = text.split(">", 1)[1].rsplit("</svg>", 1)[0].strip()
    inner = re.sub(r'id="([^"]+)"', rf'id="{BANNER_IDS}\1"', inner)
    inner = re.sub(r"url\(#([^)]+)\)", rf"url(#{BANNER_IDS}\1)", inner)
    width, top, right = BANNER_LOGO
    place = (f'x="{BANNER_WIDTH - right - width:.2f}" y="{top:.2f}" width="{width:.2f}" '
             f'height="{width * box_height / box_width:.2f}"')
    return f'<svg {place} viewBox="{view}">{inner}</svg>'


def _band_stops() -> str:
    """The stops of the band's gradient, as an SVG gradient writes them."""
    return "".join(
        f'<stop offset="{_number(at / 100)}" stop-color="#{r:02x}{g:02x}{b:02x}"/>'
        for (r, g, b), at in GRADIENT
    )


def banner_svg(theme: str) -> str:
    """The README's banner in one theme."""
    look = THEMES[theme]
    w, h = BANNER_WIDTH, BANNER_HEIGHT
    left, top, side, name_left, spacing = wordmark(*BANNER_NAME)
    baseline, size = BANNER_LINE
    line_left = (w - text_width("line", size)) / 2
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" '
        f'role="img" aria-label="{NAME}: {LINE}. Made by Eugo">'
        '<defs><linearGradient id="gnBannerRule" x1="0" y1="0" x2="1" y2="0">'
        f"{_band_stops()}"
        f'</linearGradient><clipPath id="gnBannerFrame"><rect x="1" y="1" width="{w - 2}" height="{h - 2}" rx="18"/>'
        "</clipPath></defs>"
        f'<rect x="1" y="1" width="{w - 2}" height="{h - 2}" rx="18" fill="{look["ground"]}" '
        f'stroke="{look["border"]}" stroke-width="2"/>'
        f'<rect x="0" y="0" width="{w}" height="6" fill="url(#gnBannerRule)" clip-path="url(#gnBannerFrame)"/>'
        f'<svg x="{left:.2f}" y="{top:.2f}" width="{side:.2f}" height="{side:.2f}" viewBox="0 0 64 64">'
        f"{_mark_body(theme, 'b')}</svg>"
        f"{_logo()}"
        f'<path fill="{look["ink"]}" d="{text_path("name", name_left, BANNER_NAME[1], BANNER_NAME[2], spacing)}"/>'
        f'<path fill="{look["muted"]}" d="{text_path("line", line_left, baseline, size)}"/>'
        "</svg>\n"
    )


def mark_files() -> dict[str, str]:
    """Every SVG file this script writes, by its path in the repository, with its text: the
    mark in both themes, the favicon, the README's banner in both themes, and the website's copy
    of the maker's logo, the same bytes as ``docs/readme/eugo-logo.svg``."""
    files = {MARK_FILES[theme]: mark_svg(theme) for theme in ("light", "dark")}
    files[FAVICON] = favicon_svg()
    for theme, path in BANNER_FILES.items():
        files[path] = banner_svg(theme)
    files[SITE_LOGO] = logo()[0]
    return files


def mark_refusal(repo: Path = REPO) -> str | None:
    """Why the SVG files are not the ones this script writes, or ``None`` when they are."""
    for path, text in mark_files().items():
        file = repo / path
        if not file.is_file():
            return f"{path} is missing"
        if file.read_text(encoding="utf-8") != text:
            return f"{path} is out of date"
    return None


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="share_image.py", description=__doc__.split("\n")[0])
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--check", action="store_true", help="exit 1 when a file differs")
    group.add_argument("--letters", action="store_true",
                       help=f"shape the letters again into {LETTERS}, then write every file")
    args = parser.parse_args(argv)
    path = REPO / OUTPUT
    if args.check:
        cause = refusal(path)
        if cause:
            print(f"{OUTPUT} {cause}: {REMEDY}", file=sys.stderr)
            return 1
        cause = mark_refusal(REPO)
        if cause:
            print(f"{cause}: {REMEDY}", file=sys.stderr)
            return 1
        return 0
    if args.letters:
        (REPO / LETTERS).write_text(letters_text(shape_letters()), encoding="utf-8")
        letters.cache_clear()
    for name, text in mark_files().items():
        (REPO / name).parent.mkdir(parents=True, exist_ok=True)
        (REPO / name).write_text(text, encoding="utf-8")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(png(render()))
    return 0


if __name__ == "__main__":
    sys.exit(main())

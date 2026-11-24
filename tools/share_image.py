# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo, Inc.
"""Draw the ganymede mark and the share image of the website.

The mark is a set of rounded tiles in violet and teal on a 64-unit grid, in the style of Eugo's
other documentation sites: three notebook cells and a moon. ``MARKS`` holds each proposal as data
(``MARK`` names the one in use), and this script writes it, for the light and the dark theme, to
``site/src/assets/mark-light.svg`` and ``mark-dark.svg``, the other proposals beside them, and
``site/public/favicon.svg``, which follows the reader's colour scheme.

The share image, ``site/public/assets/social-preview.png``, is what a page shared on a social site
or in a chat shows beside its title. It holds a band of the site's accent gradient along its top,
the mark in its dark colours, the name ganymede and one line on what it holds, on the dark ground
of the site. The mark is drawn as its file draws it: a soft shadow straight down under the tiles,
each tile shaded from its top to its bottom in its own colour, and a faint white line just inside
each tile's edge that fades out before the middle of the tile.

The drawing uses only the standard library and arithmetic that gives the same result on
every machine: the letters are strokes of a small built-in font, made of straight pieces
and of arcs whose points come from square roots, never from the system's fonts or its
trigonometry. The shadow is blurred by three passes of a box filter, whose widths come
from a square root and whose sums are whole numbers, never by the exponential of a Gaussian.
The build copies the committed files as they are, once ``refusal`` finds the image is the file
this script writes, up to how its rows are compressed, and ``mark_refusal`` finds the SVG files
are the ones it writes; ``is_current`` says exactly what is accepted and refused.

Usage::

    python3 tools/share_image.py          # write the marks, the favicon and the share image
    python3 tools/share_image.py --check  # exit 1, naming the cause, when a file differs
"""

from __future__ import annotations

import argparse
import itertools
import math
import struct
import sys
import zlib
from array import array
from collections.abc import Callable, Iterable, Sequence
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
OUTPUT = "site/public/assets/social-preview.png"
WIDTH = 1200
HEIGHT = 630
NAME = "ganymede"
LINE = "Open notebooks for HPC, Ray and geospatial Python"
ALT = (
    f"The name {NAME} in white beside its mark of rounded tiles and a circle in violet and teal, "
    f"above the line “{LINE}”, on a dark background under a violet-to-teal band."
)

Point = tuple[float, float]
Stroke = list[Point]
Colour = tuple[int, int, int]
# The colour of a layer at a pixel, given its column and its row.
Paint = Callable[[int, int], Colour]

# The dark palette of the site: its ground, its text, a softer text, and the two ends of its
# gradient.
BACKGROUND: Colour = (0x08, 0x0D, 0x17)
NAME_COLOUR: Colour = (0xF2, 0xF3, 0xF8)
LINE_COLOUR: Colour = (0xA9, 0xB0, 0xC3)
GRADIENT: tuple[Colour, Colour] = ((0xA0, 0x3F, 0xFF), (0x00, 0x82, 0x94))
BAND_HEIGHT = 10

# The mark, in its 64-unit space: each tile is a box (left, top, width, height), its corner
# radius (half its width makes a circle) and its colour. Proposal A: two violet cells above a
# teal cell and a teal moon. Proposal B: one tall violet cell beside a teal cell and a teal moon.
# Every tile is shaded from its top colour to its bottom one, over its own height.
Tile = tuple[float, float, float, float, float, str]
MARKS: dict[str, tuple[Tile, ...]] = {
    "a": (
        (4, 4, 26, 26, 7, "violet"),
        (34, 4, 26, 26, 7, "violet"),
        (4, 34, 26, 26, 7, "teal"),
        (34, 34, 26, 26, 13, "teal"),
    ),
    "b": (
        (4, 4, 26, 56, 7, "violet"),
        (34, 4, 26, 26, 7, "teal"),
        (34, 34, 26, 26, 13, "teal"),
    ),
}
# The proposal in use: the files mark-light.svg, mark-dark.svg and favicon.svg and the share
# image draw it; each other proposal is written as mark-<name>-light.svg and -dark.svg.
MARK = "a"
# The colours of the tiles in the dark theme, top and bottom (the share image is dark), and in
# the light theme.
DARK: dict[str, tuple[Colour, Colour]] = {
    "violet": ((0xB0, 0x61, 0xFF), (0x9C, 0x38, 0xFF)),
    "teal": ((0x3D, 0xCE, 0xE4), (0x1E, 0xC1, 0xD9)),
}
LIGHT: dict[str, tuple[Colour, Colour]] = {
    "violet": ((0x8F, 0x56, 0xF0), (0x76, 0x31, 0xEC)),
    "teal": ((0x05, 0x9B, 0xB4), (0x04, 0x78, 0x8C)),
}
MARK_UNITS = 64.0
# The shadow: black, straight down by its offset, blurred with its standard deviation, and
# cut to the mark's 64 units, the region of the file's filter.
SHADOW: Colour = (0x00, 0x00, 0x00)
SHADOW_OPACITY = 0.55
SHADOW_OFFSET = 1.2
SHADOW_DEVIATION = 1.2
# The highlight: a white line one unit wide, half a unit inside each tile's edge, at its
# opacity at the top, fading to nothing over its share of the line's height.
HIGHLIGHT: Colour = (0xFF, 0xFF, 0xFF)
HIGHLIGHT_OPACITY = 0.22
HIGHLIGHT_WIDTH = 1.0
HIGHLIGHT_REACH = 0.45
# The box passes that blur the shadow, and the whole steps of coverage the blur sums.
BLUR_PASSES = 3
COVER_STEPS = 1024

# --- directions without trigonometry -------------------------------------------------------

_R2 = math.sqrt(2.0)
_R3 = math.sqrt(3.0)
_R6 = math.sqrt(6.0)
# The cosine and sine of 0, 15, 30, 45, 60, 75 and 90 degrees, from square roots alone.
_FIRST_QUADRANT: tuple[Point, ...] = (
    (1.0, 0.0),
    ((_R6 + _R2) / 4.0, (_R6 - _R2) / 4.0),
    (_R3 / 2.0, 0.5),
    (_R2 / 2.0, _R2 / 2.0),
    (0.5, _R3 / 2.0),
    ((_R6 - _R2) / 4.0, (_R6 + _R2) / 4.0),
    (0.0, 1.0),
)
STEP_DEGREES = 15
PIECES_PER_STEP = 4


def direction(degrees: int) -> Point:
    """The unit vector at an angle that is a multiple of 15 degrees, counter-clockwise."""
    if degrees % STEP_DEGREES:
        raise ValueError(f"an angle must be a multiple of {STEP_DEGREES} degrees: {degrees}")
    turn = degrees % 360
    quadrant, rest = divmod(turn, 90)
    x, y = _FIRST_QUADRANT[rest // STEP_DEGREES]
    for _ in range(quadrant):
        x, y = -y, x
    return x, y


def _between(a: Point, b: Point, t: float) -> Point:
    """A point on the unit circle between two unit vectors, by a normalised straight mix."""
    x = a[0] + (b[0] - a[0]) * t
    y = a[1] + (b[1] - a[1]) * t
    length = math.sqrt(x * x + y * y)
    return x / length, y / length


def arc(cx: float, cy: float, rx: float, ry: float, start: int, end: int) -> Stroke:
    """The points of an elliptical arc from ``start`` to ``end`` degrees, counter-clockwise
    when ``end`` is larger, clockwise when it is smaller."""
    sign = 1 if end >= start else -1
    angles = list(range(start, end + sign, sign * STEP_DEGREES))
    points: Stroke = []
    for first, second in itertools.pairwise(angles):
        a, b = direction(first), direction(second)
        for piece in range(PIECES_PER_STEP):
            ux, uy = _between(a, b, piece / PIECES_PER_STEP)
            points.append((cx + rx * ux, cy + ry * uy))
    ux, uy = direction(angles[-1])
    points.append((cx + rx * ux, cy + ry * uy))
    return points


def line(*points: Point) -> Stroke:
    return list(points)


# --- the font ------------------------------------------------------------------------------
# Units: the baseline is y = 0, the height of a small letter is 5, of a tall one 7.5, and a
# descender goes down to -2.5. Each glyph is its width and its strokes.

X = 5.0
TALL = 7.5
DOWN = -2.5
R = X / 2


def _bowl() -> Stroke:
    return arc(R, R, R, R, 0, 360)


GLYPHS: dict[str, tuple[float, list[Stroke]]] = {
    "a": (X, [_bowl(), line((X, 0.0), (X, X))]),
    "b": (X, [_bowl(), line((0.0, 0.0), (0.0, TALL))]),
    "c": (X, [arc(R, R, R, R, 45, 315)]),
    "d": (X, [_bowl(), line((X, 0.0), (X, TALL))]),
    "e": (X, [line((0.0, R), (X, R)), arc(R, R, R, R, 0, 315)]),
    "f": (
        3.5,
        [
            line((1.25, 0.0), (1.25, 5.75)),
            arc(3.0, 5.75, 1.75, 1.75, 180, 45),
            line((0.0, X), (3.25, X)),
        ],
    ),
    "h": (X, [line((0.0, 0.0), (0.0, TALL)), arc(R, R, R, R, 180, 0), line((X, R), (X, 0.0))]),
    "i": (0.0, [line((0.0, 0.0), (0.0, X)), line((0.0, 7.0), (0.0, 7.0))]),
    "k": (
        4.25,
        [
            line((0.0, 0.0), (0.0, TALL)),
            line((4.0, X), (0.0, 1.5)),
            line((1.45, 2.7), (4.25, 0.0)),
        ],
    ),
    "l": (0.0, [line((0.0, 0.0), (0.0, TALL))]),
    "m": (
        8.0,
        [
            line((0.0, 0.0), (0.0, X)),
            arc(2.0, 3.0, 2.0, 2.0, 180, 0),
            line((4.0, 3.0), (4.0, 0.0)),
            arc(6.0, 3.0, 2.0, 2.0, 180, 0),
            line((8.0, 3.0), (8.0, 0.0)),
        ],
    ),
    "n": (X, [line((0.0, 0.0), (0.0, X)), arc(R, R, R, R, 180, 0), line((X, R), (X, 0.0))]),
    "o": (X, [_bowl()]),
    "p": (X, [_bowl(), line((0.0, DOWN), (0.0, X))]),
    "r": (3.5, [line((0.0, 0.0), (0.0, X)), arc(R, R, R, R, 180, 60)]),
    "s": (
        3.9,
        [
            arc(1.95, 3.75, 1.95, 1.25, 30, 270),
            arc(1.95, 1.25, 1.95, 1.25, 90, -150),
        ],
    ),
    "T": (5.5, [line((0.0, TALL), (5.5, TALL)), line((2.75, 0.0), (2.75, TALL))]),
    " ": (1.0, []),
    "g": (X, [_bowl(), line((X, X), (X, -0.5)), arc(R, -0.5, R, 2.0, 0, -150)]),
    "y": (X, [line((0.0, X), (R, 0.0)), line((X, X), (1.25, DOWN))]),
    "t": (
        3.5,
        [
            line((1.25, 6.75), (1.25, 1.25)),
            arc(2.75, 1.25, 1.5, 1.25, 180, 315),
            line((0.0, X), (3.25, X)),
        ],
    ),
    "O": (6.5, [arc(3.25, 3.75, 3.25, 3.75, 0, 360)]),
    "C": (6.5, [arc(3.25, 3.75, 3.25, 3.75, 45, 315)]),
    "H": (
        5.5,
        [
            line((0.0, 0.0), (0.0, TALL)),
            line((5.5, 0.0), (5.5, TALL)),
            line((0.0, 3.75), (5.5, 3.75)),
        ],
    ),
    "P": (
        5.0,
        [
            line((0.0, 0.0), (0.0, TALL)),
            line((0.0, TALL), (3.0, TALL)),
            arc(3.0, 5.5, 2.0, 2.0, 90, -90),
            line((3.0, 3.5), (0.0, 3.5)),
        ],
    ),
    "R": (
        5.0,
        [
            line((0.0, 0.0), (0.0, TALL)),
            line((0.0, TALL), (3.0, TALL)),
            arc(3.0, 5.5, 2.0, 2.0, 90, -90),
            line((3.0, 3.5), (0.0, 3.5)),
            line((2.5, 3.5), (5.0, 0.0)),
        ],
    ),
    ",": (0.5, [line((0.5, 0.25), (-0.25, -1.5))]),
}
GAP = 1.75


def text_width(text: str) -> float:
    """The width of a line of text in font units, from the first stroke to the last."""
    widths = [GLYPHS[letter][0] for letter in text]
    return sum(widths) + GAP * (len(text) - 1)


def text_strokes(text: str, left: float, baseline: float, unit: float) -> list[Stroke]:
    """The strokes of a line of text in pixels, its first letter at ``left``."""
    strokes: list[Stroke] = []
    pen = 0.0
    for letter in text:
        width, glyph = GLYPHS[letter]
        for stroke in glyph:
            strokes.append([(left + (pen + x) * unit, baseline - y * unit) for x, y in stroke])
        pen += width + GAP
    return strokes


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

    def strokes(self, strokes: Iterable[Stroke], thickness: float) -> None:
        """Draw strokes with round ends, ``thickness`` pixels wide, smoothed at the edges."""
        half = thickness / 2
        for stroke in strokes:
            pairs = list(itertools.pairwise(stroke)) or [(stroke[0], stroke[0])]
            for a, b in pairs:
                self._segment(a, b, half)

    def _segment(self, a: Point, b: Point, half: float) -> None:
        reach = half + 1.0
        x0 = max(0, math.floor(min(a[0], b[0]) - reach))
        x1 = min(self.width - 1, math.ceil(max(a[0], b[0]) + reach))
        y0 = max(0, math.floor(min(a[1], b[1]) - reach))
        y1 = min(self.height - 1, math.ceil(max(a[1], b[1]) + reach))
        dx, dy = b[0] - a[0], b[1] - a[1]
        length = dx * dx + dy * dy
        for y in range(y0, y1 + 1):
            py = y + 0.5
            for x in range(x0, x1 + 1):
                px = x + 0.5
                t = 0.0
                if length > 0.0:
                    t = ((px - a[0]) * dx + (py - a[1]) * dy) / length
                    t = 0.0 if t < 0.0 else 1.0 if t > 1.0 else t
                ex, ey = px - (a[0] + t * dx), py - (a[1] + t * dy)
                amount = half + 0.5 - math.sqrt(ex * ex + ey * ey)
                if amount > 0.0:
                    self._put(x, y, 1.0 if amount > 1.0 else amount)

    def rounded_box(
        self, left: float, top: float, width: float, height: float, radius: float
    ) -> None:
        """Fill a box with rounded corners, smoothed at the edges."""
        cx, cy = left + width / 2, top + height / 2
        hx, hy = width / 2 - radius, height / 2 - radius
        for y in range(max(0, int(top) - 1), min(self.height, int(top + height) + 2)):
            for x in range(max(0, int(left) - 1), min(self.width, int(left + width) + 2)):
                qx = abs(x + 0.5 - cx) - hx
                qy = abs(y + 0.5 - cy) - hy
                ox, oy = max(qx, 0.0), max(qy, 0.0)
                inside = min(max(qx, qy), 0.0)
                distance = math.sqrt(ox * ox + oy * oy) + inside - radius
                amount = 0.5 - distance
                if amount > 0.0:
                    self._put(x, y, 1.0 if amount > 1.0 else amount)

    def rounded_ring(
        self, left: float, top: float, width: float, height: float, radius: float, thickness: float
    ) -> None:
        """Draw the outline of a box with rounded corners, a line ``thickness`` pixels wide
        centred on the box's edge, smoothed at its sides."""
        half = thickness / 2
        cx, cy = left + width / 2, top + height / 2
        hx, hy = width / 2 - radius, height / 2 - radius
        reach = int(half) + 2
        for y in range(max(0, int(top) - reach), min(self.height, int(top + height) + reach + 1)):
            for x in range(
                max(0, int(left) - reach), min(self.width, int(left + width) + reach + 1)
            ):
                qx = abs(x + 0.5 - cx) - hx
                qy = abs(y + 0.5 - cy) - hy
                ox, oy = max(qx, 0.0), max(qy, 0.0)
                inside = min(max(qx, qy), 0.0)
                distance = math.sqrt(ox * ox + oy * oy) + inside - radius
                amount = half + 0.5 - abs(distance)
                if amount > 0.0:
                    self._put(x, y, 1.0 if amount > 1.0 else amount)

    def fade(self, top: float, height: float, opacity: float, reach: float) -> None:
        """Scale the cover of every row for a line ``height`` pixels high from ``top``: by
        ``opacity`` down to the top, then in a straight line to nothing at ``reach`` of the
        height, and nothing below."""
        end = reach * height
        for y in range(self.height):
            down = y + 0.5 - top
            scale = opacity if down <= 0.0 else opacity * (1.0 - down / end) if down < end else 0.0
            start = y * self.width
            for index in range(start, start + self.width):
                if self.cover[index] > 0.0:
                    self.cover[index] *= scale


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
    """The colour of the gradient at a column: each channel goes from the first end to the
    second in a straight line, rounded to the nearest whole value, the same on every machine."""
    (r0, g0, b0), (r1, g1, b1) = GRADIENT
    last = width - 1
    return (
        (r0 * (last - x) + r1 * x + last // 2) // last,
        (g0 * (last - x) + g1 * x + last // 2) // last,
        (b0 * (last - x) + b1 * x + last // 2) // last,
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


def render() -> bytes:
    """The image as rows of RGB pixels, each row led by its filter byte."""
    name_unit, line_unit = 13.0, 3.7
    icon = 168.0
    gap = 44.0
    name_width = text_width(NAME) * name_unit
    row_width = icon + gap + name_width
    left = (WIDTH - row_width) / 2
    icon_top = 150.0
    name_baseline = icon_top + icon / 2 + X * name_unit / 2
    line_width = text_width(LINE) * line_unit
    line_baseline = 470.0

    band = Layer(WIDTH, HEIGHT)
    band.rounded_box(0, 0, WIDTH, BAND_HEIGHT, 0)
    # The mark on its 64-unit grid, each tile in its own layer so that it is shaded and
    # highlighted over its own height; the shadow is cast by all the tiles together.
    scale = icon / MARK_UNITS
    lowered = Layer(WIDTH, HEIGHT)
    tiles: list[tuple[Layer, Paint]] = []
    highlights: list[Layer] = []
    inset = HIGHLIGHT_WIDTH / 2 * scale
    for tx, ty, tw, th, tr, colour in MARKS[MARK]:
        x, y = left + tx * scale, icon_top + ty * scale
        w, h, radius = tw * scale, th * scale, tr * scale
        layer = Layer(WIDTH, HEIGHT)
        layer.rounded_box(x, y, w, h, radius)
        tiles.append((layer, shade(DARK[colour], y, h)))
        lowered.rounded_box(x, y + SHADOW_OFFSET * scale, w, h, radius)
        highlight = Layer(WIDTH, HEIGHT)
        highlight.rounded_ring(
            x + inset, y + inset, w - 2 * inset, h - 2 * inset, radius - inset, HIGHLIGHT_WIDTH * scale
        )
        highlight.fade(y + inset, h - 2 * inset, HIGHLIGHT_OPACITY, HIGHLIGHT_REACH)
        highlights.append(highlight)
    # The shadow's region is the mark's square: the pixels whose middles lie inside it.
    region = (
        math.ceil(left - 0.5),
        math.ceil(icon_top - 0.5),
        math.ceil(left + icon - 0.5),
        math.ceil(icon_top + icon - 0.5),
    )
    shadow = blurred_shadow(lowered, region, SHADOW_DEVIATION * scale)
    name = Layer(WIDTH, HEIGHT)
    name.strokes(text_strokes(NAME, left + icon + gap, name_baseline, name_unit), 1.15 * name_unit)
    tagline = Layer(WIDTH, HEIGHT)
    tagline.strokes(
        text_strokes(LINE, (WIDTH - line_width) / 2, line_baseline, line_unit), 0.85 * line_unit
    )
    layers: list[tuple[Layer, Colour | Paint | None]] = [
        (band, None),
        (shadow, SHADOW),
        *tiles,
        *((highlight, HIGHLIGHT) for highlight in highlights),
        (name, NAME_COLOUR),
        (tagline, LINE_COLOUR),
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
# Where the mark is written: the header logo in each theme, and the favicon.
MARK_FILES = {"light": "site/src/assets/mark-light.svg", "dark": "site/src/assets/mark-dark.svg"}
FAVICON = "site/public/favicon.svg"


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
# The shadow and the highlight of each theme: the shadow's blur, colour and opacity, and the
# highlight's opacity at the top of a tile.
SVG_THEMES = {
    "light": {"blur": "1.1", "flood": "#1b1035", "opacity": "0.22", "highlight": "0.35"},
    "dark": {"blur": "1.2", "flood": "#000000", "opacity": "0.55", "highlight": "0.22"},
}


def _number(value: float) -> str:
    """A length as an SVG file writes it: no trailing zeros, no point for a whole number."""
    return f"{value:.2f}".rstrip("0").rstrip(".")


def _hex(colour: Colour) -> str:
    return "#{:02x}{:02x}{:02x}".format(*colour)


def _tiles_svg(tiles: Sequence[Tile], fill: Callable[[str], str], ring: str) -> tuple[str, str]:
    """The tiles and, over them, the highlight just inside each tile's edge."""
    boxes = "".join(
        f'<rect x="{_number(x)}" y="{_number(y)}" width="{_number(w)}" height="{_number(h)}" '
        f'rx="{_number(r)}" fill="{fill(colour)}"/>'
        for x, y, w, h, r, colour in tiles
    )
    rings = "".join(
        f'<rect x="{_number(x + 0.5)}" y="{_number(y + 0.5)}" width="{_number(w - 1)}" '
        f'height="{_number(h - 1)}" rx="{_number(r - 0.5)}"/>'
        for x, y, w, h, r, _colour in tiles
    )
    return boxes, f'<g fill="none" stroke="{ring}" stroke-width="1">{rings}</g>'


def _stops(ends: tuple[Colour, Colour]) -> str:
    return f'<stop offset="0" stop-color="{_hex(ends[0])}"/><stop offset="1" stop-color="{_hex(ends[1])}"/>'


def mark_svg(name: str, theme: str) -> str:
    """The mark ``name`` in one theme's colours, as the header shows it."""
    look = SVG_THEMES[theme]
    colours = LIGHT if theme == "light" else DARK
    ident = f"gn{name.upper()}{theme[0].upper()}"
    gradients = "".join(
        f'<linearGradient id="{ident}{colour[0]}" x1="0" y1="0" x2="0" y2="1">{_stops(ends)}</linearGradient>'
        for colour, ends in colours.items()
    )
    boxes, rings = _tiles_svg(MARKS[name], lambda c: f"url(#{ident}{c[0]})", f"url(#{ident}r)")
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" role="img" aria-label="{NAME}">'
        f'<defs><filter id="{ident}s" filterUnits="userSpaceOnUse" x="0" y="0" width="64" height="64" '
        'color-interpolation-filters="sRGB">'
        f'<feDropShadow dx="0" dy="1.2" stdDeviation="{look["blur"]}" flood-color="{look["flood"]}" '
        f'flood-opacity="{look["opacity"]}"/></filter>{gradients}'
        f'<linearGradient id="{ident}r" x1="0" y1="0" x2="0" y2="1">'
        f'<stop offset="0" stop-color="#ffffff" stop-opacity="{look["highlight"]}"/>'
        f'<stop offset=".45" stop-color="#ffffff" stop-opacity="0"/></linearGradient></defs>'
        f'<g filter="url(#{ident}s)">{boxes}</g>{rings}</svg>\n'
    )


def favicon_svg(name: str = MARK) -> str:
    """The mark as the favicon: the light theme's colours, the dark theme's when the reader's
    system asks for a dark colour scheme."""
    light, dark = SVG_THEMES["light"], SVG_THEMES["dark"]
    style = (
        f".v0{{stop-color:{_hex(LIGHT['violet'][0])}}}.v1{{stop-color:{_hex(LIGHT['violet'][1])}}}"
        f".t0{{stop-color:{_hex(LIGHT['teal'][0])}}}.t1{{stop-color:{_hex(LIGHT['teal'][1])}}}"
        f".r0{{stop-color:#fff;stop-opacity:{light['highlight']}}}.r1{{stop-color:#fff;stop-opacity:0}}"
        f".f{{flood-color:{light['flood']};flood-opacity:{light['opacity']}}}"
        "@media (prefers-color-scheme:dark){"
        f".v0{{stop-color:{_hex(DARK['violet'][0])}}}.v1{{stop-color:{_hex(DARK['violet'][1])}}}"
        f".t0{{stop-color:{_hex(DARK['teal'][0])}}}.t1{{stop-color:{_hex(DARK['teal'][1])}}}"
        f".r0{{stop-opacity:{dark['highlight']}}}.f{{flood-color:{dark['flood']};flood-opacity:{dark['opacity']}}}}}"
    )
    boxes, rings = _tiles_svg(MARKS[name], lambda c: f"url(#{c[0]})", "url(#r)")
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64"><style>{style}</style><defs>'
        '<linearGradient id="v" x1="0" y1="0" x2="0" y2="1"><stop offset="0" class="v0"/><stop offset="1" class="v1"/></linearGradient>'
        '<linearGradient id="t" x1="0" y1="0" x2="0" y2="1"><stop offset="0" class="t0"/><stop offset="1" class="t1"/></linearGradient>'
        '<linearGradient id="r" x1="0" y1="0" x2="0" y2="1"><stop offset="0" class="r0"/><stop offset=".45" class="r1"/></linearGradient>'
        '<filter id="s" filterUnits="userSpaceOnUse" x="0" y="0" width="64" height="64" color-interpolation-filters="sRGB">'
        f'<feDropShadow class="f" dx="0" dy="1.2" stdDeviation="{light["blur"]}"/></filter></defs>'
        f'<g filter="url(#s)">{boxes}</g>{rings}</svg>\n'
    )


def mark_files() -> dict[str, str]:
    """Every SVG file this script writes, by its path in the repository, with its text: the
    mark in use in both themes, the favicon, and each other proposal in both themes."""
    files = {MARK_FILES[theme]: mark_svg(MARK, theme) for theme in ("light", "dark")}
    files[FAVICON] = favicon_svg(MARK)
    for name in MARKS:
        if name != MARK:
            for theme in ("light", "dark"):
                files[MARK_FILES[theme].replace("mark-", f"mark-{name}-")] = mark_svg(name, theme)
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
    parser.add_argument("--check", action="store_true", help="exit 1 when the file differs")
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
    for name, text in mark_files().items():
        (REPO / name).parent.mkdir(parents=True, exist_ok=True)
        (REPO / name).write_text(text, encoding="utf-8")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(png(render()))
    return 0


if __name__ == "__main__":
    sys.exit(main())

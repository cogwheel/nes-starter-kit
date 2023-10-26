"""Convert an image to NES CHR tile data.

The input image must be 128 pixels wide and use exactly 4 colors when converted
to grayscale.  Darker colors in the source image will be given lower palette
indexes in the tile data. (TODO: make this customizable)

Any lossless file format supported by PIL should work (PNG, GIF, BMP, etc.).
"""

import argparse
from pathlib import Path

from functools import reduce
from PIL import Image

TILE_SIZE = 8

def sort_palette(image: Image) -> Image:
    stream = image.getpalette()
    colors = [stream[pos:pos+3] for pos in range(0, len(stream), 3)]
    palette = list(enumerate(sum(a * b for a,b in zip(color, [.299, .587, .114])) for color in colors))
    sequence = \
        [0] + [idx for (idx, _) in sorted(palette[1:], key=lambda clr: clr[1])] \
        if image.has_transparency_data else \
        [idx for (idx, _) in sorted(palette, key=lambda clr: clr[1])]
    return image.remap_palette(sequence)

def tile_data(image: Image) -> (bytes, bytes):
    image = image.quantize(colors=4)
    image = sort_palette(image)
    rows = [
        reduce(
            lambda a, b: (a[0] << 1 | b[0], a[1] << 1 | b[1]),
            (
                (px & 0b01, (px >> 1) & 0b01) for px in 
                (image.getpixel((x, y)) for x in range(image.width))
            ),
            (0, 0)
        )
        for y in range(image.height)
    ]
    planes = tuple(zip(*rows))
    return (bytes(stream) for stream in planes)

def transcribe_chr(image: Image, outfile: Path):
    image = image.convert('RGBA' if image.has_transparency_data else 'RGB')
    with open(outfile, "wb") as file:
        for y in range(TILE_SIZE, image.height + 1, TILE_SIZE):
            for x in range(TILE_SIZE, image.width + 1, TILE_SIZE): 
                p0, p1 = tile_data(image.crop((x - TILE_SIZE, y - TILE_SIZE, x, y)))
                file.write(p1)
                file.write(p0)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument(
        "source",
        help="Input .png file",
        type=Path,
    )
    parser.add_argument("dest", help="Output .chr file", type=Path)
    args = parser.parse_args()

    image = Image.open(args.source)
    transcribe_chr(image, args.dest)

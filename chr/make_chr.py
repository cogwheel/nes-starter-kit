"""Convert an image to NES CHR tile data.

For the most intuitive, consistent results, the input file should be 128x128,
and each 8x8 tile should contain no more than four colors, with either 
transparency (counted as a color if present) or black being interpreted as 
transparent areas of the tile.

Complete 8x8 tiles will be copied starting from the top left of the image in 
row-major order. Partial tiles at the right or bottom will be ignored. Each 
tile will be converted to 4 colors (or 3 if the file supports transparency)
and sorted into ascending-value order. This will cause the darkest color in 
each tile to be treated as transparent. If the tile contained more colors, 
the exact collapse into 4 or 3 colors is determined by PIL.

If the file contains fewer than 256 complete 8x8 tiles, the output file will 
be padded with zeroes to make it 4k long; if it contains more, the first 256 
will be used and a warning will be issued.

Any lossless file format supported by PIL should work (PNG, GIF, BMP, etc.).
"""

import argparse
from pathlib import Path
from sys import stderr

from functools import reduce
from itertools import chain
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
                (px >> 1 & 0b01, px & 0b01) for px in 
                (image.getpixel((x, y)) for x in range(image.width))
            ),
            (0, 0)
        )
        for y in range(image.height)
    ]
    planes = zip(*rows)
    return bytes(chain(*planes))

def transcribe_chr(image: Image, outfile: Path):
    image = image.convert('RGBA' if image.has_transparency_data else 'RGB')
    try:
        with open(outfile, "wb") as file:
            padding = 0x1000
            for y in range(TILE_SIZE, image.height + 1, TILE_SIZE):
                for x in range(TILE_SIZE, image.width + 1, TILE_SIZE): 
                    tile = tile_data(image.crop((x - TILE_SIZE, y - TILE_SIZE, x, y)))
                    if padding < len(tile):
                        raise StopIteration
                    padding -= len(tile)
                    file.write(tile)
            file.write(bytes(0 for _ in range(padding)))
    except StopIteration:
        print(f"source file \"{outfile.name}\" contains more tiles than can fit in a 4k CHR archive.", file=stderr)
    

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

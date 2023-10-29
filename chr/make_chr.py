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

If the file contains more than 256 complete 8x8 tiles, a warning will be issued.

Any lossless file format supported by PIL should work (PNG, GIF, BMP, etc.).
"""

import math
import argparse
from pathlib import Path
from sys import stderr

from functools import reduce
from itertools import chain, batched
from PIL import Image

TILE_SIZE = 8

def value_of(color):
    return sum(a * b for a,b in zip(color, [.299, .587, .114]))

def order_indices(palette):
    return [idx for (idx, _) in sorted(palette, key=lambda clr: clr[1])]

def sort_palette(image: Image) -> Image:
    # group color channels into three-channel color lists
    colors = batched(image.getpalette(), n=3)
    palette = list(enumerate(value_of(color) for color in colors))
    # sort non-transparent colors by value and save only index order
    sequence = \
        [0] + order_indices(palette[1:]) \
        if image.has_transparency_data else \
        order_indices(palette)
    return image.remap_palette(sequence)

def tile_data(image: Image) -> bytes:
    """
    Convert an 8x8 image to a tile bytestream.
    The bytes will be ordered into two planes, where the first plane 
    holds the low-order bit of each pixel and the second plane holds
    the high-order bit.
    """
    if image.height != 8 or image.width != 8:
        raise RuntimeError("Tile must be 8x8 pixels.")
    # reduces tile to four colors
    image = image.quantize(colors=4)
    image = sort_palette(image)
    rows = [
        reduce(
            # consolidate parallel stream of bits into parallel stream of bytes
            lambda a, b: (a[0] << 1 | b[0], a[1] << 1 | b[1]),
            (
                (px >> 1 & 0b01, px & 0b01) for px in 
                (image.getpixel((x, y)) for x in range(image.width))
            ),
            (0, 0)
        )
        for y in range(image.height)
    ]
    #convert sequence of pairs to pair of sequences
    planes = zip(*rows)
    # concatenate sequences (low plane then high plane) and generate bytestream
    return bytes(chain(*planes))

def transcribe_chr(image: Image, outfile: Path, width=1, limit=0, columns=False):
    """
    Takes a complete image and outputs it as a tile stream into a file at the specified location.

    By default, issues a warning if the file size exceeds the typical 4k bank size but writes all tiles.

    Optional arguments (note that width and limit use different units):
        width: the stream will be extended to the next multiple of this many TILES.
        limit: the stream will be cut off before it goes over this many BYTES
        columns: set to True to scan the source file in column-major order.
    """
    image = image.convert('RGBA' if image.has_transparency_data else 'RGB')
    try:
        if image.height % TILE_SIZE != 0 or image.width % TILE_SIZE != 0:
            print(f"WARNING: image dimensions ({image.width}, {image.height}) has margins over tile size {TILE_SIZE} that will be ignored.", file=stderr)
        with open(outfile, "wb") as file:
            available = limit if limit != 0 else 0x1000   # default bank size
            count = 0
            # iterate over tiles in image
            for major in range(TILE_SIZE, image.height + 1, TILE_SIZE):
                for minor in range(TILE_SIZE, image.width + 1, TILE_SIZE): 
                    x, y = (major, minor) if columns else (minor, major)    # supports tall sprites
                    tile = tile_data(image.crop((x - TILE_SIZE, y - TILE_SIZE, x, y)))
                    if available < len(tile):
                        # hard limit on file size?
                        if limit != 0:
                            raise StopIteration
                        else:
                            print(f"WARNING: source file \"{outfile.name}\" contains more tiles than can fit in a 4k CHR archive.", file=stderr)
                            available = math.inf    # disable further warnings
                    available -= len(tile)
                    count += 1;
                    file.write(tile)
            # pad file to speicifed tile width
            count %= width
            if count > 0:
                file.write(bytes(0 for _ in range((width - count) * TILE_SIZE + TILE_SIZE)))
    except StopIteration:
        # file larger than specified limit
        print(f"source file \"{outfile.name}\" contains more tiles than can fit in the specified archive size.", file=stderr)

if __name__ == "__main__":
    #TODO: support user input for file padding, size limit, and scan order

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

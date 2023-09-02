"""Convert an image to NES CHR tile data.

The input image must be 128 pixels wide and use exactly 4 colors when converted
to grayscale.  Darker colors in the source image will be given lower palette
indexes in the tile data. (TODO: make this customizable)

Any lossless file format supported by PIL should work (PNG, GIF, BMP, etc.).
"""

import argparse
from numbers import Number
from pathlib import Path
from typing import Any, IO, Mapping

from PIL import Image

TILE_SIZE = 8


def write_tile(
    image: Image,
    image_x: Number,
    image_y: Number,
    colormap: Mapping[Number, Number],
    outfile: IO[Any],
):
    """Write an 8x8 tile of image data to the output file.

    Args:
        image: grayscale image data to save in CHR format color
        image_x, image_y: starting coordinate (upper-left) in the source image
            for tile data
        colormap: a map from grayscale color to palette index
        outfile: the file to write

    TODO: convert the image to indexes before we get here
    """

    # Each 8x8 tile of CHR data is split into two 1bpp "planes". Plane 0 holds
    # the lower bit of the color index, and plane 1 holds the upper bit.
    plane0 = []
    plane1 = []

    for tile_y in range(TILE_SIZE):
        plane0_byte = 0
        plane1_byte = 0

        # Pixel data for each row is stored in a single byte. The left-most
        # (first) pixel is in the highest bit. So we go pixel by pixel in the
        # source image, and shift in either the 0th or 1st bit of the color
        # value
        for bit in range(TILE_SIZE):
            color = colormap[image.getpixel((image_x + bit, image_y + tile_y))]

            # Shift the low bit into plane 0
            plane0_byte <<= 1
            plane0_byte |= color & 0b01

            # Shift the high bit into plane 1
            plane1_byte <<= 1
            plane1_byte |= color & 0b10

        plane0.append(plane0_byte)

        # Shift plane 1 byte down to the 1s place and store it
        plane1_byte >>= 1
        plane1.append(plane1_byte)

    outfile.write(bytes(plane0))
    outfile.write(bytes(plane1))


def make_chr(image: Image, outfile: Path):
    """Generate a CHR file from the given grayscale image"""

    # TODO: maybe use numpy but it's probably not nearly enough data to matter

    # Figure out which 4 shades are used by the image and sort them by
    # brightness
    colors = sorted(
        set(
            image.getpixel((x, y))
            for y in range(image.height)
            for x in range(image.width)
        )
    )
    assert len(colors) == 4

    # Make it easy to get color index for given source image color
    colormap = {color: i for i, color in enumerate(colors)}

    # Create the CHR file
    with open(outfile, "wb") as file:
        for image_y in range(0, image.height, TILE_SIZE):
            for image_x in range(0, image.width, TILE_SIZE):
                write_tile(image, image_x, image_y, colormap, file)


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
    image = image.convert(mode="L")

    make_chr(image, args.dest)

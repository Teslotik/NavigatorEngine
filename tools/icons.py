# Pack images (icons) into atlas

import os, sys
import subprocess

import argparse
from argparse import ArgumentParser

from PIL import Image, ImageOps

from typing import List, Tuple

# example: python ./icons.py /home/sergei/Projects/Multilussion/stuff/icons/ --invert
parser = ArgumentParser(
    description = 'Convert svg and png to atlas'
)
parser.add_argument('dir', nargs = '?', default = '.', help = 'directory with icons (.svg, .png)')
parser.add_argument('--inkscape', nargs = '?', default = 'flatpak run org.inkscape.Inkscape', help = 'inkscape path')
parser.add_argument('--tmp', nargs = '?', default = 'tmp', help = 'temp folder')
parser.add_argument('--width', nargs = '?', default = 32, type = int, help = 'tile width')
parser.add_argument('--height', nargs = '?', default = 32, type = int, help = 'tile height')
parser.add_argument('--dpi', nargs = '?', default = 92, type = int, help = 'svg dpi')
parser.add_argument('--export', nargs = '?', default = 'atlas.png', help = 'export file name')
parser.add_argument('--invert', action='store_true', help = 'invert color')

args = parser.parse_args()
# print(args)

tmp = os.path.abspath(args.tmp)
if not os.path.isdir(tmp):
    os.mkdir(tmp)
    print('[Info] Temp folder created:', tmp)
else:
    print('[Warning] Temp folder already exist:', tmp)
print()

items = []

print('[Info] Converting svg to png')
for entry in os.listdir(args.dir):
    pathname = str(os.path.abspath(os.path.join(args.dir, entry)))
    if entry.endswith('.png'):
        items.append(pathname)
        continue
    elif entry.endswith('.svg'):
        pass
    else:
        continue
    svg = pathname
    png = os.path.join(tmp, entry[:-len('.svg')] + '.png')
    print(svg, '->', png)
    # http://tavmjong.free.fr/INKSCAPE/MANUAL/html/CommandLine-Export.html
    # https://wiki.inkscape.org/wiki/Using_the_Command_Line
    subprocess.run(
        f'{args.inkscape} --export-filename="{png}" --export-dpi={args.dpi} --export-width={args.width} --export-height={args.height} "{svg}"',
        shell = True
    )
    items.append(png)
print()

print('[Info] Creating atlas')
images = [(os.path.basename(i), Image.open(i)) for i in items]
atlas = Image.new("RGBA", (args.width * len(images), args.height))
for (i, item) in enumerate(images):
    name, image = item
    print(name[:-len('.png')].capitalize() + ' = ' + str(i))
    if args.invert:
        r, g, b, a = image.split()
        image = Image.merge('RGB', (r, g, b))
        r, g, b = ImageOps.invert(image).split()
        image = Image.merge('RGBA', (r, g, b, a))
    image = image.resize((args.width, args.height), Image.Resampling.BILINEAR)
    atlas.paste(image, (i * args.width, 0))
export = os.path.abspath(args.export)
atlas.save(export)
print("[Info] Atlas saved to:", export)
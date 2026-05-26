import struct
from PIL import Image

img = Image.open('graphics/elm_speech/elm.png')
assert img.mode == 'P', img.mode
assert img.size == (64, 64), img.size
pal = img.getpalette()
entries = [tuple(pal[i:i+3]) for i in range(0, len(pal), 3)]
assert len(entries) == 17, len(entries)
assert img.info.get('transparency') == 0, img.info

# Build a 16-color GBA OBJ palette with the tan color at slot 14 and white at slot 15.
base_palette = [entries[0]] + entries[1:14] + [entries[15], entries[16]]
assert len(base_palette) == 16

print('Palette slots:')
for i, c in enumerate(base_palette):
    print(i, c)

remap = [0] + list(range(1, 14)) + [14, 14, 15]
# original idx 14 -> 14 (tan), idx 15 -> 14 (tan), idx 16 -> 15 (white)
# This collapses the extra pale grey/blue into the tan slot so slot 14 is the intended tan color.

with open('graphics/elm_speech/elm.gbapal', 'wb') as f:
    def rgb15(rgb):
        return (rgb[0] >> 3) | ((rgb[1] >> 3) << 5) | ((rgb[2] >> 3) << 10)
    f.write(b''.join(struct.pack('<H', rgb15(rgb)) for rgb in base_palette))

# Generate 4bpp tile data in standard 8x8 tile order.
data = bytearray()
for ty in range(8):
    for tx in range(8):
        for row in range(8):
            byte = 0
            for col in range(8):
                idx = img.getpixel((tx * 8 + col, ty * 8 + row))
                idx = remap[idx]
                if col % 2 == 0:
                    byte = idx
                else:
                    byte |= idx << 4
                    data.append(byte)
print('wrote', len(data), 'bytes')
assert len(data) == 2048, len(data)
with open('graphics/elm_speech/elm.4bpp', 'wb') as f:
    f.write(data)

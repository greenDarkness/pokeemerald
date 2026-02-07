from PIL import Image
import sys

# Hardcoded palette from npc_1.pal (JASC-PAL format, first 16 colors)
npc1_palette = [
    (115, 197, 164), (255, 213, 180), (255, 197, 148), (222, 148, 115),
    (123, 65, 65), (255, 164, 180), (213, 106, 123), (139, 65, 82),
    (148, 197, 246), (90, 139, 189), (16, 49, 82), (180, 164, 98),
    (123, 115, 65), (57, 57, 24), (255, 255, 255), (0, 0, 0)
]

# Load indexed PNG
img = Image.open('graphics/object_events/pics/pokemon/meowth.png')
if img.mode != 'P':
    raise ValueError('Image must be indexed (mode P)')

# Get current palette
old_palette = img.getpalette()[:48]
old_colors = [(old_palette[i], old_palette[i+1], old_palette[i+2]) for i in range(0, 48, 3)]

# Build mapping from old palette to npc_1 palette (by closest color)
def dist(c1, c2):
    return sum((a-b)**2 for a, b in zip(c1, c2))

mapping = {}
for npc_idx, npc_col in enumerate(npc1_palette):
    best = min(range(len(old_colors)), key=lambda i: dist(npc_col, old_colors[i]))
    mapping[best] = npc_idx

# Remap all pixels
pixels = [mapping.get(p, 0) for p in img.getdata()]
img2 = Image.new('P', img.size)
img2.putdata(pixels)

# Set new palette
flat_npc1 = [v for rgb in npc1_palette for v in rgb]
img2.putpalette(flat_npc1 + [0]*(768-len(flat_npc1)))

# Save output
img2.save('graphics/object_events/pics/pokemon/meowth_npc1pal.png')
print('Wrote graphics/object_events/pics/pokemon/meowth_npc1pal.png with npc_1 palette')

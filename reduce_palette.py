from PIL import Image

path = 'graphics/object_events/pics/pokemon/meowth.png'
img = Image.open(path)

# Ensure indexed
if img.mode != 'P':
    img = img.convert('P', palette=Image.ADAPTIVE, colors=16)

# Get palette and used indices
palette = img.getpalette()
used = sorted(set(img.getdata()))

# Keep at most 16 colors, preserving index 0 if present
if len(used) > 16:
    # Fallback: re-quantize to 16 colors
    img = img.convert('P', palette=Image.ADAPTIVE, colors=16)
    palette = img.getpalette()
    used = sorted(set(img.getdata()))

# Build new palette with used colors first
new_palette = []
index_map = {}

# Ensure index 0 stays 0 if present
start_indices = [0] if 0 in used else []
for idx in start_indices:
    r, g, b = palette[idx*3:idx*3+3]
    index_map[idx] = len(new_palette)//3
    new_palette.extend([r, g, b])

for idx in used:
    if idx in index_map:
        continue
    r, g, b = palette[idx*3:idx*3+3]
    index_map[idx] = len(new_palette)//3
    new_palette.extend([r, g, b])
    if len(new_palette)//3 >= 16:
        break

# Pad palette to 16 colors
while len(new_palette) < 16*3:
    new_palette.extend([0, 0, 0])

# Remap pixels
data = list(img.getdata())
remapped = [index_map.get(px, 0) for px in data]

img2 = Image.new('P', img.size)
img2.putpalette(new_palette)
img2.putdata(remapped)
img2.save(path)

print(f"Reduced palette to {len(set(remapped))} colors (max 16). Saved {path}.")

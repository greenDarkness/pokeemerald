from PIL import Image

# Load the palette-matched PNG
img = Image.open('graphics/object_events/pics/pokemon/meowth_npc1pal.png')
if img.mode != 'P':
    raise ValueError('Image must be indexed (mode P)')

# Assume the background is the most common color (excluding index 0 if possible)
from collections import Counter
pixels = list(img.getdata())
counts = Counter(pixels)

# If index 0 is not the most common, use the most common as background
bg_index = counts.most_common(1)[0][0]
output_path = 'graphics/object_events/pics/pokemon/meowth_npc1pal_trans.png'
if bg_index != 0:
    # Remap all bg_index pixels to 0
    pixels = [0 if p == bg_index else p for p in pixels]
    img.putdata(pixels)
    img.save(output_path)
    print(f'Wrote {output_path} with background set to index 0')
else:
    # Always write the output, even if unchanged
    img.save(output_path)
    print(f'Background is already index 0, wrote {output_path} unchanged.')

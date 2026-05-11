from PIL import Image

img = Image.open('/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald/graphics/object_events/pics/people/boy_1.png')
print('boy_1.png size:', img.size)
print('=== Frame 0 (x=0-15) non-zero rows ===')
for y in range(32):
    row = [img.getpixel((x, y)) for x in range(16)]
    if any(v != 0 for v in row):
        print(f'  y={y}: {row}')

print('=== Frame 1 (x=16-31) non-zero rows ===')
for y in range(32):
    row = [img.getpixel((x, y)) for x in range(16, 32)]
    if any(v != 0 for v in row):
        print(f'  y={y}: {row}')

from PIL import Image
img = Image.open('/mnt/c/Users/drfor/Documents/Porymap/Decomps/pokeemerald/graphics/object_events/pics/people/gym_leaders/falkner.png')
print('size:', img.size, 'mode:', img.mode)
# Print first frame pixels (x=0-15, y=0-31)
print('=== Frame 0 pixel values ===')
for y in range(0, 32):
    row = [img.getpixel((x, y)) for x in range(16)]
    nz = [v for v in row if v != 0]
    if nz:
        print(f'y={y:02d}: {row}')
    
# Also check the palette
pal = img.getpalette()
print('=== First 32 palette entries (R,G,B) ===')
for i in range(16):
    r, g, b = pal[i*3], pal[i*3+1], pal[i*3+2]
    print(f'  [{i}]: ({r},{g},{b})')

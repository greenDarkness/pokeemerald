from PIL import Image

img = Image.open('graphics/object_events/pics/people/special/silver.png')
print('size:', img.size, 'mode:', img.mode)

# Show which columns/rows have non-zero pixels
print('=== Non-zero pixel columns by frame (16px each) ===')
for frame in range(9):
    x0 = frame * 16
    has_data = []
    for y in range(32):
        row = [img.getpixel((x0+x, y)) for x in range(16)]
        if any(v != 0 for v in row):
            has_data.append(y)
    if has_data:
        print(f'Frame {frame} (x={x0}-{x0+15}): rows with data: {has_data[:5]}...')
    else:
        print(f'Frame {frame} (x={x0}-{x0+15}): ALL BLANK')

# Check first frame pixel data
print('\n=== Frame 0 sample rows with non-zero values ===')
for y in range(32):
    row = [img.getpixel((x,y)) for x in range(16)]
    nz = [v for v in row if v != 0]
    if nz:
        print(f'y={y:02d}: {row}')

from PIL import Image

# Input/output paths
input_png = "graphics/object_events/pics/pokemon/meowth_npc1pal_trans.png"
output_4bpp = "graphics/object_events/pics/pokemon/meowth.4bpp"

# Sprite/frame settings for 9 frames in a row (144x32)
frame_width = 16
frame_height = 32
num_frames = 9
tiles_per_row = frame_width // 8
tiles_per_col = frame_height // 8


img = Image.open(input_png)
assert img.mode == "P", "Image must be indexed color (mode 'P')"
assert img.width == frame_width * num_frames
assert img.height == frame_height

# Check for palette indices above 15
pixels = list(img.getdata())
max_index = max(pixels)
if max_index > 15:
    print(f"WARNING: Image contains palette indices above 15 (max found: {max_index})! This will cause color issues in 4bpp output.")
    # Optionally, clamp indices to 0-15 (uncomment to enable):
    # pixels = [min(p, 15) for p in pixels]
    # img.putdata(pixels)

with open(output_4bpp, "wb") as out:
    for frame in range(num_frames):
        x0 = frame * frame_width
        y0 = 0
        for ty in range(tiles_per_col):
            for tx in range(tiles_per_row):
                x = x0 + tx * 8
                y = y0 + ty * 8
                tile = img.crop((x, y, x + 8, y + 8))
                tile_bytes = bytearray()
                pixels = list(tile.getdata())
                for i in range(0, 64, 2):
                    tile_bytes.append((pixels[i + 1] << 4) | pixels[i])
                out.write(tile_bytes)

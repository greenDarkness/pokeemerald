from PIL import Image

# Load the image
img = Image.open('graphics/object_events/pics/pokemon/meowth.png')

# Convert to indexed color with 16 colors max
img_indexed = img.convert('P', palette=Image.ADAPTIVE, colors=16)

# Save as PNG - this should create 4-bit indexed
img_indexed.save('graphics/object_events/pics/pokemon/meowth.png')

print('Converted meowth.png to 4-bit indexed color')

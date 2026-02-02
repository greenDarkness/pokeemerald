from PIL import Image
img = Image.open('graphics/object_events/pics/pokemon/meowth.png')
print(img.mode, img.size)
counts = img.getcolors()
print('unique colors:', len(counts))
counts = sorted(counts, reverse=True)
print(counts[:10])
nonzero = sum(1 for c,i in counts if i != 0)
print('nonzero indices:', nonzero)

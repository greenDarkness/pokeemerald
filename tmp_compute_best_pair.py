from PIL import Image
import math
img = Image.open('graphics/elm_speech/elm.png')
pal = img.getpalette()
entries = [tuple(pal[i:i+3]) for i in range(0, len(pal), 3)]
# candidates for collapse are PNG indices 2..13, because 14->13, 15->14, 16->15 are fixed
candidates = list(range(2, 14))
best = None
for i in range(len(candidates)):
    for j in range(i+1, len(candidates)):
        a = entries[candidates[i]]
        b = entries[candidates[j]]
        d = sum((a[k]-b[k])**2 for k in range(3))
        if best is None or d < best[0]:
            best = (d, candidates[i], candidates[j], a, b)
print('best pair', best)
for k in candidates:
    print(k, entries[k])

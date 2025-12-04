import struct

# Read maps - github_map.bin is the shortcut version (qol branch)
# local_map.bin is the current VS Code version
with open('github_map.bin', 'rb') as f:
    github_data = f.read()
with open('local_map.bin', 'rb') as f:
    local_data = f.read()

width = 45

print("VERDANTURF SHORTCUT AREA (x=32-42, y=0-6)")
print("=" * 60)
print()

print("GITHUB qol branch (shortcut version):")
for y in range(7):
    tiles = []
    for x in range(32, 43):
        offset = y * width * 2 + x * 2
        val = struct.unpack('<H', github_data[offset:offset+2])[0] & 0x3FF
        tiles.append(f"{val:03X}")
    print(f"y={y}: " + " ".join(tiles))

print()
print("LOCAL VS Code (your current version):")
for y in range(7):
    tiles = []
    for x in range(32, 43):
        offset = y * width * 2 + x * 2
        val = struct.unpack('<H', local_data[offset:offset+2])[0] & 0x3FF
        tiles.append(f"{val:03X}")
    print(f"y={y}: " + " ".join(tiles))

print()
print("DIFFERENCES at Verdanturf shortcut (x=32-42, y=0-6):")
print("To overlay GitHub onto Local, use setmetatile with GitHub values")
for y in range(7):
    for x in range(32, 43):
        offset = y * width * 2 + x * 2
        gh = struct.unpack('<H', github_data[offset:offset+2])[0] & 0x3FF
        loc = struct.unpack('<H', local_data[offset:offset+2])[0] & 0x3FF
        if gh != loc:
            print(f"  setmetatile {x}, {y}, 0x{loc:03X}, TRUE  @ was 0x{gh:03X}")

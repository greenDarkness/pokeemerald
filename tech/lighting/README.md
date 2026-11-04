# Night lighting tools

Python helpers for adding or fixing night-time building lighting in Hoenn tilesets. Run them from this
folder (`cd tech/lighting`). They need Pillow. Scratch renders go to `out/` (git-ignored).

To preview a whole region at night, use `python tech/night_map.py hoenn` (or `Night Map.bat`).

## How lighting works

- **Lit palette colors.** A color glows at night when its index is listed in the palette's `NN.pla`
  file. At night a lit color is blended halfway toward `DEFAULT_LIGHT_COLOR`.
- **Night palettes.** Dark lit colors look muddy, so a tileset can list palettes in `.swapPalettes`
  (`src/data/tilesets/headers.h`, absolute palette numbers). At night, palette `P` is replaced by
  palette file `(P + 9) % 16`. Day colors in `P` never change.
- **Shared palettes.** The primary General palettes 0-5 are shared by every Hoenn tileset, so lighting
  one of their indices lights it on every map. The tools put lit pixels in secondary palettes 6-12 instead.
- **Overlays.** If a tile's palette has no free slot, put a tile that holds only the lit pixels on the
  top layer, move the original tile to the bottom layer, and set the metatile layer type to COVERED.
- **Reserved tiles.** Door animations write into the last 16 VRAM tiles (secondary local tiles 496-511),
  and tileset animations overwrite their own destination tiles. Never store edited tiles there.
  `Lighter.reserved_tiles()` handles this.
- **Light sprites.** These are `OBJ_EVENT_GFX_LIGHT_SPRITE` objects in a map's `map.json`. The type is set
  in `trainer_sight_or_berry_tree_id`: `0` lantern, `1` Poké Center sign, `2` Mart sign,
  `3`/`4` PC/Mart Pokéball emblems (spawned automatically by `1`/`2` one tile up and left), `5` lantern
  pair (lanterns at x and x+3), and `6` Lilycove Department Store emblem (centered on its tile). A map can
  have at most 64 object templates. The emblem sprites are 32x32: the lit ball plus a dim ring that adds
  ambient glow under the 12/12 alpha blend, like the dim plaque fill on the sign sprites. Emblems use OAM
  priority 1 because the roof tiles under them are on the top BG layer.
- **Sprite palettes.** Light sprites use blend-immune palette tags (bit 15 set, see `IS_BLEND_IMMUNE_TAG`).
  `neon_light.pal` is shared by the PC sign and all emblems; the Mart sign has its own brighter
  `mart_light.pal`, tuned so one 12/12 blend looks like two (overlapping sprites never blend with each
  other on the GBA). `night_map.py` draws a light listed by two maps at the same spot only once.
- **Neighbor lights.** Besides its own objects, a map spawns the light objects of its connected maps when they
  come into view (`TrySpawnConnectedLightSprites`), and lights keep their place when the player crosses a
  connection. So a sign near a border is already lit before you walk into its map; there's no need to copy
  light objects into neighboring maps.
- **Door animations.** Door frames come from `graphics/door_anims/*.png` with fixed BG palettes, so door
  metatile lighting doesn't apply while a door opens or closes. At night `src/field_door.c` draws a 16x32
  sprite over the animating door holding only the lit pixels of each frame: the closed door's glow, glass
  that slides, and the warm doorway revealed behind it. The frames and palettes are generated:
  `python doors.py` lists lit doors, `python doors.py --build` rewrites `graphics/door_anims/night/` and
  `src/data/door_night_overlays.h`, and `doors.preview()` renders day/night frames to `out/`. Rerun
  `--build` after changing the lighting of any door metatile.

## kit.py: inspect and render

```python
from kit import *
pair('RustboroCity', (0, 0, 20, 15), 'rust', scale=4)   # day | night render -> out/rust.png
render(map, box, night, scale)                          # one render
P, S = TS('gTileset_General'), TS('gTileset_Rustboro')
print(dump(P, S, 0x2b5))                                # metatile entries and 16x16 color-index grid
where('RustboroCity', [0x2b5])                          # map coordinates that use these metatiles
warps('RustboroCity')                                   # door / warp positions
add_light('SootopolisCity', 45, 31, 1)                  # append a light object to map.json
uses(['gTileset_Rustboro'], 640)                        # metatiles that draw a tile
secondaries_of('gTileset_General')                      # secondary tilesets on a primary
```

`TS` reads and writes a tileset: `entries/set_entry`, `layer/set_layer`, `px/set_px`, `pal/write_pal`,
`pla/write_pla`, `free_tiles`, `save_tiles`, and `save_mt`.

## lighter.py: make pixels glow

```python
from lighter import *
L = Lighter('gTileset_Rustboro')
L.glow(0x2b5, [(2, 0, 14, 11)], idx_filter={7})          # rects are (x0, y0, x1, y1) in 0..16
L.glow(0x2b6, [(3, 4)], night={7: (248, 216, 144)})      # single pixels, custom night color
L.glow_overlay(0x2ac, [(0, 11, 16, 16)], {7})             # when the palette is full
L.glow_overlay(0x3a1, [(0, 0, 16, 11)], {10, 4},          # night can be a function of (x, y, index):
               night=lambda x, y, c: (80, 160, 248) if y < 5 else None)   # blue top band, warm below
L.flatten(0x340, 0)                                       # merge a quadrant's top tile down so
                                                          # glow_overlay can use the top layer
L.solid(0x20a, [2, 3], day8=(48, 48, 56))                 # fill quadrants with one lit color
L.save()
```

- Door glass convention (PC/Mart/gym doors): blue night color `(80, 160, 248)` on the upper part of the
  glass and warm `(248, 216, 144)` below, so some blue shows through like the Verdanturf houses.
- `L.orphans(P)` lists lit indices no tile draws anymore (left over from undone edits); `L.reclaim(P)`
  frees them. Check `sDoorAnimPalettes_*` in `src/field_door.c` first, since door animations draw with
  tileset palettes too.

- `glow()` never edits shared tiles. It copies the tile to a free slot, allocates a lit color with the
  same day color and a warm night color (`default_night`), and adds the tileset to `.swapPalettes`
  when needed.
- If `glow()` raises `no palette room`, use `glow_overlay()`. That fails if the top layer is already
  used by a tile with transparent pixels. In that case, `flatten()` the quadrant first (both layers
  must use the same palette) or restrict the rect to the other quadrants.
- Undo helpers: run `snapshot_head()` once to extract `data/tilesets` at `HEAD` into `.head/`, then
  use `restore_meta(L.ts, gid)` and `restore_tile(L.ts, local_tile)`.

## Checklist

1. Render with `pair(...)` before and after the change. The day half must look identical.
2. Build: `wsl -e bash -lc 'cd /mnt/d/projects/pokeemerald && make -j8'`.
3. Run `python tech/night_map.py hoenn` and read the unlit-door report.

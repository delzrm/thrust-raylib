# ThrustC

C11 port of the Thrust arcade game, using raylib 5.5. All source files are in `src/`.

## Build

```
cmake --build build --config Debug
```

Output: `Debug/thrust.exe`

## Source files

| File | Purpose |
|------|---------|
| `src/thrust.c` | Main file: game state, physics, input, rendering orchestration |
| `src/Draw.c/h` | Mesh2D renderer + all static game object meshes + landscape mesh cache |
| `src/Levels.c/h` | Level definitions (`gLevels[6]`) and level data types |
| `src/Collision.c/h` | Pure math: `CheckIntersect`, `CheckIntersectCircle` |
| `src/VectorFont.c/h` | Vector font rendering: `DrawVectorStr`, `VectorStrWidth` |
| `src/HUD.c/h` | HUD and message rendering: `DrawHUD`, `DrawMessage` |
| `src/Types.h` | Shared `V2` typedef |

## Key conventions

- **C11**, MSVC on Windows
- **raylib coordinate system**: Y increases downward, CCW winding for `DrawTriangle`
- `SXf(wx)` / `SYf(wy)` — world → screen transform (defined in thrust.c)
- `ParseHex("#rrggbb")` — hex colour parser (local copies in thrust.c and HUD.c)
- `HUD_H 51` — pixel height of the HUD strip at the top of the screen
- Landscape is ear-clipped once in `CreateLandscapeMesh()` (Draw.c), called at end of `LoadLevel()`
- All game state lives in the global `gGame` struct

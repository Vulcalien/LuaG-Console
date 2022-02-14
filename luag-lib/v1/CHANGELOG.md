# LuaG Library changelog

## version 1.4
1. `spr` now has a 10th parameter: `color_mod` of type `int`.\
    It is optional, and its default value is `0xffffff` (white).
2. `settransparent` can now be called without an argument.
    This will remove the transparent color (aka color key).
3. mouse functions: `mouse` (or `mouse_down`), `mouse_pressed`,
   `mouse_released`, `mouse_pos`, `scroll`.

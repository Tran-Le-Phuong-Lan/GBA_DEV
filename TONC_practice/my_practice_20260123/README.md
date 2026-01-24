# ATTENTION

- **when export tile design with indexed colored from GIMP to `.png`**, in order to have the correct indexed colored converted by `grit`, the following options must be set 

    - The first index color (i.e index 0) must be black = the default transparent background color in GBA. Otherwise, any other color (not black) at index 0, when being loaded into GBA, it will appear to be black on the screen.

    - Export (as default) :Pixel format = Automatic, No selection is checked. Then the data converted by `grit` will follow exact color index as shown in GIMP.

- **In the current program**

    - Disable the `grit` function in `Makefile`, in order to add the Cascasone tile map (i.e 1 cascasonne tile = 1 map : made of 9 TILEs from the tileset created earlier from `tiles-gbc.png`).

    - Description of the manual added map data `tiles_gbcMap` in `./build/tiles-gbc.s` and its corresponding definition in `tiles-gbc.h`:

        - **1 map data (9 x u32)** = **1 cacasonne graphic tile**

        - **each element/data (u32)** in the `tiles_gbcMap` = **the index of which TILE** is used from the imported `tiles_gbcTiles` in `./build/tiles-gbc.s`

    - obj sprite named `cursor`- 32x32p made of TILEs (8x8p @ 4bpp)

    - address of the obj graphic tiles @ 06010000, because the TID in ATTR 2 is set to 0, and 0 = `&tile_mem[4][0]` 

    - **obj palette shared with the Cas tiles** @ `&pal_obj_bank[2][0]`

    - obj oam `cursor` @ 0700:0000

        - each attribute 16-bit long

        - attribute 0, attribute 1, attribute 2, affine attribute
    
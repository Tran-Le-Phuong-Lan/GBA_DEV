# NOTE

1. How to set pixel grid in GIMP: 

- https://www.gimp-forum.net/Thread-1-pixel-1-grid-cell
 
2. Mappy tutorial

- https://www.youtube.com/watch?v=JckIPqdLpHg&list=PLOG3-y9fHFjlXvHZ9UkkyQ25I7JTAyu7Z&index=16

- Problem with using Mappy with png: see https://www.tilemap.co.uk/pngfiles.html
  
- some explanation of data exported in tileset and tilemap (but GB not GBA): https://flozz.github.io/img2gb/howto.html

- Another map editor (similar to Mappy): https://www.mapeditor.org/ - with plug-ins: https://github.com/djedditt/tiled-to-gba-export to export the map data to GBA .c and .h file.

- **Mappy tutorial for GBA** here: file:///home/tlplan/Downloads/GBAMappy/GBAMappy.html - **examples downloaded** from https://www.tilemap.co.uk/mappy.php -> section **Mappy Playback Libraries** -> **GBA**

3. File exported from Mappy for GBA:

- <file_name>_blockgfx : tile graphics

- <file_name>_cmap : color palette

- <file_name>_map0 : tile map

# Successful attempt

4. This folder `my_second_brin_demo` attempts to **learn how to load in a customed regular tile background with customed tileset**, to **understand how the data of SE (screen entry) of the tilemap data related to how the graphical tile on the screen** is decided based on the which SE data. The steps below will help to achieve the objectives of this folder:

- Using **Mappy** to design the map - **32 tile x 32 tile** (= 1 whole Screen Block = 256 pixel x 256 pixel) **based on the tiles** (`./tiles-gbc.png` = 64 pixel x 16 pixel, 8-bit colormap) -> export the map into **`./graphics/First_mapscrn.bmp`** file.

- at the moment, no clear understanding of how the data exported for GBA from Mappy is made up: **the tilemap** data exported in `./tiles/First_map.TXT` **by Mappy** is in **INTEGER**, while **in the TONC tutorial**, the form is **HEX**, whether it is compatible with the Tonc example code? How to make it compatible? As introduced in the tutorial, `grit` can also generate tileset, color palette and tilemap data from a `.png` or a `.bmp`. It's better to use `grit` to transform the output Mappy file, then learn from it, when the understanding of regular tile map in GBA gets better, return to answer the mentioned questions.  

- Modify `Makefile` to 
  
  - **use the official Tonc-lib provided with DEVKITPRO** instead of simplified verion such as: `./include/toolbox.h` - `./source/toolbox.c`, `./include/input.h` files which are deleted from this folder (but these files were demonstrated in the other folder `../my_first_brin_demo`) 

  - convert **the `.bmp` instead of `.png`**

- Modify the source code `./source/brin_demo.c` to

  - use the `memcpy32` and `memcpy16` from Tonc-lib, to avoid the bugs of `memcpy` as reported in `../my_first_brin_demo`

  - adjust to REG_BG0CNT for smaller size image: 32x32 tile

  - no moving
  
- Then, `make` the  program to see the result.
  
- To understand the SE content and its relation to the graphical tile on the screen, do as follows: after the first `make`, comment out the following command lines in the `Makefile`

```
 %.s %.h: %.BMP %.grit
#---------------------------------------------------------------------------------
 	@echo "grit $(notdir $<)"
 	@grit $< -fts -o$*
``` 

- From now on, the manually changed/manipulated the content SE data of the  `./build/First_mapscrn.s` will be kept for the next `make`, without being overwritten when `make` again. Therefore, this experiment will help the understanding of SE better. **How to manipulate the SE data and see its graphic tile on screen?**

  - In TONC tutorial (chapter 9 regular background -section Regular background tile-maps): we could see the 16-bit discription (content) of a SE. Use an online Binary to HEX converter to have the corresponding HEX format for the desire binary 16-bit of SE.
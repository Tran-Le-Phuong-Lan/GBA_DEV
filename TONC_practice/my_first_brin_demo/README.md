
# NOTE

- This project is a copy and modification from the original TONC code and the newer version TONC code (libtonc-examples).

  - The `Makefile` (copied from libtonc-examples) is modified to not to use the `/opt/devkit/libtonc`, but use the simplified-version of libtonc in the `./include` and `./source`. It helps the learning process much more fundamental.

  - The all the source codes in folder `./include` and `./source` are copied from the original TONC code.

  - The `./graphics` contains exact `<image>.png` from original TONC/ libtonc-examples. The file `./graphics/<image>.grit` is copied from libtonc-examples.
 
- 1 SB = 32X32 TILE/ SE (1024 TILE) = 256x256 pixel

- If the graphical map is bigger than 256x256 pixel, more than 1 SB is used -> CAREFUL to index the SE correctly, because of the index of SE should start with index in the SB which contains it then index it.

- **inpsect the hardware GBA such VRAM, PALRAM, etc with mGBA**: load ROM game -> Tools -> Game states view -> view memory 

# Failure attempt

- Using the orignal source from the original TONC code, that means to move tileset data and tilemap data into VRAM, the code `./source/brin_demo.c` uses the following code (same as in the TONC tutorial text - section 9. Regular background - 9.4 Tilemaps demo)

```
memcpy(&tile_mem[0][0], brinTiles, brinTilesLen);
...
memcpy(&se_mem[30][0], brinMap, brinMapLen);
```
  - **Result**: the gba ROM when opened in mGBA is totally blank.

  - Debug process: load the failure gba ROM into mGBA -> open Tools -> game state views -> view memory : observe that **@0x0600:0000h** where it expects to receives the tileset data (`./build/brin.s`) is totally blank (i.e all full of 0x00000000); **@0600:F000h** (i.e 30th ScreenBlock) where the tilemap data is expected is also totally blank. That means the `memcpy` has problem. (see the section **Successful attempts** below to know **the solution**)

# Successfull attempts

- 1. in the source code `./source/brin_demo.c` make the following change

> memcpy(&tile_mem[0][0], brinTiles, 988);
>
> ...
>
> memcpy(&se_mem[30][0], brinMap, 3968);

  - **Result**: the gba ROM works in mGBA.

  - **Reason**: only based on the experiment observations: by checking `brinTiles` and `brinMap` in `./build/brin.s`, **one thing in commond is that they both end with blank data**, for example, `0x00000000` in case of `brinTiles` or the very last block of data of  `brinMap` as follow


>	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
>
>	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
>
>	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
>
>	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
>
>	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
>
>	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
>
>	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
>
>	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

Because of the blank data ending, it somehow makes the `memcpy` does not function normally. Therefore the solution is to change the length of the byte must be transferd by `memcpy`, so that the blank data ending is not seen by `memcpy` any more. 

- **ADVICE**: for real project, simply using the `libtonc` libraries (by using the `Makefile` as in libtonc-examples) which contains the new function `memcpy32` and `memcpy16` as demonstrated in the libtonc-examples/basic/brin_demo. To avoid the buggy thing of `memcpy` mentioned above.  
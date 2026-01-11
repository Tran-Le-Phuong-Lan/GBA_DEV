# NOTE 

- 0. **In TONC tutorial** - section 7->9 (Regular Sprite & Regular Tile background), **8x8 tile = 1 tile is made of 8 pixel x 8 pixel x pixel bit depth**; the pixel bit depth is either 4 bit per pixel (bpp) or 8bpp. **Any notation of \<number\>x\<number\> (e.g 8x8 tile, 64x64 tile, 16x16 tile, 16x64 tile, etc) tile = \<number\>-pixel-x-\<number\>-pixel tile (e.g 8-pixel-x-8-pixel tile, 64-pixel-x-16-pixel tile) (i.e unit pixel)**; unless \<number\>x\<number\>t tile = \<number\>-tile-x-\<number\>-tile tile (i.e unit tile).

- 1. `tile_mem` is defined in `./include/memmap.h`

- 2. `CHARBLOCK`, `TILE`, `TILE4`, `TILE8`: defined in `./include/types.h`

- 3. `#define ALIGN4      __attribute__((aligned(4)))` : 4 = 4 bytes.
`__attribute__((aligned(n)))`: n = 2^n.
Very good explaination of data alignment: https://stackoverflow.com/questions/11770451/what-is-the-meaning-of-attribute-packed-aligned4
**Forcing alignment** in `struct` will always ensure safe and correct (sometimes even faster) data moved into the VRAM (because structure copy is with memcpy, memcpy always work with forcing alignment).

- 4. `obj_set_attr` : `./include/toolbox.h`

- 5. `ATTR0_SQUARE`, `ATTR0_AFF`, ATTR2_PALBANK..: `./include/memdef.h`

- 6. `oam_init`: `./source/toolbox.c`

```
typedef TILE     CHARBLOCK[512];
...
typedef struct { u32 data[8];  } TILE, TILE4;
...
typedef struct OBJ_ATTR
{
	u16 attr0;
	u16 attr1;
	u16 attr2;
	s16 fill;
} ALIGN4 OBJ_ATTR;

typedef struct OBJ_AFFINE
{
	u16 fill0[3];	s16 pa;
	u16 fill1[3];	s16 pb;
	u16 fill2[3];	s16 pc;
	u16 fill3[3];	s16 pd;
} ALIGN4 OBJ_AFFINE;
```

```
tile_mem = (CHARBLOCK*)0x06000000 // no 8bit write !!
```

- 7. To understand better how the .png is converted to sprite tile data to load into the VRAM in tile mode: [`grit` document](https://www.coranac.com/man/grit/html/grit.htm):

  - by looking at the additional options, `-ar64`, of `grit` in `./graphics/metr.grit`, the original image `./graphics/metr.png` is actually being extracted to get only the the left half before being converted into the tile-sprite data file. In details, the original image is 128 pixel width x 64 pixel height, with the option `-ar64`, the `grit` only takes the (0th pixel -> 63th pixel) width x 64 pixel height to convert. If the additional option is `-al64`, the `grit` only takes the (64th pixel -> 127th pixel) width x 64 pixel height to convert. Try out: by simply change `-ar64` to `-al64` in the `./graphics/metr.grit`, and `make` the whole project again.

- 8. **Explanation of sprite tile mode mapping**

  - 2D mapping mode should be combined being reading the [TONC tutorial - section 8 Regular sprites - 8.2.1 The sprite mapping mode](https://gbadev.net/tonc/regobj.html) and the [PERN tutorial - Day 3](https://web.archive.org/web/20030329130625fw_/http://www.thepernproject.com/English/tutorial_3.html). 
  
  - 1D (a stack of TILE (TILE = the definition of 8 pixel x 8 pixel x 4bpp tile)), this is explained better with [PERN tutorial - Day 3](https://web.archive.org/web/20030329130625fw_/http://www.thepernproject.com/English/tutorial_3.html).

  - Important note: **In PERN tutorial**, **the author prefers 1D mapping mode over 2D** because of its very easy to load TILEs sprite data into VRAM without worrying about the right offset of data index as in 2D mode. They also mentioned that they have never used the 2D mapping mode, and with 1D mapping mode the data transfer could be even faster with DMA while 2D could not. On the other hand, **In TONC tutorial, the author also mentions similar benefit and the example is also 1D mapping**.
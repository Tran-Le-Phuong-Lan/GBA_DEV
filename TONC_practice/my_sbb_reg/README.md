# NOTE

## information extracted from libtonc to understand the source code

```
//--- tonc_types.h-----

typedef u16 SCR_ENTRY, SE;			//!< Type for screen entries

//! Regular bg points; range: :0010 - :001F
typedef struct POINT16 { s16 x, y; } ALIGN4 POINT16, BG_POINT;

typedef u16 COLOR;					//!< Type for colors

//! Palette bank type, for 16-color palette banks 
typedef COLOR PALBANK[16];

//--- tonc_memdef.h----

#define MEM_PAL		0x05000000	//!< Palette. Note: no 8bit write !!

#define SE_PALBANK_SHIFT		12
#define SE_PALBANK(n)		((n)<<SE_PALBANK_SHIFT)

//--- tonc_memmap.h---

//! Background palette matrix. 
/*! pal_bg_bank[y]		= bank y				( COLOR[ ] )<br>
	pal_bg_bank[y][x]	= color color y*16+x	( COLOR )
*/
#define pal_bg_bank		((PALBANK*)MEM_PAL)

//--- tonc_video.h---
INLINE COLOR RGB15(int red, int green, int blue);

//! Create a 15bit BGR color.
INLINE COLOR RGB15(int red, int green, int blue)
{	return red + (green<<5) + (blue<<10);					}

```

## GBATEK

- in GBATEK, the term "dots" = pixels.

## Successful attempts

- This foler `my_sbb_reg` is modifed in order to learn (**see the comment in the source code `./source/sbb_reg.c`**):

	- how the data of TILE (=4bpp) is arranged and plot on the screen

	- how different pallettes (in this case 4 palettes) are stored in the PAL MEM, how the color in each palette is stored, how the TILES (=4bpp) use a specific color from a specific palette.

	- how the SE 16-bit data is used to which TILE in the tileset is used, how to set the palette for that TILE, where that TILE will be plotted on the screen.

	- understand how multi-SBBs are arranged to create a big map size (>32x32tile).    
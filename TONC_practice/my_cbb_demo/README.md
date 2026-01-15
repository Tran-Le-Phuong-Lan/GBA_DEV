# NOTE

## information to understand the source code

```
// --- tonc_types.h ---
//! 4bpp tile type, for easy indexing and copying of 4-bit tiles
typedef struct { u32 data[8];  } TILE, TILE4;

//! 8bpp tile type, for easy indexing and 8-bit tiles
typedef struct { u32 data[16]; } TILE8;

typedef TILE		CHARBLOCK[512];
typedef TILE8		CHARBLOCK8[256];

typedef u16 SCR_ENTRY, SE;			//!< Type for screen entries
typedef SCR_ENTRY	SCREENBLOCK[1024];

// --- tonc_memmap.h ----
//!	Charblocks, 4bpp tiles.
/*!	tile_mem[y]		= charblock y				( TILE[ ] )<br>
	tile_mem[y][x]	= block y, tile x			( TILE )
*/
#define tile_mem		( (CHARBLOCK*)MEM_VRAM)

//!	Charblocks, 8bpp tiles.
/*!	tile_mem[y]		= charblock y				( TILE[ ] )<br>
	tile_mem[y][x]	= block y, tile x			( TILE )
*/
#define tile8_mem		((CHARBLOCK8*)MEM_VRAM)

//! Screenblocks as arrays
/*!	se_mem[y]		= screenblock y				( SCR_ENTRY[ ] )<br>
*	se_mem[y][x]	= screenblock y, entry x	( SCR_ENTRY )
*/
#define se_mem			((SCREENBLOCK*)MEM_VRAM)
```

## Folder objective

- Only add some comments into the datafile `./soruce/cbb_ids.c` and the source code `./cbb_demo.c`:

    - to understand how to use multi-CBBs to store a lot of graphic tiles, how the tiles are indexed.

    - Be aware that CBB 4 and 5 are only for sprite graphics, tile background graphics if accidentally stored here, they can not be accessed.
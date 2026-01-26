//
// obj_demo.c
// testing various sprite related things
//
// (20031003 - 20060924, Cearn)

#include <string.h>
#include <tonc.h>
#include "tiles-gbc.h"

// define the access to cacasone-tile map
typedef u32 CAS_TILE_MAP[9];
#define cas_tile_map_id	((CAS_TILE_MAP*)tiles_gbcMap)
#define elem_cas_tile_set	((TILE*)tiles_gbcTiles)


OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

#define CBB_0  0
// ? while some graphic error when SBB_0  = 0 
#define SBB_0  1
SCR_ENTRY *bg0_map= se_mem[SBB_0];

void init_reg_bg ()
{
		// initialize a background
	REG_BG0CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_32x32;
	REG_BG0HOFS= 0;
	REG_BG0VOFS= 0;

		const TILE tiles[1]=
	{
		// bg tile
		{{0x10000001, 0x01111110, 0x01111110, 0x01111110,
		  0x01111110, 0x01111110, 0x01111110, 0x10000001}},
	};

	// Place the bg tile 
	// into VRAM: cbb == 0
	tile_mem[CBB_0][0] = tiles[0];
	memcpy32(&tile_mem[CBB_0][1], tiles_gbcTiles, tiles_gbcTilesLen/sizeof(u32));

	// create and place a palette for bg
	//			P  C
	pal_bg_bank[0][1]= RGB15(31,  0,  0);
	pal_bg_bank[1][1]= RGB15(0,  31,  0);
	pal_bg_bank[2][1]= RGB15(0,  0,  31);
	pal_bg_bank[3][1]= RGB15(31,  31,  0);
	// memcpy16(&pal_bg_bank[4][0], tiles_gbcPal, tiles_gbcPalLen/sizeof(u16));
	memcpy16(&pal_bg_bank[4][0], tiles_gbcPal, tiles_gbcPalLen/sizeof(u16));

}

void init_reg_obj ()
{
		const unsigned int tiles[128]=
	{
		// cursor tile 32x32p
			// tile 1 , Graphic RIGHT -> Graphic LEFT
			0x01111111, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
			// tile 2
			0x11111111, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			//tile 3
			0x11111110, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000,
			// tile 4
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,

			// tile 5
			0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001,
			// tile 6
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 7
			0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000,
			// tile 8
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,

			// tile 9
			0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x01111111,
			// tile 10
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x11111111, 
			//tile 11
			0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x11111110,
			// tile 12
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,

			// tile 13
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 14
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 15
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
			// tile 16
			0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000

	};
	

	// creat a obj palette (already put into the OBJPALMEM)
	pal_obj_bank[0][1]= RGB15(0,  31,  0);
	pal_obj_bank[1][1]= RGB15(0,  0, 31);
	memcpy16(&pal_obj_bank[2][0], tiles_gbcPal, tiles_gbcPalLen/sizeof(u16));
	
	
	// Place the cursor 
	// into VRAM: LOW obj memory (cbb == 4)
	// tile_mem[4][0] = tiles[0];

	memcpy32(&tile_mem[4][0], tiles, 128);
	// memcpy(pal_obj_mem, LinkZeldaPal, LinkZeldaPalLen);
}

void draw_func()
{
	int x= 0, y= 0; // [pixel]

	u32 tid= 0, pb= 0;		// tile id, pal-bank

	OBJ_ATTR *cursor= &obj_buffer[0];
	obj_set_attr(cursor, 
		ATTR0_SQUARE,				// Square, regular sprite
		ATTR1_SIZE_32,					// 32x32p,
		// the object priority is zero
		ATTR2_PALBANK(pb) | tid);		// palbank 0, tile 0 = base tile of the sprite
										// tile 0 = &tile_mem[4][0]

	// set the starting position of the obj
	// x,y [pixel] = position of the top-left pixel of the obj
	obj_set_pos(cursor, x, y);

	// enable a timer
	REG_TM2D =  -0x065E;
	REG_TM2CNT= TM_ENABLE | TM_FREQ_1024; 
	// 1 period = 1024 default clock cycle
	// = 1024 * (1/16.78 Mhz) = 61 us 

	// tick every ~ 100ms
	REG_TM3CNT=  TM_ENABLE | TM_CASCADE; 
	
	u32 sec = -1;

	while(1)
	{
		VBlankIntrWait();
		key_poll();
		
		// // increment/decrement starting tile with R/L
		// tid += bit_tribool(key_hit(-1), KI_R, KI_L);

		
		SCR_ENTRY *pse= bg0_map;
		u16 obj_x_coord, obj_y_coord; // in unit [pixel]
		u32 se_curr;
		// get the cursor (object) position (using Tonc BF_GET())
		obj_x_coord = BFN_GET(cursor->attr1, ATTR1_X);
		obj_y_coord = BFN_GET(cursor->attr0, ATTR0_Y);
		// calculate the Se_index, map size 32x32t
		// >> 3: divided by 8 to convert to unit [tile] (tile = TILE)
		//							 32 = width of the map size in unit [tile]	
		se_curr = (obj_y_coord >> 3)*32 + (obj_x_coord >>3);
		// draw a green tile
		int cas_r, cas_col;
		int rand_pal;
		u16 map_width_unit_tile = 32;

		if(key_hit(KEY_A))
			{
				// update the palette according to Se_index
				// use the tonc `qran_range` to generate the
				// random palette.
				rand_pal = qran_range(0, 3);

				// load the cas graphics into the cas_tile_buffer = buffer of the cursor
				for (cas_r = 0; cas_r < 3; cas_r++)
				{
					for (cas_col=0; cas_col<3; cas_col++)
					{
						//				  CBB TILE_index	
						memcpy32(&tile_mem[4][cas_r*4 + cas_col], 
							&elem_cas_tile_set[cas_tile_map_id[rand_pal][cas_r*3 + cas_col]], 
							8 // 1 TILE = 8x u32
						);
					}
				}

				// example of using BFN_GET to set only the palette bank bit 
				// in the object attribute 2.
				// need palette 2 because this is the same palette of the tileset
				BFN_SET(cursor->attr2, 2, ATTR2_PALBANK);

			};
		// redo a wrong green tile	
		if(key_hit(KEY_B))
			{
				for (cas_r = 0; cas_r < 3; cas_r++)
				{
					for (cas_col=0; cas_col<3; cas_col++)
					{
						//																	 the TID drawn from `cas_tile_map_id` need `+1` because of a manual-created tile stored at base index 0
						pse[se_curr + cas_r*map_width_unit_tile + cas_col] = SE_PALBANK(4) | (cas_tile_map_id[rand_pal][cas_r*3 + cas_col]+1);
					}
				}
			};
				
		// start timer, wait for 10ms ~ 163 ticks
		if (REG_TM3D != sec)
		{
			sec = REG_TM3D;

			x += 24*key_tri_horz();
			// move up/down
			y += 24*key_tri_vert();
			obj_set_pos(cursor, x, y);
		}
	
		// In this example, there is only 1 sprite of size 64 pixel x 64 pixel (= 8 TILE x 8 TILE)
		oam_copy(oam_mem, obj_buffer, 1);	// only need to update one
	}
}

int main()
{
	// Init interrupts and VBlank irq.
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	// #define DCNT_OBJ			0x1000	//!< Enable objects
	// DCNT_OBJ = set 12th-bit to 1 =
	// in GBATEK- LCD I/O Display Control, 
	// it is defined as Screen Display OBJ  (0=Off, 1=On).
	// #define DCNT_OBJ_1D			0x0040	//!< OBJ-VRAM as array
	// DCNT_OBJ_1D = set 6th-bit to 1 =
	// using 1D sprite map mode to load the tile data.
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 |DCNT_OBJ | DCNT_OBJ_1D;

	init_reg_obj();
	init_reg_bg();

	// init the OAM, and make sure that 
	// the all the sprites (all 128 sprites) are hidden at the beginning.
	// The author inits 128 sprites, 
	// even though there is only one sprite (= 1 data of a sprite image stored in VRAM).
	// the function is in `./toolbox.c` 
	oam_init(obj_buffer, 128);
	
	// test background tiles
	SCR_ENTRY *pse_tiles= bg0_map;
	pse_tiles[0*32 + 0] = SE_PALBANK(4) | 0x0000; // SBB 0
	pse_tiles[0*32 + 1] = SE_PALBANK(4) | 0x0001; // SBB 0
	pse_tiles[0*32 + 2] = SE_PALBANK(4) | 0x0002; // SBB 0
	pse_tiles[0*32 + 3] = SE_PALBANK(4) | 0x0003; // SBB 0
	pse_tiles[0*32 + 4] = SE_PALBANK(4) | 0x0004; // SBB 0
	pse_tiles[0*32 + 5] = SE_PALBANK(4) | 0x0005; // SBB 0
	pse_tiles[0*32 + 6] = SE_PALBANK(4) | 0x0006; // SBB 0
	pse_tiles[0*32 + 7] = SE_PALBANK(4) | 0x0007; // SBB 0
	pse_tiles[0*32 + 8] = SE_PALBANK(4) | 0x0008; // SBB 0
	pse_tiles[0*32 + 9] = SE_PALBANK(4) | 0x0009; // SBB 0
	pse_tiles[0*32 + 10] = SE_PALBANK(4) | 0x000A; // SBB 0
	pse_tiles[0*32 + 11] = SE_PALBANK(4) | 0x000B; // SBB 0
	pse_tiles[0*32 + 12] = SE_PALBANK(4) | 0x000C; // SBB 0
	pse_tiles[0*32 + 13] = SE_PALBANK(4) | 0x000D; // SBB 0
	pse_tiles[0*32 + 14] = SE_PALBANK(4) | 0x000E; // SBB 0
	pse_tiles[0*32 + 15] = SE_PALBANK(4) | 0x000F; // SBB 0
	pse_tiles[0*32 + 16] = SE_PALBANK(4) | 0x0010; // SBB 0

	draw_func();

	while(1);

	return 0;
}

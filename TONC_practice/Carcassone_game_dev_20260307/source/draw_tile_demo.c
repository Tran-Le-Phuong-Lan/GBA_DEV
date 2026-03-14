//
// obj_demo.c
// testing various sprite related things
//
// (20031003 - 20060924, Cearn)

#include <string.h>
#include <tonc.h>
#include "tiles-gbc-v2.h"

int carcassonne_number_of_tiles = 0; // assume 72 carcasonne tiles

// FSM for game state
enum GAME_STATE {
	START,
	GET_TILE,
	PUT_DOWN_TILE
};

enum GAME_STATE current_game_state = PUT_DOWN_TILE;

// define the access to cacasone-tile map
typedef u32 CAS_TILE_MAP[9];
#define cas_tile_map_id	((CAS_TILE_MAP*)tiles_gbcMap_v2)
#define elem_cas_tile_set	((TILE*)tiles_gbc_v2Tiles)


OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

#define CBB_0  0

// ? while some graphic error when SBB_0  = 0 
// because the tileset is stored from tile_mem[CBB_0][0],
// the tile map if also starts from tile_mem[CBB_0][0],
// the tile map (i.e SEs) will be corrupted at the overlap.

// 1 SBB = 2048 byte long  = 64 TILEs (1 TILE 8x8p@ 4bpp)
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
	// tid for Carcassonne graphic starts from 1, tid 0 is the bg tile.
	memcpy32(&tile_mem[CBB_0][1], tiles_gbc_v2Tiles, tiles_gbc_v2TilesLen/sizeof(u32));

	// create and place a palette for bg
	//			P  C
	pal_bg_bank[0][1]= RGB15(31,  0,  0);
	pal_bg_bank[1][1]= RGB15(0,  31,  0);
	pal_bg_bank[2][1]= RGB15(0,  0,  31);
	pal_bg_bank[3][1]= RGB15(31,  31,  0);
	memcpy16(&pal_bg_bank[4][0], tiles_gbc_v2Pal, tiles_gbc_v2PalLen/sizeof(u16));

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
	memcpy16(&pal_obj_bank[2][0], tiles_gbc_v2Pal, tiles_gbc_v2PalLen/sizeof(u16));
	
	
	// Place the cursor 
	// into VRAM: LOW obj memory (cbb == 4)
	// tile_mem[4][0] = tiles[0];

	memcpy32(&tile_mem[4][0], tiles, 128);
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
		// >> 3: divided by 8 to convert to unit [tile]
		//							 32 = width of the map size in unit [tile]	
		se_curr = (obj_y_coord >> 3)*32 + (obj_x_coord >>3);
		// draw a green tile
		int cas_r, cas_col;
		int rand_cat, rand_cat_min, rand_cat_max;
		u16 map_width_unit_tile = 32;

		// allow to get a tile
		if (current_game_state == PUT_DOWN_TILE)
		{
			// update the palette according to Se_index
			// use the tonc `qran_range` to generate the
			// random palette.
			rand_cat = qran_range(0, 2);
			// real Carcassonne Tile ID
			rand_cat = rand_cat << 2;
			rand_cat_min = rand_cat;
			rand_cat_max = rand_cat + 3;

			// load the cas graphics into the cas_tile_buffer = buffer of the cursor
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					//				  CBB TILE_index	
					memcpy32(&tile_mem[4][cas_r*4 + cas_col], 
						&elem_cas_tile_set[cas_tile_map_id[rand_cat][cas_r*3 + cas_col]], 
						8 // 1 TILE = 8x u32
					);
				}
			}

			// example of using BFN_GET to set only the palette bank bit 
			// in the object attribute 2.
			// need palette 2 because this is the same palette of the tileset
			BFN_SET(cursor->attr2, 2, ATTR2_PALBANK);

			// change to a new game state
			current_game_state = GET_TILE;
		};
		

		// put allow to draw/putdown a tile
		int se_idx = 0;
		int cur_se_tid;
		// int rot_dir = 0;
		if(current_game_state == GET_TILE)
			{
				if (key_hit(KEY_A))
				// allow to put down the drawn tile + with conditions
				{
					cur_se_tid = pse[se_curr] & 0x03FF;
					// Tile allowed to be put down if the back ground is empty, and its the first carcas tile
					if (cur_se_tid == 0)
					{
						if(carcassonne_number_of_tiles == 0)
						{
							for (cas_r = 0; cas_r < 3; cas_r++)
							{
								for (cas_col=0; cas_col<3; cas_col++)
								{
									se_idx = se_curr + cas_r*map_width_unit_tile + cas_col; 
									pse[se_idx] = SE_PALBANK(4) | (cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+1);
								}
							}

							// game state is allowed to change only when the tile is put down
							carcassonne_number_of_tiles = carcassonne_number_of_tiles +1;
							current_game_state = PUT_DOWN_TILE;
						}
						else
						{
							// must be adjacent to another tile, and match its edge
							//    MSB                      LSB
							// 0x ADL_ML_ADR_MR_ADB_MB_ADT_MT : cnd_flg
							// ADx: adjacent @ x (x: Top, Bottom, Right, Left)
							// Mx: match @ x
							// 1 = match/ adj; 0 = not match/ not adj
							// 0x 0000_CL_CR_CB_CT: cnd_chk
							// Cx: check @ condition x = ADx == 1 & Mx == 1 
							// 1 = valid; 0 = not valid
							// => the condition for not first carcassonne tile to be put down
							// cnd_flg != 0 && cnd_chk == 0x00001111 == 15
							int cnd_flg = 0x00000000, cnd_chk = 0x00000000;
							
							// check AdT and MT => CT
							int se_indx_top = se_curr - 1*map_width_unit_tile;
							int se_tid_top = pse[se_indx_top] & 0x03FF;
							if (se_tid_top != 0 )
							{
								cnd_flg = cnd_flg | 0x00000010;
								int cnt=0;
									// - middle top tile of obj
									// - middle tile of bg
								if ((cas_tile_map_id[rand_cat][0*3 + 1]+1) == (pse[se_indx_top + 1] & 0x03FF))
								{
									cnd_chk = cnd_chk | 0x00000001;
								}  
			
							}
							else // no top adjacent
							{
								cnd_chk = cnd_chk | 0x00000001;
							}

							// check AdB and MB => CB
							int se_indx_bot = se_curr + 3*map_width_unit_tile;
							int se_tid_bot = pse[se_indx_bot] & 0x03FF;
							if (se_tid_bot != 0 )
							{
								cnd_flg = cnd_flg | 0x00001000;
									// - middle bot tile of obj
									// - middle tile of bg
								if ((cas_tile_map_id[rand_cat][2*3 + 1]+1) == (pse[se_indx_bot + 1] & 0x03FF))
								{
									cnd_chk = cnd_chk | 0x00000010;
								}  
							}
							else // no bot adjacent
							{
								cnd_chk = cnd_chk | 0x00000010;
							}

							// check AdR and MR => CR
							int se_indx_r = se_curr + 3;
							int se_tid_r = pse[se_indx_r] & 0x03FF;
							if (se_tid_r != 0 )
							{
								cnd_flg = cnd_flg | 0x00100000;
							
								//	miidle r tiles of obj
								//  middle l tiles of bg
								if ((cas_tile_map_id[rand_cat][1*3 + 2]+1) == (pse[se_indx_r + 1*map_width_unit_tile] & 0x03FF))
								{
									cnd_chk = cnd_chk | 0x00000100;
								}  
							}
							else // no r adjacent
							{
								cnd_chk = cnd_chk | 0x00000100;
							}

							// check AdL and ML => CL
							int se_indx_l = se_curr - 1;
							int se_tid_l = pse[se_indx_l] & 0x03FF;
							if (se_tid_l != 0 )
							{
								cnd_flg = cnd_flg | 0x10000000;
								//	middle l tiles of obj
								//  middle r tiles of bg
								if ((cas_tile_map_id[rand_cat][1*3 + 0]+1) == (pse[se_indx_l + 1*map_width_unit_tile] & 0x03FF))
								{
									cnd_chk = cnd_chk | 0x00001000;
								}  
								
							}
							else // no r adjacent
							{
								cnd_chk = cnd_chk | 0x00001000;
							}

							// cnd_flg = 0x00000000 | 0x00001000;
							// cnd_chk =  0x00001111;
							// adj and match conds are valid, then allow put down the tile
							if (cnd_flg != 0 && cnd_chk == 0x00001111)
							{
								for (cas_r = 0; cas_r < 3; cas_r++)
								{
									for (cas_col=0; cas_col<3; cas_col++)
									{
										se_idx = se_curr + cas_r*map_width_unit_tile + cas_col; 
										pse[se_idx] = SE_PALBANK(4) | (cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+1);
									}
								}

								// game state is allowed to change only when the tile is put down
								carcassonne_number_of_tiles = carcassonne_number_of_tiles +1;
								current_game_state = PUT_DOWN_TILE;

							}

						};
					};

				}
				else
				{
					// (tonc_input.h) INLINE int key_tri_shoulder()   // R/L : +/-
					// rot_dir = rot_dir + 1*key_tri_shoulder();
					if (key_hit(KEY_R))
					{
						if ( rand_cat == rand_cat_max)
							rand_cat = rand_cat_min;
						else
							rand_cat = rand_cat + 1;
					}
					else if (key_hit(KEY_L))
					{
						if (rand_cat == rand_cat_min)
							rand_cat = rand_cat_max;
						else
							rand_cat = rand_cat -1;
					}
					else
					{
						// nothing
					};
					
					//rand_cat = rand_cat + rot_dir;
					// load the cas graphics into the cas_tile_buffer = buffer of the cursor
					for (cas_r = 0; cas_r < 3; cas_r++)
					{
						for (cas_col=0; cas_col<3; cas_col++)
						{
							//				  CBB TILE_index	
							memcpy32(&tile_mem[4][cas_r*4 + cas_col], 
								&elem_cas_tile_set[cas_tile_map_id[rand_cat][cas_r*3 + cas_col]], 
								8 // 1 TILE = 8x u32
							);
						}
					}
	
				};

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

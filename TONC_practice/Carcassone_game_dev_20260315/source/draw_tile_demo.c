//
// obj_demo.c
// testing various sprite related things
//
// (20031003 - 20060924, Cearn)

#include <string.h>
#include <tonc.h>
#include "tiles-gbc-v2.h"

// ===========
// CARCASSONNE GAME RULES
// ===========
int carcassonne_number_of_tiles = 0; // assume 72 carcasonne tiles

// FSM for game state
enum GAME_STATE {
	START,
	GET_TILE,
	PUT_DOWN_TILE
};

enum GAME_STATE current_game_state = PUT_DOWN_TILE;

// define the access to cacasone-tile map
typedef unsigned short CAS_TILE_MAP[9];
#define cas_tile_map_id	((CAS_TILE_MAP*)tiles_gbcMap_v2)
#define elem_cas_tile_set	((TILE8*)tiles_gbc_v2Tiles)

// ===========
// OBJ/ SPRITE
// ===========
OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

// ===========
// BG
// ===========
BG_AFFINE bgaff;

#define CBB_0  0
// 1 SBB = 2048 byte long  = 64 TILEs (1 TILE 8x8p@ 4bpp = 32 bytes) = 32 TILE8s (1 TILE8 8x8p@ 8bpp = 64 bytes)
#define SBB_0  1
// typedef u8  SCR_AFF_ENTRY, SAE;		//!< Type for affine screen entries (tonc_types)
// typedef u16 SCR_ENTRY, SE;			//!< Type for screen entries
SCR_ENTRY *bg0_map= se_mem[SBB_0];
// int map_width_unit_tile = 128; // [DTILE]
int map_width_unit_tile = 32;

void init_reg_bg ()
{
	// initialize a background
	// REG_BG0CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_32x32;
											  // BG size: 32x32 DTILEs
	REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_32x32; 
	// REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_128x128; 
	bgaff= bg_aff_default;

		const TILE8 tiles[1]=
	{
		// bg tile8: 8bit/pixel = 2hex/pixel
		// {{0x10000001, 0x01111110, 0x01111110, 0x01111110,
		//   0x01111110, 0x01111110, 0x01111110, 0x10000001}},

		{{0x00000010, 0x01000000, 
		  0x01010100, 0x00010101,
		  0x01010100, 0x00010101, 
		  0x01010100, 0x00010101,
		  0x01010100, 0x00010101, 
		  0x01010100, 0x00010101,
		  0x01010100, 0x00010101, 
		  0x00000010, 0x01000000}}
	};

	// Place the bg tile 
	// into VRAM: cbb == 0
	tile8_mem[CBB_0][0] = tiles[0];
	// tid for Carcassonne graphic starts from 1, tid 0 is the bg tile.
	memcpy32(&tile8_mem[CBB_0][1], tiles_gbc_v2Tiles, tiles_gbc_v2TilesLen/sizeof(u32));

	// no palette, only 256 colors
	memcpy32(pal_bg_mem, tiles_gbc_v2Pal, tiles_gbc_v2PalLen/sizeof(u32));

}

void init_reg_obj ()
{	
	// SPrite is 8x8p@8bpp, bcz it shares data with the background (8x8p@8bpp)

	// load in 256 palette (shared data with the background), 
	memcpy32(pal_obj_mem, tiles_gbc_v2Pal, tiles_gbc_v2PalLen/sizeof(u32));
	
	
	// Place the cursor/sprite
	// into VRAM: LOW obj memory (cbb == 4)
	// tile_mem[4][0] = tiles[0];
	// first graphic is loaded in the func `draw_func` - game state `PUT_DOWN_TILE`

}

void draw_func()
{
	int x= 0, y= 0; // [pixel]

	u32 tid= 0, pb= 0;		// tile id, pal-bank

	OBJ_ATTR *cursor= &obj_buffer[0];
	obj_set_attr(cursor, 
		ATTR0_SQUARE | ATTR0_8BPP,				// Square, regular sprite
												// & 8bpp
		ATTR1_SIZE_32,							// 32x32p, = 4x4 Tile
		// the object priority is zero
		ATTR2_ID(tid));				// tile 0 = base tile of the sprite
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
		u32 sae_curr;
		// get the cursor (object) position (using Tonc BF_GET())
		obj_x_coord = BFN_GET(cursor->attr1, ATTR1_X);
		obj_y_coord = BFN_GET(cursor->attr0, ATTR0_Y);
		// calculate the Se_index, map size 32x32t
		// >> 3: divided by 8 to convert to unit [tile]
		//							 32 = width of the map size in unit [tile]	
		sae_curr = (obj_y_coord >> 3)*32 + (obj_x_coord >>3);
		se_curr = sae_curr >> 1;
		// draw a green tile
		int cas_r, cas_col;
		int rand_cat, rand_cat_min, rand_cat_max;

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
					memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
						&elem_cas_tile_set[cas_tile_map_id[rand_cat][cas_r*3 + cas_col]], 
						16 // 1 DTILE = 16 x u32
					);
				}
			}

			// memcpy32(&tile_mem[4][0], 
			// 			&elem_cas_tile_set[0], 
			// 			16); // 1 DTILE = 16 x u32
			
			// memcpy32(&tile_mem[4][2], 
			// 			&elem_cas_tile_set[1], 
			// 			16); // 1 DTILE = 16 x u32
			
			// memcpy32(&tile_mem[4][4], 
			// 			&elem_cas_tile_set[3], 
			// 			16); // 1 DTILE = 16 x u32
			
			// change to a new game state
			current_game_state = GET_TILE;
		};
		

		// put allow to draw/putdown a tile
		int se_idx = 0, sea_idx = 0;
		int cur_se_tid;
		// int rot_dir = 0;
		if(current_game_state == GET_TILE)
			{
				if (key_hit(KEY_A))
				// allow to put down the drawn tile + with conditions
				{
					// pse[se_curr] = 2 SAE at once = the current one and the one adjacent to it
					// , because SAE is 8-bit, while pse[se_curr] is 16-bit.
					if (sae_curr % 2 == 0)
					{
						// lower 8-bit of pse
						cur_se_tid = pse[se_curr] & 0x00FF;
					}
					else
					{
						// higer 8-bit of pse
						cur_se_tid = pse[se_curr] & 0xFF00;
					}
					
					// Tile allowed to be put down if the back ground is empty, and its the first carcas tile
					if (cur_se_tid == 0)
					{
						if(carcassonne_number_of_tiles == 0)
						{
							for (cas_r = 0; cas_r < 3; cas_r++)
							{
								for (cas_col=0; cas_col<3; cas_col++)
								{
									sea_idx = sae_curr + cas_r*map_width_unit_tile + cas_col; 
									se_idx = sea_idx >> 1;
									if (sea_idx % 2 == 0)
									{
										// write to lower 8-bit of pse, preseve the higher 8-bit of pse
										pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+1) & 0x00FF);
									}
									else
									{
										// write higer 8-bit of pse, preserve the lower 8 bit of pse
										pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+1)<<8)  & 0xFF00);
									}
								}
							}
							// se_idx = se_curr; 
							// if (sae_curr % 2 == 0)
							// {
							// 	// write to lower 8-bit of pse, preseve the higher 8-bit of pse
							// 	pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[rand_cat][0*3 + 0]+1) & 0x00FF);
							// }
							// else
							// {
							// 	// write higer 8-bit of pse, preserve the lower 8 bit of pse
							// 	pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[rand_cat][0*3 + 0]+1)<<8)  & 0xFF00);
							// }

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
							int sea_indx_top = sae_curr - 1*map_width_unit_tile;
							int se_indx_top = sea_indx_top >> 1;
							unsigned short sae_tid_top;
							int lower_or_higher;
							lower_or_higher = sea_indx_top % 2; 
							if (lower_or_higher == 0)
							{
								// extract lower 8-bit of pse
								sae_tid_top = pse[se_indx_top] & 0x00FF;
							}
							else
							{
								// extract higher 8-bit of pse
								sae_tid_top = (pse[se_indx_top] & 0xFF00) >> 8;
							}
							if (sae_tid_top != 0 )
							{
								cnd_flg = cnd_flg | 0x00000010;
									// - middle top tile of obj
									// - middle tile of bg
								int sea_indx_top_next = sea_indx_top + 1;
								int se_indx_top_next = sea_indx_top_next >> 1;
								unsigned short sae_tid_top_next;
								lower_or_higher = sea_indx_top_next % 2;
								if (lower_or_higher == 0)
								{
									// extract lower 8-bit of pse
									sae_tid_top_next = pse[se_indx_top_next] & 0x00FF;
								}
								else
								{
									// extract lower 8-bit of pse
									sae_tid_top_next = (pse[se_indx_top_next] & 0xFF00) >> 8;
								}
								if ((cas_tile_map_id[rand_cat][0*3 + 1]+1) == sae_tid_top_next)
								{
									cnd_chk = cnd_chk | 0x00000001;
								}  
			
							}
							else // no top adjacent
							{
								cnd_chk = cnd_chk | 0x00000001;
							}

							// check AdB and MB => CB
							int sae_indx_bot = sae_curr + 3*map_width_unit_tile;
							int se_indx_bot = sae_indx_bot >> 1;
							lower_or_higher = sae_indx_bot % 2;
							unsigned short sae_tid_bot;
							if (lower_or_higher == 0)
							{
								// extract lower 8-bit of pse
								sae_tid_bot = pse[se_indx_bot] & 0x00FF;
							}
							else
							{
								// extract higher 8-bit of pse
								sae_tid_bot = (pse[se_indx_bot] & 0xFF00) >> 8;
							}
							if (sae_tid_bot != 0 )
							{
								cnd_flg = cnd_flg | 0x00001000;
									// - middle bot tile of obj
									// - middle tile of bg
								int sea_indx_bot_next = sae_indx_bot + 1;
								int se_indx_bot_next = sea_indx_bot_next >> 1;
								unsigned short sae_tid_bot_next;
								lower_or_higher = sea_indx_bot_next % 2;
								if (lower_or_higher == 0)
								{
									// extract lower 8-bit of pse
									sae_tid_bot_next = pse[se_indx_bot_next] & 0x00FF;
								}
								else
								{
									// extract lower 8-bit of pse
									sae_tid_bot_next = (pse[se_indx_bot_next] & 0xFF00) >> 8;
								}
								if ((cas_tile_map_id[rand_cat][2*3 + 1]+1) == sae_tid_bot_next)
								{
									cnd_chk = cnd_chk | 0x00000010;
								}  
							}
							else // no bot adjacent
							{
								cnd_chk = cnd_chk | 0x00000010;
							}

							// check AdR and MR => CR
							int sae_indx_r = sae_curr + 3;
							int se_indx_r = sae_indx_r >> 1;
							lower_or_higher = sae_indx_r % 2;
							unsigned short sae_tid_r;
							if (lower_or_higher == 0)
							{
								// extract lower 8-bit of pse
								sae_tid_r = pse[se_indx_r] & 0x00FF;
							}
							else
							{
								// extract higher 8-bit of pse
								sae_tid_r = (pse[se_indx_r] & 0xFF00) >> 8;
							}
							if (sae_tid_r != 0 )
							{
								cnd_flg = cnd_flg | 0x00100000;
							
								//	miidle r tiles of obj
								//  middle l tiles of bg
								int sea_indx_r_mid = sae_indx_r + 1*map_width_unit_tile;
								int se_indx_r_mid = sea_indx_r_mid >> 1;
								unsigned short sae_tid_r_mid;
								lower_or_higher = sea_indx_r_mid % 2;
								if (lower_or_higher == 0)
								{
									// extract lower 8-bit of pse
									sae_tid_r_mid = pse[se_indx_r_mid] & 0x00FF;
								}
								else
								{
									// extract lower 8-bit of pse
									sae_tid_r_mid = (pse[se_indx_r_mid] & 0xFF00) >> 8;
								}
								if ((cas_tile_map_id[rand_cat][1*3 + 2]+1) == sae_tid_r_mid)
								{
									cnd_chk = cnd_chk | 0x00000100;
								}  
							}
							else // no r adjacent
							{
								cnd_chk = cnd_chk | 0x00000100;
							}

							// check AdL and ML => CL
							int sae_indx_l = sae_curr - 1;
							int se_indx_l = sae_indx_l >> 1;
							lower_or_higher = sae_indx_l % 2;
							unsigned short sae_tid_l;
							if (lower_or_higher == 0)
							{
								// extract lower 8-bit of pse
								sae_tid_l = pse[se_indx_l] & 0x00FF;
							}
							else
							{
								// extract higher 8-bit of pse
								sae_tid_l = (pse[se_indx_l] & 0xFF00) >> 8;
							}
							if (sae_tid_l != 0 )
							{
								cnd_flg = cnd_flg | 0x10000000;
								//	middle l tiles of obj
								//  middle r tiles of bg
								int sea_indx_l_mid = sae_indx_l + 1*map_width_unit_tile;
								int se_indx_l_mid = sea_indx_l_mid >> 1;
								unsigned short sae_tid_l_mid;
								lower_or_higher = sea_indx_l_mid % 2;
								if (lower_or_higher == 0)
								{
									// extract lower 8-bit of pse
									sae_tid_l_mid = pse[se_indx_l_mid] & 0x00FF;
								}
								else
								{
									// extract lower 8-bit of pse
									sae_tid_l_mid = (pse[se_indx_l_mid] & 0xFF00) >> 8;
								}
								if ((cas_tile_map_id[rand_cat][1*3 + 0]+1) == sae_tid_l_mid)
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
										sea_idx = sae_curr + cas_r*map_width_unit_tile + cas_col; 
										se_idx = sea_idx >> 1;
										if (sea_idx % 2 == 0)
										{
											// write to lower 8-bit of pse, preseve the higher 8-bit of pse
											pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+1) & 0x00FF);
										}
										else
										{
											// write higer 8-bit of pse, preserve the lower 8 bit of pse
											pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+1)<<8)  & 0xFF00);
										}
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
							memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
								&elem_cas_tile_set[cas_tile_map_id[rand_cat][cas_r*3 + cas_col]], 
								16 // 1 DTILE = 16 x u32
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

	// REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 |DCNT_OBJ | DCNT_OBJ_1D;
	REG_DISPCNT= DCNT_MODE1 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;

	init_reg_obj();
	init_reg_bg();

	// init the OAM, and make sure that 
	// the all the sprites (all 128 sprites) are hidden at the beginning.
	// The author inits 128 sprites, 
	// even though there is only one sprite (= 1 data of a sprite image stored in VRAM).
	// the function is in `./toolbox.c` 
	oam_init(obj_buffer, 128);
	
	// test background tiles
	// SAE (screen affine entry) only 8 bit, and we have to write 16/32 bit !!!! because SEs is in VRAM
	// write 2 DTILEs at once
	SCR_ENTRY *pse_tiles= bg0_map;
	// 0th row
					    // SAE 1, SAE 0 
	pse_tiles[0*(map_width_unit_tile >> 1) + 0] = 0x0001;
	pse_tiles[0*(map_width_unit_tile >> 1) + 0] = 0x0200 | pse_tiles[0*(map_width_unit_tile >> 1) + 0];
						// SAE 3, SAE 2
	pse_tiles[0*16 + 1] = 0x0302; 
	pse_tiles[0*16 + 2] = 0x0506; 
	pse_tiles[0*16 + 3] = 0x0807; 
	pse_tiles[0*16 + 4] = 0x0A09; 
	pse_tiles[0*16 + 5] = 0x0C0B; 
	pse_tiles[0*16 + 6] = 0x0E0D; 
	pse_tiles[0*16 + 7] = 0x100F;
					    // SAE 13, SAE 12
	pse_tiles[0*16 + ((map_width_unit_tile >> 1) - 2)] = 0x1413;
				// the screen fits only 30 tiles, 
				// so last 2 tiles can not be seen,
				// if no screen rolling is applied.
	pse_tiles[0*16 + 15] = 0x1413;

	// 1st row
	pse_tiles[1*(map_width_unit_tile >> 1) + 0] = 0x1211;
	pse_tiles[1*16 + 1] = 0x1615; 

	draw_func();

	while(1);

	return 0;
}

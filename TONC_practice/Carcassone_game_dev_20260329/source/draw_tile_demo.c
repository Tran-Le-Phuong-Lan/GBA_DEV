//
// reference: obj_demo.c, sbb_aff.c, 
// Carcassonne GBA
// 20250115 - , Tran Le Phuong Lan & David Guttandin
// 

#include <string.h>
#include <tonc.h>
#include "tiles-gbc-v2.h"

// ===========
// CARCASSONNE GAME RULES
// ===========
#define CAR_TILES_MAX 72
int carcassonne_number_of_tiles = 0; // assume 72 carcasonne tiles

// FSM for game state
enum GAME_STATE {
	START,
	GET_TILE,
	PUT_DOWN_TILE,
	END
};

enum GAME_STATE current_game_state = START;

// define the access to cacasone-tile map
typedef unsigned short CAS_TILE_MAP[9];
#define cas_tile_map_id	((CAS_TILE_MAP*)tiles_gbcMap_v2)
#define elem_cas_tile_set	((TILE8*)tiles_gbc_v2Tiles)

#define CAR_CAT 32 // cacarcasonne category number (tile with different graphic features)

int car_cat_max[32] = {
	// CAT 17
	4,
	// CAT 1
	8,
	// CAT 2
	1,
	// CAT 3
	7,
	// CAT 4
	1,
	// CAT 5
	4,
	// CAT 6
	1,
	// CAT 7
	4,
	// CAT 8
	2,
	// CAT 9
	4,
	// CAT 10
	1,
	// CAT 11
	3,
	// CAT 12
	3,
	// CAT 13
	1,
	// CAT 14
	1,
	// CAT 15
	2,
	// CAT 16
	1,
	// CAT 18
	3,
	// CAT 19
	2,
	// CAT 20
	1,
	// CAT 21
	1,
	// CAT 22
	1,
	// CAT 23
	3,
	// CAT 24
	2,
	// CAT 25
	1,
	// CAT 26
	2,
	// CAT 27
	1,
	// CAT 28
	2,
	// CAT 29
	1,
	// CAT 30
	1,
	// CAT 31
	1,
	// CAT 32
	2
};

// ===========
// OBJ/ SPRITE
// ===========
OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

// ===========
// BG
// ===========
BG_AFFINE bgaff; // .8f = the last eight bit representing the fractional, 1 fraction = 1/256 [pixel]

#define INIT_SCR_X_OFF  0x0188 // = 392 [pixel]
#define INIT_SCR_Y_OFF  0x01B0 // = 432 [pixel]

#define CBB_0  0
// 1 CBB = 8 SBBs
// 1 SBB either = 2048 byte long  = 64 TILEs (1 TILE 8x8p@ 4bpp = 32 bytes) = 32 TILE8s (1 TILE8 8x8p@ 8bpp = 64 bytes)
// 		 or = 32 x 32 [tile] regular map (bcz 1 SE = 16 bits = 2 bytes) = 64 x 32 [tile] aff map (1 SAE = 8 bit = 1 byte)
// In this context, we store 28 Tiles from 0th CBB ~ 0th SBB in 0th CBB 
#define SBB_0  1
// typedef u8  SCR_AFF_ENTRY, SAE;		//!< Type for affine screen entries (tonc_types)
// typedef u16 SCR_ENTRY, SE;			//!< Type for screen entries
SCR_ENTRY *bg0_map= se_mem[SBB_0];
// aff bg 128 x 128 tile = 8 SBBs 
int map_width_unit_tile = 128; // [DTILE]

// ===========
// BG TEXT
// ===========
#define CBB_0_TEXT 2  // tiles for text	
#define SBB_0_TEXT 31 // map for text

// ==========
// FUNCS
// ==========
void win_textbox(int bgnr, int left, int top, int right, int bottom, int bldy)
{
	REG_WIN0H= left<<8 | right; 
	REG_WIN0V=  top<<8 | bottom;
	REG_WIN0CNT= WIN_ALL | WIN_BLD;
	REG_WINOUTCNT= WIN_ALL;

	REG_BLDCNT= (BLD_ALL&~BIT(bgnr)) | BLD_BLACK;
	REG_BLDY= bldy; // fade levels

	REG_DISPCNT |= DCNT_WIN0;

	tte_set_margins(left, top, right, bottom);
}

void init_reg_bg ()
{
	// initialize a background
	// REG_BG0CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_32x32;
											  // BG size: 32x32 DTILEs
	// REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_32x32; 
	REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_128x128; 
	// set the initial position of the screen
	bgaff.pa = bg_aff_default.pa;
	bgaff.pb = bg_aff_default.pb;
	bgaff.pc = bg_aff_default.pc;
	bgaff.pd = bg_aff_default.pd;
	bgaff.dx = INIT_SCR_X_OFF<<8; // = 24 [pixel] to the right 
	bgaff.dy = INIT_SCR_Y_OFF<<8;
	REG_BG_AFFINE[2]= bgaff;

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
	int car_cat_track[32] = {0};

	// === aff bg
	AFF_SRC_EX asx=
	{
		INIT_SCR_X_OFF<<8, INIT_SCR_Y_OFF<<8,			// Map coords.
		0, 0,				// Screen coords.
		0x0100, 0x0100, 0		// Scales (= x1) and angle (= 0 degree).
	};

	// ==== obj
	// set the inital position of the cursor sprite
	int x= 104, y= 56; // [pixel] 

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

	// === enable a timer
	REG_TM2D =  -0x065E;
	REG_TM2CNT= TM_ENABLE | TM_FREQ_1024; 
	// 1 period = 1024 default clock cycle
	// = 1024 * (1/16.78 Mhz) = 61 us 

	// tick every ~ 100ms
	REG_TM3CNT=  TM_ENABLE | TM_CASCADE; 
	
	u32 sec = -1;

	while(1)
	{
		VBlankIntrWait(); // check at the end of each frame
		key_poll();
		
		// === obj
		u16 obj_x_coord, obj_y_coord; // in unit [pixel]

		// === BG
		SCR_ENTRY *pse= bg0_map;
		u32 se_curr;
		u32 sae_curr;
		// int scr_x_offset = 0, scr_y_offset =0;
		int bg_x_offset = 0, bg_y_offset =0;

		// === FUNCS/ ACTIONS
		// start timer, wait for 10ms ~ 163 ticks
		if (REG_TM3D != sec)
		{
			sec = REG_TM3D;
			// == OBJ, moving relative to the screen = screen is static
			// left/right
			x += 0; // 24*key_tri_horz(); // [pixel]
			// move up/down
			y += 0; // 24*key_tri_vert(); // [pixel]
			obj_set_pos(cursor, x, y);
			
			// == BG rolling, i am moving the map, the screen/camera is static
			asx.tex_x += (24*key_tri_horz()) << 8;
			asx.tex_y += (24*key_tri_vert()) << 8;

			// == SCR rolling, i am moving the screen, the map is static
			// scr_x_offset = 24*key_tri_horz();  // [pixel]
			// scr_y_offset = 24*key_tri_vert(); 
			// asx.scr_x += (scr_x_offset); // converted to unit [pixel] for REG_BGX
			// asx.scr_y += (scr_y_offset); // converted to unit [pixel] for REG_BGY

			bg_rotscale_ex(&bgaff, &asx);
			REG_BG_AFFINE[2]= bgaff;
			
		}

		// get the cursor (object) position (using Tonc BF_GET())
		obj_x_coord = BFN_GET(cursor->attr1, ATTR1_X);
		obj_y_coord = BFN_GET(cursor->attr0, ATTR0_Y);
		// calculate the Se_index, map size 32x32t
		// >> 3: divided by 8 to convert to unit [tile]
		//							 * 32 = width of the map size in unit [tile]	
		sae_curr = ((obj_y_coord+ (bgaff.dy >> 8)) >> 3)*map_width_unit_tile + ((obj_x_coord + (bgaff.dx >> 8)) >>3);
		se_curr = sae_curr >> 1;
		// draw a green tile
		int cas_r, cas_col;
		int rand_cat, rand_cat_min, rand_cat_max, rand_cat_id;

		// game start
		if (current_game_state == START)
		{
			int se_idx = 0, sea_idx = 0;
			// put down the starter tile
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					sea_idx = sae_curr + cas_r*map_width_unit_tile + cas_col; 
					se_idx = sea_idx >> 1;
					if (sea_idx % 2 == 0)
					{
						// write to lower 8-bit of pse, preseve the higher 8-bit of pse
						//														0 = CARCASONNE CAT 0 = STARTER TILE
						pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[0][cas_r*3 + cas_col]+1) & 0x00FF);
					}
					else
					{
						// write higer 8-bit of pse, preserve the lower 8 bit of pse
						pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[0][cas_r*3 + cas_col]+1)<<8)  & 0xFF00);
					}
				}
			}

			// game state is allowed to change only when the tile is put down
			carcassonne_number_of_tiles = carcassonne_number_of_tiles +1;
			car_cat_track[0] = car_cat_track[0] + 1; 
			current_game_state = PUT_DOWN_TILE;
		}

		// allow to get a tile, only if there is still carcassonne tiles to take
		if (current_game_state == PUT_DOWN_TILE && carcassonne_number_of_tiles < CAR_TILES_MAX)
		{
			// update the palette according to Se_index
			// use the tonc `qran_range` to generate the
			// random Carcassonne category.
			rand_cat = qran_range(0, CAR_CAT);
			rand_cat_id = rand_cat;
				// generate another carcasonne category, if the generated category is out of tiles.
			while(car_cat_track[rand_cat_id] == car_cat_max[rand_cat_id])
			{
				rand_cat = qran_range(0, CAR_CAT);
				rand_cat_id = rand_cat;
			}
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
			
			// change to a new game state
			current_game_state = GET_TILE;
		};
		

		// put allow to draw/putdown a tile
		if(current_game_state == GET_TILE && carcassonne_number_of_tiles < CAR_TILES_MAX)
			{
				int se_idx = 0, sea_idx = 0;
				int cur_se_tid;
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
							else
							{
								// CARCAS TID 25 (garden) == CARCAS TID 1 (green) [index in VRAM ]
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in .s file]
								if ((sae_tid_top_next == 25 && cas_tile_map_id[rand_cat][0*3 + 1] == 0) 
									|| (sae_tid_top_next == 1 && cas_tile_map_id[rand_cat][0*3 + 1] == 24))
								{
									cnd_chk = cnd_chk | 0x00000001;
								} 
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
							else
							{
								// CARCAS TID 25 (garden) == CARCAS TID 1 (green) [index in VRAM ]
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in .s file]
								if ((sae_tid_bot_next == 25 && cas_tile_map_id[rand_cat][2*3 + 1] == 0) 
									|| (sae_tid_bot_next == 1 && cas_tile_map_id[rand_cat][2*3 + 1] == 24))
								{
									cnd_chk = cnd_chk | 0x00000010;
								} 
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
							else
							{
								// CARCAS TID 25 (garden) == CARCAS TID 1 (green) [index in VRAM ]
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in .s file]
								if ((sae_tid_r_mid == 25 && cas_tile_map_id[rand_cat][1*3 + 2] == 0) 
									|| (sae_tid_r_mid == 1 && cas_tile_map_id[rand_cat][1*3 + 2] == 24))
								{
									cnd_chk = cnd_chk | 0x00000100;
								} 
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
							else
							{
								// CARCAS TID 25 (garden) == CARCAS TID 1 (green) [index in VRAM ]
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in .s file]
								if ((sae_tid_l_mid == 25 && cas_tile_map_id[rand_cat][1*3 + 0] == 0) 
									|| (sae_tid_l_mid == 1 && cas_tile_map_id[rand_cat][1*3 + 0] == 24))
								{
									cnd_chk = cnd_chk | 0x00001000;
								} 
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
							car_cat_track[rand_cat_id] = car_cat_track[rand_cat_id] + 1;
							current_game_state = PUT_DOWN_TILE;

						}

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
		
		if (carcassonne_number_of_tiles == CAR_TILES_MAX)
		{
			current_game_state = END;
		}

		if (current_game_state == END)
		{
				const TILE8 end_tiles[1]=
			{
				// bg tile8: 8bit/pixel = 2hex/pixel
				// {{0x10000001, 0x01111110, 0x01111110, 0x01111110,
				//   0x01111110, 0x01111110, 0x01111110, 0x10000001}},

			{{0x00000050, 0x05000000, 
			  0x05050500, 0x00050505,
			  0x05050500, 0x00050505, 
			  0x05050500, 0x00050505,
			  0x05050500, 0x00050505, 
			  0x05050500, 0x00050505,
			  0x05050500, 0x00050505, 
			  0x00000050, 0x05000000}}
			};
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					//				  CBB TILE_index	
					memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
						&end_tiles[0], 
						16 // 1 DTILE = 16 x u32
					);
				}
			}	
		}
	
		// In this example, there is only 1 sprite of size 64 pixel x 64 pixel (= 8 TILE x 8 TILE)
		oam_copy(oam_mem, obj_buffer, 1);	// only need to update one

		// write text
		tte_printf("#{es;P}Tile ID: %d %d/%d\t\nLeft: %d/%d",
			rand_cat_id, car_cat_track[rand_cat_id], car_cat_max[rand_cat_id],
			carcassonne_number_of_tiles, CAR_TILES_MAX);
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

	// BG 0 = for text
	// BG 2 = Carcassonne game
	REG_DISPCNT= DCNT_MODE1 | DCNT_BG0 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;

	// init text
	// Init BG 0 for text on screen entries, using CBB 
	tte_init_chr4c_b4_default(0, BG_CBB(CBB_0_TEXT)|BG_SBB(SBB_0_TEXT));
	tte_init_con();
	// left, top, right, bottom
	// tte_set_margins(8, 120, 232, 156);
	//	void win_textbox(int bgnr, int left, int top, int right, int bottom, int bldy)
	//	bgnr = bg for windows; coordinates in unit [pixel]
	//			   l,       r,
	//				  t         b	
	win_textbox(0, 8, 120, 232, 156, 8);

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
	pse_tiles[1*(map_width_unit_tile >> 1) + 1] = 0x1615; 

	draw_func();

	while(1);

	return 0;
}

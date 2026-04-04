//
// reference: obj_demo.c, sbb_aff.c, 
// Carcassonne GBA
// 20250115 - , Tran Le Phuong Lan & David Guttandin
// 

#include <string.h>
#include <stdlib.h> 
// abs()
#include <tonc.h>
#include "tiles-carcas_v20260329.h"
#include "carcas_data.h"
#include <math.h> 
// ceil(), floor()

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
	MEEPLE,
	END
};

enum GAME_STATE current_game_state = START;

// define the access to cacasone-tile map
typedef unsigned short CAS_TILE_MAP[9];
#define cas_tile_map_id	((CAS_TILE_MAP*)tiles_gbcMap_v2)
#define bg_tile_map_id	((CAS_TILE_MAP*)Bg_tile)
#define elem_cas_tile_set	((TILE8*)tiles_carcas_v20260329Tiles)

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

// for the virtual backround (vbg)
typedef struct VBG_BORDER
{
	s32 right, left, top, bottom;
} ALIGN4 VBG_BORDER;

#define CAR_BG_ID 150 // > the CAR ID available in `tiles-carcas_v20260329.s`

typedef struct COORD_2D
{
	s32 x;
	s32 y;
} ALIGN4 COORD_2D;

#define CAR_MAP_WIDTH_x 41 // [ctile]
#define CAR_MAP_HEIGHT_y 41

// ===========
// OBJ/ SPRITE
// ===========
OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;

#define INIT_OBJ_X 104 // [pixel]
#define INIT_OBJ_Y 56

// ===========
// BG
// ===========
BG_AFFINE bgaff; // .8f = the last eight bit representing the fractional, 1 fraction = 1/256 [pixel]

#define INIT_BG_X_OFF  0x0088 // = 136 [pixel]
#define INIT_BG_Y_OFF  0x00B0 // = 176 [pixel]

#define CBB_0  0
// 1 CBB = 8 SBBs
// 1 SBB either = 2048 byte long  = 64 TILEs (1 TILE 8x8p@ 4bpp = 32 bytes) = 32 TILE8s (1 TILE8 8x8p@ 8bpp = 64 bytes)
// 		 or = 32 x 32 [tile] regular map (bcz 1 SE = 16 bits = 2 bytes) = 64 x 32 [tile] aff map (1 SAE = 8 bit = 1 byte)
// In this context, we store 28 Tiles from 0th CBB ~ 0th SBB in 0th CBB 
#define SBB_0  2
// typedef u8  SCR_AFF_ENTRY, SAE;		//!< Type for affine screen entries (tonc_types)
// typedef u16 SCR_ENTRY, SE;			//!< Type for screen entries
SCR_ENTRY *bg0_map= se_mem[SBB_0];
// aff bg 64 x 64 tile = 2 SBBs 
int map_width_unit_tile = 64; // [DTILE]
int map_height_unit_tile = 64; // [DTILE]
// because of 2 manual tile added manually (background tile, transparent tile)
#define CAR_TILE_OFFSET_IN_VRAM 2

// ===========
// BG TEXT
// ===========
#define CBB_0_TEXT 2  // tiles for text	
#define SBB_0_TEXT 31 // map for text

// ===========
// GBA SCREEN
// ===========
#define SCR_WIDTH_uPx 240 // [pixel]
#define SCR_HEIGHT_uPx 160  

// ==========
// FUNCS
// ==========
// [ctile] = 3x3 [tile]
// [tile] = 8x8 [pixel]
//					 = sae_curr_x		= sae_curr_y
void map_tile_to_ctile (int tile_x, int tile_y, s32* ctile_x, s32* ctile_y)
{
	// receive unit tile, return unit ctile

	s32 REFERENCE_CTILE = 20; // [ctile], ctile_dx = 20, ctile_dy=20, bcz index starts from 0
	s32 INIT_TILE_DX = (INIT_OBJ_X + (INIT_BG_X_OFF)) >> 3; // [tile]
	s32 INIT_TILE_DY = (INIT_OBJ_Y + (INIT_BG_Y_OFF)) >> 3; //

	s32 move_ctile_dx = ((tile_x - INIT_TILE_DX)/3);
	s32 move_ctile_dy = ((tile_y - INIT_TILE_DY)/3);
	s32 reminder_x=abs(tile_x-INIT_TILE_DX)%3;
	s32 reminder_y=abs(tile_y - INIT_TILE_DY)%3;
	if (tile_x >= 0)
	{
		*ctile_x = move_ctile_dx + REFERENCE_CTILE; // [ctile]	
	} 
	else
	{
		if (reminder_x == 0)
		{
			*ctile_x = move_ctile_dx + REFERENCE_CTILE;
		}
		else
		{
			*ctile_x = move_ctile_dx -1 + REFERENCE_CTILE;
		}
	}

	if(tile_y >= 0)
	{
		*ctile_y = move_ctile_dy + REFERENCE_CTILE;  
	}
	else
	{
		if (reminder_y == 0)
		{
			*ctile_y = move_ctile_dy + REFERENCE_CTILE;
		}
		else
		{
			*ctile_y = move_ctile_dy -1 + REFERENCE_CTILE;
		}
	}
}

void map_ctile_to_tile (s32 ctile_x, s32 ctile_y, s32* tile_x, s32* tile_y, bool return_wrap_tile)
{
	// receive unit ctile, return unit tile. Return the tile within the bg size !

	// must be within the bg size
	s32 REFERENCE_CTILE = 20; // [ctile], ctile_dx = 20, ctile_dy=20, bcz index starts from 0
	s32 INIT_TILE_DX = (INIT_OBJ_X + (INIT_BG_X_OFF)) >> 3; // [tile]
	s32 INIT_TILE_DY = (INIT_OBJ_Y + (INIT_BG_Y_OFF)) >> 3; // 
	s32 tile_x_max = map_width_unit_tile -1, tile_y_max = map_height_unit_tile -1;

	*tile_x = (ctile_x - REFERENCE_CTILE) * 3 + INIT_TILE_DX;
	*tile_y = (ctile_y - REFERENCE_CTILE)*3 + INIT_TILE_DY;
	if (return_wrap_tile == true)
	{
		if (*tile_x > tile_x_max) 
		{
			*tile_x = *tile_x - map_width_unit_tile;
		}
		if (*tile_x < 0)
		{
			*tile_x = map_width_unit_tile + *tile_x;
		}


		if (*tile_y > tile_y_max) 
		{
			*tile_y = *tile_y - map_height_unit_tile;
		}
		if (*tile_y < 0)
		{
			*tile_y = map_height_unit_tile + *tile_y;
		}  
	}
	else
	{
		// nothing
	}

}

void wrapping_tile_coord (COORD_2D* tile_coord)
{
	s32 tile_x_max = map_width_unit_tile -1, tile_y_max = map_height_unit_tile -1;
	if (tile_coord->x > tile_x_max) 
	{
		tile_coord->x = tile_coord->x - map_width_unit_tile;
	}
	if (tile_coord->x < 0)
	{
		tile_coord->x = map_width_unit_tile + tile_coord->x;
	}


	if (tile_coord->y > tile_y_max) 
	{
		tile_coord->y = tile_coord->y - map_height_unit_tile;
	}
	if (tile_coord->y < 0)
	{
		tile_coord->y = map_height_unit_tile + tile_coord->y;
	}  
}

void render_bg (s32 tile_prev_x, s32 tile_prev_y, s32 tile_cur_x, s32 tile_cur_y, int* car_fmap, SCR_ENTRY *bg_gba, 
	u16* mv_tflg, u16* update_tfg, COORD_2D* tst_start_ctile,COORD_2D* tst_end_ctile, COORD_2D* tst_render_tid)
{
	// move 1 ctile step in any direction x/y -> trigger render 1 col/ 1 row
	s32 SAFE_ZONE_R = (SCR_WIDTH_uPx -INIT_OBJ_X) /24 + 1, // [ctile], 1 ctile = 3x3 tile
		SAFE_ZONE_L = INIT_OBJ_X /24 +1, 
		SAFE_ZONE_T = INIT_OBJ_Y /24 +1,
		SAFE_ZONE_B = (SCR_HEIGHT_uPx - INIT_OBJ_Y) /24 +1;
	
	s32 tile_x_car_max, tile_y_car_max;// [tile], limit by the carcasonne conceptual map (41x41 ctile)
	s32 tile_x_car_min, tile_y_car_min;
	map_ctile_to_tile(40, 40, &tile_x_car_max, &tile_y_car_max, false);
	map_ctile_to_tile(0, 0, &tile_x_car_min, &tile_y_car_min, false);

	//			   TBRL; 1 = move there, 0 = not move there
	u16 mv_flg = 0x0000;
	
	if (tile_cur_x != tile_prev_x)
	{
		if (tile_cur_x > tile_prev_x) 
		{
			// move right -> render 1 column on the right
			mv_flg = mv_flg | 0x0010; 
		}
		else
		{
			// move left -> render 1 column on the left
			mv_flg = mv_flg | 0x0001;
		}
	}
	else
	{
		// nothing
	}

	if (tile_cur_y != tile_prev_y)
	{
		if (tile_cur_y > tile_prev_y) 
		{
			// move down -> render 1 column on the bot
			mv_flg = mv_flg | 0x0100;

		}
		else
		{
			// move up -> render 1 column on the top
			mv_flg = mv_flg | 0x1000;
		}		
	}
	else
	{
		// nothing
	}
	
	*mv_tflg = mv_flg;
	// -> find ctile coordinate for starting rendering
	COORD_2D start_ctile_col, end_ctile_col; // [ctile]
	COORD_2D start_ctile_row, end_ctile_row;
	COORD_2D reference_ctile;
	map_tile_to_ctile(tile_cur_x, tile_cur_y, &reference_ctile.x, &reference_ctile.y); // [ctile]
	if ((mv_flg & 0x0011) == 0x0000) 
	{
		// no colum rendering
		// *flag_res = mv_flg;
	}
	else
	{
		start_ctile_col.y = reference_ctile.y - SAFE_ZONE_T;
		end_ctile_col.y = start_ctile_col.y + SAFE_ZONE_T + SAFE_ZONE_B;
		if ((mv_flg & 0x0001) == 0x0001)
		{
			*update_tfg = mv_flg;
			// move left, 	
			start_ctile_col.x = reference_ctile.x - (SAFE_ZONE_L+1);
			end_ctile_col.x = start_ctile_col.x;
		}
		else
		{
			// move right
			*update_tfg = mv_flg;
			start_ctile_col.x = reference_ctile.x + (SAFE_ZONE_R+1);
			end_ctile_col.x = start_ctile_col.x;

		}

	// 	// start rendering
		tst_start_ctile->x = start_ctile_col.x;
		tst_start_ctile->y = start_ctile_col.y;
		tst_end_ctile->x = end_ctile_col.x;
		tst_end_ctile->y = end_ctile_col.y;
	
			COORD_2D tile_curr_coord;
			// tile_curr_coord.x = tile_cur_x; //ERROR !!! HOW THE RENDER TILE = THE CURRENT TID!!!!
			// tile_curr_coord.y = tile_cur_y;
			map_ctile_to_tile(start_ctile_col.x, start_ctile_col.y, &tile_curr_coord.x, &tile_curr_coord.y, true);
			// wrapping_tile_coord(&tile_curr_coord);
			COORD_2D tile_render_coord;
					tile_render_coord.x = tile_curr_coord.x + 0;
					tile_render_coord.y = tile_curr_coord.y + 0;
					wrapping_tile_coord(&tile_render_coord);
					int se_idx = 0, sea_idx = 0;
					sea_idx = tile_render_coord.y*map_width_unit_tile + tile_render_coord.x; 
		tst_render_tid->x = tile_render_coord.x;
		tst_render_tid->y = tile_render_coord.y;

		for (start_ctile_col.y; start_ctile_col.y < end_ctile_col.y + 1; start_ctile_col.y += 1)
		{
			int car_map_id;
			CAS_TILE_MAP* car_map_ptr = NULL;
			int car_off_vram = 0;

			if (start_ctile_col.x <41 && start_ctile_col.y <41)
			{
				if (start_ctile_col.x >=0 && start_ctile_col.y >=0)
				{
					// extract the CAR MAP TID
					car_map_id = car_fmap[start_ctile_col.x*CAR_MAP_WIDTH_x + start_ctile_col.y];
					if (car_map_id == CAR_BG_ID)
					{
						car_map_ptr = bg_tile_map_id;
						car_map_id = 1;
						car_off_vram = 0;
					}
					else
					{
						car_map_ptr = cas_tile_map_id;
						car_off_vram = CAR_TILE_OFFSET_IN_VRAM;
					}
				}
				else
				{
					// outside of the CAR MAP, it should be transparent background
					car_map_ptr = bg_tile_map_id;
					car_map_id = 0;
					car_off_vram = 0;
				}

			}
			else
			{
				// outside of the CAR MAP, it should be transparent background
				car_map_ptr = bg_tile_map_id;
				car_map_id = 0;
				car_off_vram = 0;
			}

				// car_map_ptr = bg_tile_map_id;
				// car_map_id = 0;
				// car_off_vram = 0;
			// render
			int se_idx = 0, sea_idx = 0;
			COORD_2D tile_curr_coord;
			// tile_curr_coord.x = tile_cur_x;
			// tile_curr_coord.y = tile_cur_y;
			map_ctile_to_tile(start_ctile_col.x, start_ctile_col.y, &tile_curr_coord.x, &tile_curr_coord.y, false);
			COORD_2D tile_render_coord;
		
			// put down the starter tile
			int cas_r, cas_col;
			for (cas_r = 0; cas_r < 3; cas_r++)
			{
				for (cas_col=0; cas_col<3; cas_col++)
				{
					tile_render_coord.x = tile_curr_coord.x + cas_col;
					tile_render_coord.y = tile_curr_coord.y + cas_r;
					wrapping_tile_coord(&tile_render_coord);
					sea_idx = tile_render_coord.y*map_width_unit_tile + tile_render_coord.x; 

					se_idx = sea_idx >> 1;
					if (sea_idx % 2 == 0)
					{
						// write to lower 8-bit of pse, preseve the higher 8-bit of pse
						//														0 = CARCASONNE CAT 0 = STARTER TILE
						bg_gba[se_idx] = (bg_gba[se_idx] & 0xFF00) | ((car_map_ptr[car_map_id][cas_r*3 + cas_col]+car_off_vram) & 0x00FF);
					}
					else
					{
						// write higer 8-bit of pse, preserve the lower 8 bit of pse
						bg_gba[se_idx] = (bg_gba[se_idx] & 0x00FF) | (((car_map_ptr[car_map_id][cas_r*3 + cas_col]+car_off_vram)<<8)  & 0xFF00);
					}
				}
			}

		} 
	}
	
	// if ((mv_flg & 0b1100) == 0b0000) 
	// {
	// 	// no row rendering
	// }
	// else
	// {
	// 	if ((mv_flg & 0b0100) == 0b0100)
	// 	{
	// 		// move down, 	
	// 	}
	// 	else
	// 	{
	// 		// move up
	// 	}
	// }	


}


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
	REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_64x64 | BG_WRAP;
	// REG_BG2CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_AFF_64x64; 
	// set the initial position of the screen
	bgaff.pa = bg_aff_default.pa;
	bgaff.pb = bg_aff_default.pb;
	bgaff.pc = bg_aff_default.pc;
	bgaff.pd = bg_aff_default.pd;
	bgaff.dx = INIT_BG_X_OFF<<8; // = 24 [pixel] to the right 
	bgaff.dy = INIT_BG_Y_OFF<<8;
	REG_BG_AFFINE[2]= bgaff;

	// Place the bg tile 
	// into VRAM: cbb == 0
	// tid for Carcassonne graphic starts from 1, tid 0 is the bg tile.
	memcpy32(&tile8_mem[CBB_0][0], tiles_carcas_v20260329Tiles, tiles_carcas_v20260329TilesLen/sizeof(u32));

	// no palette, only 256 colors
	memcpy32(pal_bg_mem, tiles_carcas_v20260329Pal, tiles_carcas_v20260329PalLen/sizeof(u32));

}

void init_reg_obj ()
{	
	// SPrite is 8x8p@8bpp, bcz it shares data with the background (8x8p@8bpp)

	// load in 256 palette (shared data with the background), 
	memcpy32(pal_obj_mem, tiles_carcas_v20260329Pal, tiles_carcas_v20260329PalLen/sizeof(u32));
	
	
	// Place the cursor/sprite
	// into VRAM: LOW obj memory (cbb == 4)
	// tile_mem[4][0] = tiles[0];
	// first graphic is loaded in the func `draw_func` - game state `PUT_DOWN_TILE`

}

void game_loop()
{
	// === Carcassonne data
	int car_cat_track[32] = {0};
	// (virtual/conceptual) carcassonne map = 41 x 41 ctiles = 1681.
	// graphical stored tiles in VRAM.
	int carcassonne_full_map[1681] = {[0 ... 1680]= CAR_BG_ID};

	// === aff bg
	AFF_SRC_EX asx=
	{
		INIT_BG_X_OFF<<8, INIT_BG_Y_OFF<<8,			// Map coords.
		0, 0,				// Screen coords.
		0x0100, 0x0100, 0		// Scales (= x1) and angle (= 0 degree).
	};

	// ==== obj
	// set the inital position of the cursor sprite
	int x= INIT_OBJ_X, y= INIT_OBJ_Y; // [pixel] 

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
	REG_TM2D =  -0x4009;
	REG_TM2CNT= TM_ENABLE | TM_FREQ_1024; 
	// 1 period = 1024 default clock cycle
	// = 1024 * (1/16.78 Mhz) = 61 us 
	// timer 2 overflows -> re-start at the value set by REG_TM2D. in cascade, it enables the timer 3 to tick.
	// timer overflows at 0xFFFF.
	// set REG_TM2D = -n = starting value of timer 2 @ 0x10000 -n (two complement)

	// tick every ~ (n (in decimal) * 61us * 1000) ms
	REG_TM3CNT=  TM_ENABLE | TM_CASCADE; 
	
	u32 sec = -1;

	// ==== render bg
	COORD_2D sae_prev;

	while(1)
	{
		VBlankIntrWait(); // check at the end of each frame
		key_poll();
		
		// === obj
		u16 obj_x_coord, obj_y_coord; // in unit [pixel]
		int obj_x_min=INIT_OBJ_X, obj_x_max=INIT_OBJ_X + 2*8, // 2 tiles * 8 pixel per tile 
			obj_y_min=INIT_OBJ_Y, obj_y_max=INIT_OBJ_Y + 2*8;

		// === BG
		SCR_ENTRY *pse= bg0_map;
		s32 se_curr;
		s32 sae_curr;
		// int scr_x_offset = 0, scr_y_offset =0;
		int bg_x_offset = 0, bg_y_offset =0;


		// === FUNCS/ ACTIONS
		// get the cursor (object) position (using Tonc BF_GET())
		obj_x_coord = BFN_GET(cursor->attr1, ATTR1_X);
		obj_y_coord = BFN_GET(cursor->attr0, ATTR0_Y);
		// calculate the Se_index, map size 32x32t
		// >> 3: divided by 8 to convert to unit [tile]
		//							 * 32 = width of the map size in unit [tile]	
		s32 sae_curr_x = (obj_x_coord + (bgaff.dx >> 8)) >>3;
		s32 sae_curr_y = (obj_y_coord+ (bgaff.dy >> 8)) >> 3;


		if (REG_TM3D != sec)
		{
			sec = REG_TM3D;

			sae_prev.x = sae_curr_x;
			sae_prev.y = sae_curr_y;

			if (current_game_state == MEEPLE)
			{
				// == OBJ, moving relative to the screen = screen is static
				// left/right
				x += 8*key_tri_horz(); // [pixel]
				// move up/down
				y += 8*key_tri_vert(); // [pixel]
				if (x > obj_x_max)
				{
					x = obj_x_max;
				}
				if (x < obj_x_min)
				{
					x = obj_x_min;
				}
				if (y > obj_y_max)
				{
					y = obj_y_max;
				}
				if (y < obj_y_min)
				{
					y = obj_y_min;
				}
				obj_set_pos(cursor, x, y);

				// BG MUST BE STATIC
			}

			
			if (current_game_state == PUT_DOWN_TILE)
			{
				// === allow bg rolling, sprite static
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
			sae_curr_x = (obj_x_coord + (bgaff.dx >> 8)) >>3;
			sae_curr_y = (obj_y_coord+ (bgaff.dy >> 8)) >> 3;
			sae_curr = sae_curr_y*map_width_unit_tile + sae_curr_x;
			se_curr = sae_curr >> 1;
		}



		// render the bg
// render_bg (s32 tile_prev_x, s32 tile_prev_y, s32 tile_cur_x, s32 tile_cur_y, int* car_fmap, SCR_ENTRY *bg_gba)
		u16 tst_mvflag, tst_updflg;
		COORD_2D tst_start_ct, tst_end_ct, tst_rd_tid;
		render_bg(sae_prev.x, sae_prev.y, sae_curr_x, sae_curr_y, carcassonne_full_map, pse, 
			&tst_mvflag, &tst_updflg, &tst_start_ct, &tst_end_ct, &tst_rd_tid);

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
						pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[0][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM) & 0x00FF);
					}
					else
					{
						// write higer 8-bit of pse, preserve the lower 8 bit of pse
						pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[0][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM)<<8)  & 0xFF00);
					}
				}
			}

			// game state is allowed to change only when the tile is put down
			carcassonne_number_of_tiles = carcassonne_number_of_tiles +1;
			car_cat_track[0] = car_cat_track[0] + 1; 
			// save to the conceptual carmap
			COORD_2D car_coord;
			map_tile_to_ctile(sae_curr_x, sae_curr_y, &car_coord.x, &car_coord.y);
			carcassonne_full_map[car_coord.y*CAR_MAP_WIDTH_x  + car_coord.x] = 0;
			current_game_state = GET_TILE;
		}

		// allow to get a tile, only if there is still carcassonne tiles to take
		if (current_game_state == GET_TILE && carcassonne_number_of_tiles < CAR_TILES_MAX)
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
						&elem_cas_tile_set[cas_tile_map_id[rand_cat][cas_r*3 + cas_col] + CAR_TILE_OFFSET_IN_VRAM], 
						16 // 1 DTILE = 16 x u32
					);
				}
			}
			
			// change to a new game state
			current_game_state = PUT_DOWN_TILE;
		};
		

		// put allow to draw/putdown a tile
		if(current_game_state == PUT_DOWN_TILE && carcassonne_number_of_tiles < CAR_TILES_MAX)
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
						
							if ((cas_tile_map_id[rand_cat][0*3 + 1]+CAR_TILE_OFFSET_IN_VRAM) == sae_tid_top_next)
							{
								cnd_chk = cnd_chk | 0x00000001;
							}  
							else
							{
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in carcas_data.s file]
								// CARCAS TID 26 (garden) == CARCAS TID 2 (green) [index in tiles-carcas_v20260329.s file / VRAM]
								if ((sae_tid_top_next == 26 && cas_tile_map_id[rand_cat][0*3 + 1] == 0) 
									|| (sae_tid_top_next == 2 && cas_tile_map_id[rand_cat][0*3 + 1] == 24))
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

							if ((cas_tile_map_id[rand_cat][2*3 + 1]+CAR_TILE_OFFSET_IN_VRAM) == sae_tid_bot_next)
							{
								cnd_chk = cnd_chk | 0x00000010;
							}
							else
							{
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in carcas_data.s file]
								// CARCAS TID 26 (garden) == CARCAS TID 2 (green) [index in tiles-carcas_v20260329.s file / VRAM]
								if ((sae_tid_bot_next == 26 && cas_tile_map_id[rand_cat][2*3 + 1] == 0) 
									|| (sae_tid_bot_next == 2 && cas_tile_map_id[rand_cat][2*3 + 1] == 24))
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

							if ((cas_tile_map_id[rand_cat][1*3 + 2]+CAR_TILE_OFFSET_IN_VRAM) == sae_tid_r_mid)
							{
								cnd_chk = cnd_chk | 0x00000100;
							}
							else
							{
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in carcas_data.s file]
								// CARCAS TID 26 (garden) == CARCAS TID 2 (green) [index in tiles-carcas_v20260329.s file / VRAM]
								if ((sae_tid_r_mid == 26 && cas_tile_map_id[rand_cat][1*3 + 2] == 0) 
									|| (sae_tid_r_mid == 2 && cas_tile_map_id[rand_cat][1*3 + 2] == 24))
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

							if ((cas_tile_map_id[rand_cat][1*3 + 0]+CAR_TILE_OFFSET_IN_VRAM) == sae_tid_l_mid)
							{
								cnd_chk = cnd_chk | 0x00001000;
							}
							else
							{
								// CARCAS TID 24 (garden) == CARCAS TID 0 (green) [index in carcas_data.s file]
								// CARCAS TID 26 (garden) == CARCAS TID 2 (green) [index in tiles-carcas_v20260329.s file / VRAM]
								if ((sae_tid_l_mid == 26 && cas_tile_map_id[rand_cat][1*3 + 0] == 0) 
									|| (sae_tid_l_mid == 2 && cas_tile_map_id[rand_cat][1*3 + 0] == 24))
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
										pse[se_idx] = (pse[se_idx] & 0xFF00) | ((cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM) & 0x00FF);
									}
									else
									{
										// write higer 8-bit of pse, preserve the lower 8 bit of pse
										pse[se_idx] = (pse[se_idx] & 0x00FF) | (((cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM)<<8)  & 0xFF00);
									}
								}
							}

							// game state is allowed to change only when the tile is put down
							carcassonne_number_of_tiles = carcassonne_number_of_tiles +1;
							car_cat_track[rand_cat_id] = car_cat_track[rand_cat_id] + 1;
							// current_game_state = MEEPLE;
							current_game_state = GET_TILE;

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
								&elem_cas_tile_set[cas_tile_map_id[rand_cat][cas_r*3 + cas_col]+CAR_TILE_OFFSET_IN_VRAM], 
								16 // 1 DTILE = 16 x u32
							);
						}
					}
	
				};

			};
		
		// if (current_game_state == MEEPLE && carcassonne_number_of_tiles < CAR_TILES_MAX)
		// {
		// 		const TILE8 transparent_tiles[1]=
		// 	{
		// 		// bg tile8: 8bit/pixel = 2hex/pixel
		// 		// {{0x10000001, 0x01111110, 0x01111110, 0x01111110,
		// 		//   0x01111110, 0x01111110, 0x01111110, 0x10000001}},

		// 	{{0x00000000, 0x00000000, 
		// 	  0x00000000, 0x00000000,
		// 	  0x00000000, 0x00000000, 
		// 	  0x00000000, 0x00000000,
		// 	  0x00000000, 0x00000000, 
		// 	  0x00000000, 0x00000000,
		// 	  0x00000000, 0x00000000, 
		// 	  0x00000000, 0x00000000}}
		// 	};

		// 	// load the cas graphics into the buffer of the cursor: the meeple
		// 	for (cas_r = 0; cas_r < 3; cas_r++)
		// 	{
		// 		for (cas_col=0; cas_col<3; cas_col++)
		// 		{
		// 			if (cas_r == 0 && cas_col == 0)
		// 			{
		// 				// meeple TID = 26 [index in .s file]
		// 				memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
		// 					&elem_cas_tile_set[26], 
		// 					16 // 1 DTILE = 16 x u32
		// 				);						
		// 			}
		// 			else
		// 			{
		// 				//  transparent tile
		// 				memcpy32(&tile_mem[4][cas_r*8 + cas_col*2], 
		// 					&transparent_tiles[0], 
		// 					16 // 1 DTILE = 16 x u32
		// 				);					
		// 			}

		// 		}
		// 	}
		// 	if (key_hit(KEY_A))
		// 	{
		// 		// put down the meeple -> on another background, otherwise the graphic is BAD
		// 		// change game state
		// 	}
		// }
		
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
		s32 ctile_idx=0, ctile_idy=0;
		s32 render_tile_idx=0, render_tile_idy=0;
		map_tile_to_ctile(sae_curr_x, sae_curr_y, &ctile_idx, &ctile_idy);
		map_ctile_to_tile(ctile_idx, ctile_idy, &render_tile_idx, &render_tile_idy, true);
		// tte_printf("#{es;P}Tile ID#: %d\t\nLeft: %d/%d - %d/%d - bgdx/y: %ld/%ld %ld/%ld\nctile_dx/y: %ld/%ld - tile_idx/y: %ld/%ld",
		// 	rand_cat_id, 
		// 	car_cat_track[rand_cat_id]+ 1, car_cat_max[rand_cat_id],
		// 	carcassonne_number_of_tiles, CAR_TILES_MAX,
		// 	bgaff.dx >> 8, (bgaff.dx >> 8)/3, bgaff.dy >> 8,(bgaff.dy >> 8)/3,
		// 	ctile_idx, ctile_idy,
		// 	tile_idx, tile_idy);
		int tst = (-13)%3;
		s32 render_ctile_idx=0, render_ctile_idy=0;
		map_tile_to_ctile(67, 69, &render_ctile_idx, &render_ctile_idy);
		tte_printf("#{es;P}mv/up#:%d/%d-strctidx/y:%ld/%ld-ectid:%ld/%ld\ntile_dx/y: %ld/%ld ctile_dx/y: %ld/%ld\nrender_tile_dx/y: %ld/%ld - car_ID: %d",
			tst_mvflag, tst_updflg,tst_start_ct.x, tst_start_ct.y, tst_end_ct.x, tst_end_ct.y, 
			sae_curr_x, sae_curr_y,
			ctile_idx, ctile_idy,
			tst_rd_tid.x, tst_rd_tid.y,
			carcassonne_full_map[ctile_idx*41 + ctile_idy]);
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

	game_loop();

	return 0;
}
